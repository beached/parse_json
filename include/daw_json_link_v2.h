// The MIT License (MIT)
//
// Copyright (c) 2014-2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <boost/utility/string_view.hpp>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include <daw/daw_traits.h>

#include "value_t.h"

namespace daw {
	namespace json {
		template<typename Derived>
		class json_link;

		namespace impl {
			template<typename D>
			void from_json_object( D &, impl::object_value const & );

			std::string to_json_integer( int64_t i ) {
				using std::to_string;
				return to_string( i );
			}

			std::string to_json_real( double d ) {
				using std::to_string;
				return to_string( d );
			}

			std::string to_json_string( boost::string_view s ) {
				std::string result;
				result = "\"" + s.to_string( ) + "\"";
				return result;
			}

			std::string to_json_boolean( bool b ) {
				return b ? "true" : "false";
			}

			std::string to_json_null( void * ) {
				return "null";
			}

			template<typename Container>
			std::string to_json_array( Container const & );

			template<typename Derived>
			std::string to_json_object( json_link<Derived> const & );

			template<typename T>
			std::string to_json_value( T const &value, impl::value_t::value_types t ) {
				switch( t ) {
				case value_t::value_types::integer:
					return to_json_integer( value );
				case value_t::value_types::real:
					return to_json_real( value );
				case value_t::value_types::string:
					return to_json_string( value );
				case value_t::value_types::boolean:
					return to_json_boolean( value );
				case value_t::value_types::null:
					return to_json_null( value );
				case value_t::value_types::array:
					return to_json_array( value );
				case value_t::value_types::object:
					return to_json_object( value );
				}
			}

			template<typename Container>
			std::string to_json_array( Container const &c, impl::value_t::value_types t ) {
				using std::begin;
				using std::end;
				auto first = begin( c );
				auto const last = end( c );
				std::string result = "[";
				while( first != last ) {
					result += ",";
					result += to_json_value( *first, t );
					++first;
				}
				result += "]";
				return result;
			}

			template<typename Integer>
			constexpr bool can_fit( int64_t const value ) noexcept {
				static_assert( std::numeric_limits<Integer>::is_integer, "Must supply an integral type" );
				if( value >= 0 ) {
					return value <= std::numeric_limits<Integer>::max( );
				} else {
					return value >= std::numeric_limits<Integer>::min( );
				}
			}

			template<typename T, typename... Args>
			void copy_array_emplace_value( daw::json::impl::value_t const &source,
			                               std::vector<T, Args...> const &destination ){

			};

			template<typename T, typename... Args>
			void copy_array( daw::json::impl::array_value const &source, std::vector<T, Args...> const &destination ) {
				destination.clear( );
				for( auto const &value : source ) {
					switch( value.type( ) ) {
					case value_t::value_types::integer:
						destination.emplace_back( value.get_integer( ) );
						break;
					case value_t::value_types::real:
						destination.emplace_back( value.get_real( ) );
						break;
					case value_t::value_types::string:
						destination.emplace_back( value.get_string( ) );
						break;
					case value_t::value_types::boolean:
						destination.emplace_back( value.get_boolean( ) );
						break;
					case value_t::value_types::null:
						destination.push_back( nullptr );
						break;
					case value_t::value_types::array: {
						T cur_array;
						copy_array( value.get_array( ), cur_array );
						destination.push_back( std::move( cur_array ) );
					} break;
					case value_t::value_types::object:
						destination.emplace_back( value.get_object( ) );
						break;
					}
				}
			}

			struct or_all {};
		} // namespace impl

		struct link_types_t {
			enum types_t : uint8_t {
				nullable = 0,
				integer = 1,
				real = 2,
				boolean = 4,
				string = 8,
				array = 16,
				object = 32
			};
			template<types_t... types>
			link_types_t( types_t... types ) {}
		};

		template<typename Derived>
		class json_link {
			using serialize_function_t = std::function<std::string( Derived const & )>;
			using deserialize_function_t = std::function<void( Derived &, impl::value_t const & )>;

			struct mapping_functions_t {
				serialize_function_t serialize_function;
				deserialize_function_t deserialize_function;
			}; // mapping_functions_t

			static std::map<std::string, mapping_functions_t> s_maps;

			static void check_json_maps( ) {
				if( s_maps.empty( ) ) {
					Derived::map_to_json( );
				}
			}
			static std::map<std::string, mapping_functions_t> get_json_maps( ) {
				check_json_maps( );
				return s_maps;
			}

			static void add_json_map( boost::string_view name, mapping_functions_t m ) {
				s_maps[name.to_string( )] = std::move( m );
			}

		  protected:
			json_link( ) = default;

			static bool has_key( boost::string_view name ) {
				return get_json_maps( ).count( name.to_string( ) ) > 0;
			}

			template<typename GetFunction, typename SetFunction>
			static void json_link_integer_fn( boost::string_view name, GetFunction get_function,
			                                  SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_integer( get_function( obj ) );
				};

