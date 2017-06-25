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

#include <daw/daw_function_iterator.h>
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

			std::string to_json_null( ) {
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
					return to_json_null( );
				case value_t::value_types::array:
					return to_json_array( value );
				case value_t::value_types::object:
					return to_json_object( value );
				}
			}
			namespace impl {
				decltype( auto ) add_appender( std::string &str ) {
					return daw::make_function_iterator( [&str]( std::string const &val ) { str += val; } );
				}

				template<typename Container, typename Function>
				std::string to_json_array( Container const &container, Function func ) {
					using std::begin;
					using std::end;
					std::string result = "[";
					bool is_first = true;
					std::transform( begin( container ), end( container ), add_appender( result ),
					                [&is_first, func]( auto const &value ) {
						                if( !is_first ) {
							                return "," + func( value );
						                }
						                is_first = false;
						                return func( value );
					                } );
					result += ']';
					return result;
				};
			} // namespace impl

			template<typename Container>
			std::string to_json_integer_array( Container const &container ) {
				return impl::to_json_array( container, &to_json_integer );
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

			/// @brief Copies a value_t::array to a copy assignable value stored in destination
			/// @tparam Vector a std::vector<T>
			/// @tparam Function a function to extract the value from the source
			/// @param source array of json values
			/// @param destination destination vector
			/// @param func function object to extract from a value_t
			template<typename Vector, typename Function>
			void copy_array( daw::json::impl::array_value const &source, Vector &destination, Function func ) {
				destination.clear( );
				using value_type = std::decay_t<decltype( *std::begin( destination ) )>;
				std::transform( std::begin( source ), std::end( source ), std::back_inserter( destination ),
				                [func]( auto const &v ) {
					                value_type dest_v = func( v );
					                return dest_v;
				                } );
			};

			constexpr auto or_all( ) noexcept {
				return 0;
			}

			template<typename Value>
			constexpr auto or_all( Value value ) noexcept {
				return value;
			}

			template<typename Value, typename... T>
			constexpr auto or_all( Value value, T... values ) noexcept {
				return value | or_all( values... );
			}

			template<typename Value>
			constexpr size_t bitcount( Value value ) noexcept {
				size_t result = 0;
				while( value ) {
					result += static_cast<size_t>( value & ~static_cast<Value>( 0b1 ) );
					value >>= 1;
				}
				return result;
			}

			template<typename Value, typename... T>
			constexpr size_t bitcount( Value value, T... values ) noexcept {
				return bitcount( value ) + bitcount( values... );
			};
		} // namespace impl

		struct link_types_t {
			enum types_t : uint8_t {
				nullable = 1,
				integer = 2,
				real = 4,
				boolean = 8,
				string = 16,
				array = 32,
				object = 64
			};
			uint8_t mask;

			template<types_t... types>
			constexpr link_types_t( ) noexcept : mask{impl::or_all( types... )} {

				static_assert(
				    impl::bitcount( ( impl::or_all( types... ) & ~( types_t::nullable | types_t::array ) ) == 1 ),
				    "Invalid type specified.  No variants allow" );
			}
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
			static std::map<std::string, mapping_functions_t> & get_json_maps( ) {
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

			template<typename GetFunction, typename SetFunction>
			static void json_link_integer_array_fn( boost::string_view name, GetFunction get_function,
			                                        SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_integer_array( get_function( obj ) );
				};
				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         impl::value_t const &value ) mutable {
					set_function( obj, value.get_array( ) );
				};
				add_json_map( name, std::move( mapping_functions ) );
			}

		  protected:
			/*
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
			}*/

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
			/*			template<typename D>
			            void from_json_object( D &json_link_obj, daw::json::impl::object_value const &obj ) {
			                for( auto const &kv : obj ) {
			                    auto const value_name = kv.first.to_string( );
			                    if( json_link_obj.has_key( value_name ) ) {
			                        auto const mapped_functions = json_link_obj.get_json_maps( )[value_name];
			                        mapped_functions.deserialize_function( json_link_obj, kv.second );
			                    }
			                }
			            }*/
		} // namespace impl
	}     // namespace json
} // namespace daw

#define json_link_integer( name, member_name )                                                                         \
	json_link_integer_fn(                                                                                              \
	    name, []( auto const &obj ) { return obj.member_name; },                                                       \
	    []( auto &obj, auto const &value ) { obj.member_name = static_cast<decltype( obj.member_name )>( value ); } );

#define json_link_object( name, member_name )                                                                          \
	json_link_object_fn( name, []( auto const &obj ) { return obj.member_name; },                                      \
	                     []( auto &obj, auto const &value ) { from_json_object( obj.member_name, value ); } );

#define json_link_integer_array( name, member_name )                                                                   \
	json_link_integer_array_fn(                                                                                        \
	    name, []( auto const &macro_var_obj ) { return macro_var_obj.member_name; },                                   \
	    []( auto &macro_var_obj, auto const &macro_var_value ) {                                                       \
		    daw::json::impl::copy_array(                                                                               \
		        macro_var_value, macro_var_obj.member_name,                                                            \
		        []( daw::json::impl::value_t const &macro_var_val2 ) { return macro_var_val2.get_integer( ); } );      \
	    } );

#define json_link_real_array( name, member_name )                                                                   \
	json_link_real_array_fn(                                                                                        \
	    name, []( auto const &macro_var_obj ) { return macro_var_obj.member_name; },                                   \
	    []( auto &macro_var_obj, auto const &macro_var_value ) {                                                       \
		    daw::json::impl::copy_array(                                                                               \
		        macro_var_value, macro_var_obj.member_name,                                                            \
		        []( daw::json::impl::value_t const &macro_var_val2 ) { return macro_var_val2.get_real( ); } );      \
	    } );

#define json_link_boolean_array( name, member_name )                                                                   \
	json_link_boolean_array_fn(                                                                                        \
	    name, []( auto const &macro_var_obj ) { return macro_var_obj.member_name; },                                   \
	    []( auto &macro_var_obj, auto const &macro_var_value ) {                                                       \
		    daw::json::impl::copy_array(                                                                               \
		        macro_var_value, macro_var_obj.member_name,                                                            \
		        []( daw::json::impl::value_t const &macro_var_val2 ) { return macro_var_val2.get_boolean( ); } );      \
	    } );

#define json_link_string_array( name, member_name )                                                                   \
	json_link_string_array_fn(                                                                                        \
	    name, []( auto const &macro_var_obj ) { return macro_var_obj.member_name; },                                   \
	    []( auto &macro_var_obj, auto const &macro_var_value ) {                                                       \
		    daw::json::impl::copy_array(                                                                               \
		        macro_var_value, macro_var_obj.member_name,                                                            \
		        []( daw::json::impl::value_t const &macro_var_val2 ) { return macro_var_val2.get_string( ); } );      \
	    } );

#define json_link_real( name, member_name )                                                                            \
	json_link_real_fn( name, []( auto const &obj ) { return obj.member_name; },                                        \
	                   []( auto &obj, auto const &value ) { obj.member_name = value; } );

#define json_link_boolean( name, member_name )                                                                         \
	json_link_boolean_fn( name, []( auto const &obj ) { return obj.member_name; },                                     \
	                      []( auto &obj, auto const &value ) { obj.member_name = value; } );

#define json_link_string( name, member_name )                                                                          \
	json_link_string_fn( name, []( auto const &obj ) { return obj.member_name; },                                      \
	                     []( auto &obj, auto const &value ) { obj.member_name = value; } );