				mapping_functions.deserialize_function =
				    [get_function, set_function]( Derived &obj, impl::value_t const &value ) mutable {
					    int64_t v = value.get_integer( );
					    assert( impl::can_fit<decltype( get_function( obj ) )>( v ) );
					    set_function( obj, std::move( v ) );
				    };
				add_json_map( name, std::move( mapping_functions ) );
			}

#define json_link_integer( name, member_name )                                                                         \
	json_link_integer_fn(                                                                                              \
	    name, []( auto const &obj ) { return obj.member_name; },                                                       \
	    []( auto &obj, auto const &value ) { obj.member_name = static_cast<decltype( obj.member_name )>( value ); } );

			template<typename GetFunction, typename SetFunction>
			static void json_link_real_fn( boost::string_view name, GetFunction get_function,
			                               SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_real( get_function( obj ) );
				};

				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         impl::value_t const &value ) mutable {
					set_function( obj, value.get_real( ) );
				};
				add_json_map( name, std::move( mapping_functions ) );
			}

#define json_link_real( name, member_name )                                                                            \
	json_link_real_fn( name, []( auto const &obj ) { return obj.member_name; },                                        \
	                   []( auto &obj, auto const &value ) { obj.member_name = value; } );

			template<typename GetFunction, typename SetFunction>
			static void json_link_string_fn( boost::string_view name, GetFunction get_function,
			                                 SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_string( get_function( obj ) );
				};

				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         impl::value_t const &value ) mutable {
					set_function( obj, value.get_string( ) );
				};
				add_json_map( name, std::move( mapping_functions ) );
			}

#define json_link_string( name, member_name )                                                                          \
	json_link_string_fn( name, []( auto const &obj ) { return obj.member_name; },                                      \
	                     []( auto &obj, auto const &value ) { obj.member_name = value; } );

			template<typename GetFunction, typename SetFunction>
			static void json_link_boolean_fn( boost::string_view name, GetFunction get_function,
			                                  SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_boolean( get_function( obj ) );
				};

				mapping_functions.deserialize_function =
				    [get_function, set_function]( Derived &obj, impl::value_t const &value ) mutable {
					    set_function( obj, value.get_boolean( ) );
				    };
				add_json_map( name, std::move( mapping_functions ) );
			}

#define json_link_boolean( name, member_name )                                                                         \
	json_link_boolean_fn( name, []( auto const &obj ) { return obj.member_name; },                                     \
	                      []( auto &obj, auto const &value ) { obj.member_name = value; } );

			template<typename GetFunction, typename SetFunction>
			static void json_link_integer_array_fn( boost::string_view name, GetFunction get_function,
			                                        SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_array( get_function( obj ), impl::value_t::value_types::integer );
				};
				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         impl::value_t const &value ) mutable {
					set_function( obj, value.get_array( ) );
				};
				add_json_map( name, std::move( mapping_functions ) );
			}

#define json_link_integer_array( name, member_name )                                                                   \
	json_link_integer_array_fn(                                                                                        \
	    name, []( auto const &obj ) { return obj.member_name; },                                                       \
	    []( auto &obj, auto const &value ) { daw::json::impl::copy_array( value, obj.member_name ); } );

		  public:
			template<typename D>
			friend void impl::from_json_object( D &, impl::object_value const & );

		  protected:
			template<typename GetFunction, typename SetFunction>
			static void json_link_object_fn( boost::string_view name, GetFunction get_function,
			                                 SetFunction set_function ) {
				mapping_functions_t mapping_functions;

				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return get_function( obj ).to_json_string( );
				};

				mapping_functions.deserialize_function =
				    [get_function, set_function]( Derived &obj, impl::value_t const &value ) mutable {
					    set_function( obj, value.get_object( ) );
				    };
				add_json_map( name, std::move( mapping_functions ) );
			}

#define json_link_object( name, member_name )                                                                          \
	json_link_object_fn( name, []( auto const &obj ) { return obj.member_name; },                                      \
	                     []( auto &obj, auto const &value ) { from_json_object( obj.member_name, value ); } );

		  public:
			~json_link( ) = default;
			json_link( json_link const & ) = default;
			json_link( json_link && ) = default;
			json_link &operator=( json_link const & ) = default;
			json_link &operator=( json_link && ) = default;

			std::string to_json_string( ) const {
				check_json_maps( );
				std::stringstream ss;
				bool is_first = true;
				ss << "{";
				for( auto const &kv : get_json_maps( ) ) {
					if( is_first ) {
						is_first = false;
					} else {
						ss << ",";
					}
					ss << impl::to_json_string( kv.first );
					ss << ":";
					ss << kv.second.serialize_function( *static_cast<Derived const *>( this ) );
				}
				ss << "}";
				return ss.str( );
			}

			static Derived from_json_string( boost::string_view json_string ) {
				return {};
			}
		}; // json_link

		template<typename Derived>
		std::map<std::string, typename json_link<Derived>::mapping_functions_t> json_link<Derived>::s_maps;

		namespace impl {
			template<typename D>
			void from_json_object( D &json_link_obj, daw::json::impl::object_value const &obj ) {
				for( auto const &kv : obj ) {
					auto const value_name = kv.first.to_string( );
					if( json_link_obj.has_key( value_name ) ) {
						auto const mapped_functions = json_link_obj.get_json_maps( )[value_name];
						mapped_functions.deserialize_function( json_link_obj, kv.second );
					}
				}
			}
		} // namespace impl
	}     // namespace json
} // namespace daw
