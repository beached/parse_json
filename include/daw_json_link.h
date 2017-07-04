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

#include <boost/utility/string_view_fwd.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include <daw/daw_function_iterator.h>
#include <daw/daw_memory_mapped_file.h>
#include <daw/daw_traits.h>
#include <daw/daw_utility.h>

#include "daw_json_parser.h"
#include "daw_json_value_t.h"

namespace daw {
	namespace json {
		template<typename Derived>
		class json_link;

		namespace impl {
			template<typename T, typename Dest>
			using can_dereference_to_t = typename std::is_convertible<decltype( *( std::declval<T>( ) ) ), Dest>::type;

			template<typename T, typename Dest>
			constexpr bool can_dereference_to_v = can_dereference_to_t<T, Dest>::value;

			std::string to_json_integer( json_value_t::integer_t i );
			std::string to_json_real( json_value_t::real_t d );
			std::string to_json_string( boost::string_view s );
			std::string to_json_boolean( bool b );
			std::string to_json_null( );

			template<typename T, std::enable_if_t<can_dereference_to_v<T, json_value_t::integer_t>>>
			std::string to_json_integer( T const &value ) {
				using std::to_string;
				return to_string( static_cast<json_value_t::integer_t>( *value ) );
			}

			template<typename D>
			void from_json_object( D &, json_object_value const & );

			template<typename Container>
			std::string to_json_array( Container const & );

			template<typename Derived>
			std::string to_json_object( json_link<Derived> const & );

			namespace impl {
				inline decltype( auto ) add_appender( std::string &str ) {
					return daw::make_function_iterator( [&str]( auto const &val ) { str += val; } );
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
				using std::begin;
				using value_t = std::decay_t<decltype( *begin( container ) )>;
				static_assert( std::is_convertible<value_t, json_value_t::integer_t>::value,
				               "Must supply an integer type" );
				return impl::to_json_array( container,
				                            []( value_t const &v ) -> std::string { return to_json_integer( v ); } );
			}

			template<typename Container>
			std::string to_json_real_array( Container const &container ) {
				using std::begin;
				using value_t = std::decay_t<decltype( *begin( container ) )>;
				static_assert( std::is_convertible<value_t, json_value_t::real_t>::value, "Must supply an real type" );
				return impl::to_json_array( container,
				                            []( value_t const &v ) -> std::string { return to_json_real( v ); } );
			}

			template<typename Container>
			std::string to_json_boolean_array( Container const &container ) {
				using std::begin;
				using value_t = std::decay_t<decltype( *begin( container ) )>;
				static_assert( std::is_convertible<value_t, json_value_t::boolean_t>::value,
				               "Must supply an boolean type" );
				return impl::to_json_array( container,
				                            []( value_t const &v ) -> std::string { return to_json_boolean( v ); } );
			}

			template<typename Container>
			std::string to_json_string_array( Container const &container ) {
				using std::begin;
				using value_t = std::decay_t<decltype( *begin( container ) )>;
				static_assert( std::is_convertible<value_t, std::string>::value, "Must supply an string type" );
				return impl::to_json_array( container,
				                            []( value_t const &v ) -> std::string { return to_json_string( v ); } );
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
			void copy_array_emplace_value( json_value_t const &source, std::vector<T, Args...> const &destination ){

			};

			/// @brief Copies a value_t::array to a copy assignable value stored in destination
			/// @tparam Vector a std::vector<T>
			/// @tparam Function a function to extract the value from the source
			/// @param source array of json values
			/// @param destination destination vector
			/// @param func function object to extract from a value_t
			template<typename Vector, typename Function>
			void copy_array( json_array_value const &source, Vector &destination, Function func ) {
				destination.clear( );
				using value_type = std::decay_t<decltype( *std::begin( destination ) )>;
				std::transform( std::begin( source ), std::end( source ), std::back_inserter( destination ),
				                [&func]( auto const &v ) {
					                value_type dest_v = static_cast<value_type>( func( v ) );
					                return dest_v;
				                } );
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
			constexpr link_types_t( ) noexcept : mask{daw::or_all( types... )} {

				static_assert(
				    daw::bitcount( ( daw::or_all( types... ) & ~( types_t::nullable | types_t::array ) ) == 1 ),
				    "Invalid type specified.  No variants allow" );
			}
		};

		template<typename Derived>
		class json_link {
			using serialize_function_t = std::function<std::string( Derived const & )>;
			using deserialize_function_t = std::function<void( Derived &, json_value_t const & )>;

			struct mapping_functions_t {
				serialize_function_t serialize_function;
				deserialize_function_t deserialize_function;
			}; // mapping_functions_t

			static auto &get_map( ) {
				static std::unordered_map<std::string, mapping_functions_t> s_maps;
				return s_maps;
			}

			static void check_json_maps( ) {
				if( get_map( ).empty( ) ) {
					Derived::map_to_json( );
				}
			}

			static void add_json_map( boost::string_view name, mapping_functions_t m ) {
				get_map( )[name.to_string( )] = std::move( m );
			}

			Derived &this_as_derived( ) {
				return *static_cast<Derived *>( this );
			}

			Derived const &this_as_derived( ) const {
				return *static_cast<Derived const *>( this );
			}

		  protected:
			json_link( ) = default;

			template<typename GetFunction, typename SetFunction>
			static void json_link_integer_fn( boost::string_view name, GetFunction get_function,
			                                  SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_integer( get_function( obj ) );
				};

				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         json_value_t const &value ) mutable {
					json_value_t::integer_t v = value.get_integer( );
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
				                                                         json_value_t const &value ) mutable {
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
				                                                         json_value_t const &value ) mutable {
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

				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         json_value_t const &value ) mutable {
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
				                                                         json_value_t const &value ) mutable {
					set_function( obj, value.get_array( ) );
				};

				add_json_map( name, std::move( mapping_functions ) );
			}

			template<typename GetFunction, typename SetFunction>
			static void json_link_real_array_fn( boost::string_view name, GetFunction get_function,
			                                     SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_real_array( get_function( obj ) );
				};
				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         json_value_t const &value ) mutable {
					set_function( obj, value.get_array( ) );
				};

				add_json_map( name, std::move( mapping_functions ) );
			}

			template<typename GetFunction, typename SetFunction>
			static void json_link_boolean_array_fn( boost::string_view name, GetFunction get_function,
			                                        SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_boolean_array( get_function( obj ) );
				};
				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         json_value_t const &value ) mutable {
					set_function( obj, value.get_array( ) );
				};

				add_json_map( name, std::move( mapping_functions ) );
			}

			template<typename GetFunction, typename SetFunction>
			static void json_link_string_array_fn( boost::string_view name, GetFunction get_function,
			                                       SetFunction set_function ) {
				mapping_functions_t mapping_functions;
				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return impl::to_json_string_array( get_function( obj ) );
				};
				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         json_value_t const &value ) mutable {
					set_function( obj, value.get_array( ) );
				};

				add_json_map( name, std::move( mapping_functions ) );
			}

			template<typename GetFunction, typename SetFunction>
			static void json_link_object_fn( boost::string_view name, GetFunction get_function,
			                                 SetFunction set_function ) {
				mapping_functions_t mapping_functions;

				mapping_functions.serialize_function = [get_function]( Derived const &obj ) {
					return get_function( obj ).to_json_string( );
				};

				mapping_functions.deserialize_function = [set_function]( Derived &obj,
				                                                         json_value_t const &value ) mutable {
					set_function( obj, value.get_object( ) );
				};

				add_json_map( name, std::move( mapping_functions ) );
			}

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
					ss << kv.second.serialize_function( this_as_derived( ) );
				}
				ss << "}";
				return ss.str( );
			}

			static bool has_key( boost::string_view name ) {
				return get_json_maps( ).count( name.to_string( ) ) > 0;
			}

			static Derived from_json_value( json_value_t const &json_value ) {
				auto const &json_obj = json_value.get_object( );

				Derived result;

				for( auto const &linked_item : get_json_maps( ) ) {
					auto const it = json_obj.find( linked_item.first );
					// TODO: when optional

					try {
						linked_item.second.deserialize_function( result, it->second );
					} catch( std::exception const &ex ) {
						std::string msg =
						    "Exception while deserializing member '" + linked_item.first + "': " + ex.what( );
						throw std::runtime_error( msg );
					}
				}
				return result;
			}

			static Derived from_json_string( boost::string_view json_string ) {
				auto const json_value = parse_json( json_string );
				daw::exception::daw_throw_on_false( json_value.is_object( ), "Only JsonObjects can be deserialized" );
				return from_json_value( json_value );
			}

			static auto &get_json_maps( ) {
				check_json_maps( );
				return get_map( );
			}

			template<typename D>
			static void from_json_object( D &json_link_obj, json_object_value const &obj ) {
				for( auto const &kv : obj ) {
					auto const value_name = kv.first.to_string( );
					if( json_link_obj.has_key( value_name ) ) {
						auto const mapped_functions = json_link_obj.get_json_maps( )[value_name];
						mapped_functions.deserialize_function( json_link_obj, kv.second );
					}
				}
			}
		}; // json_link

		template<typename Derived, typename = std::enable_if<std::is_base_of<json_link<Derived>, Derived>::value>>
		Derived from_file( boost::string_view file_name, bool use_default_on_error ) {
			if( !boost::filesystem::exists( file_name.data( ) ) ) {
				if( use_default_on_error ) {
					return Derived{};
				}
				throw std::runtime_error( "file not found" );
			}
			daw::filesystem::MemoryMappedFile<char> in_file{file_name};
			daw::exception::daw_throw_on_false( in_file, "Could not open file" );

			auto const json_value = parse_json( in_file.begin( ), in_file.end( ) );
			return Derived::from_json_value( json_value );
		}

		template<typename Derived, typename = std::enable_if<std::is_base_of<json_link<Derived>, Derived>::value>>
		std::vector<Derived> array_from_json_value( json_value_t const &json_value, bool use_default_on_error ) {
			std::vector<Derived> result;
			daw::exception::daw_throw_on_false( json_value.is_array( ),
			                                    "Value expected to be json array.  It was as " +
			                                        daw::json::to_string( json_value.type( ) ) );
			for( auto const &d : json_value.get_array( ) ) {
				daw::exception::dbg_throw_on_false( d.is_object( ), "Expected a json object" );
				try {
					result.push_back( Derived::from_json_value( d ) );
				} catch( std::exception const &ex ) {
					using namespace std::string_literals;
					std::string msg = "Exception while deserializing json value: "s + ex.what( );
					throw std::runtime_error( msg );
				}
			}
			return result;
		}

		template<typename Derived, typename = std::enable_if<std::is_base_of<json_link<Derived>, Derived>::value>>
		std::vector<Derived> array_from_string( boost::string_view data, bool use_default_on_error ) {
			return array_from_json_value<Derived>( parse_json( data ) );
		}

		template<typename Derived, typename = std::enable_if<std::is_base_of<json_link<Derived>, Derived>::value>>
		std::vector<Derived> array_from_file( boost::string_view file_name, bool use_default_on_error ) {
			if( !boost::filesystem::exists( file_name.data( ) ) ) {
				if( use_default_on_error ) {
					return std::vector<Derived>{};
				}
				throw std::runtime_error( "file not found" );
			}
			daw::filesystem::MemoryMappedFile<char> in_file{file_name};
			daw::exception::daw_throw_on_false( in_file, "Could not open file" );
			json_value_t json_value;
			try {
				json_value = parse_json( in_file.begin( ), in_file.end( ) );
			} catch( std::exception const &ex ) {
				std::string msg = "Exception while parsing json file '" + file_name.to_string( ) + "': " + ex.what( );
				throw std::runtime_error( msg );
			}
			try {
				return array_from_json_value<Derived>( json_value, use_default_on_error );
			} catch( std::exception const &ex ) {
				std::string msg = "Exception while deserializing json array value from file '" +
				                  file_name.to_string( ) + "': " + ex.what( );
				throw std::runtime_error( msg );
			}
		}

		template<typename Derived, typename = std::enable_if<std::is_base_of<json_link<Derived>, Derived>::value>>
		Derived from_file( boost::string_view file_name ) {
			daw::filesystem::MemoryMappedFile<char> in_file{file_name};
			daw::exception::daw_throw_on_false( in_file, "Could not open file" );

			auto const json_value = parse_json( in_file.begin( ), in_file.end( ) );
			return Derived::from_json_value( json_value );
		}

		template<typename Derived>
		void to_file( boost::string_view file_name, json_link<Derived> const &obj, bool overwrite ) {

			// obj.to_file( file_name, overwrite );
		}

		template<typename Derived>
		std::ostream &operator<<( std::ostream &os, json_link<Derived> const &data ) {
			os << data.to_json_string( );
			return os;
		}

		template<typename Derived>
		std::istream &operator>>( std::istream &is, json_link<Derived> &data ) {
			data = Derived::from_json_value(
			    parse_json( std::string{std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{}} ) );
			return is;
		}

	} // namespace json
} // namespace daw

#define json_link_integer( name, member_name )                                                                         \
	json_link_integer_fn(                                                                                              \
	    name, []( auto const &obj ) { return obj.member_name; },                                                       \
	    []( auto &obj, auto const &value ) { obj.member_name = static_cast<decltype( obj.member_name )>( value ); } );

#define json_link_object( name, member_name )                                                                          \
	json_link_object_fn( name, []( auto const &obj ) { return obj.member_name; },                                      \
	                     []( auto &obj, auto const &value ) {                                                          \
		                     json_link<decltype( obj )>::from_json_object( obj.member_name, value );                   \
	                     } );

#define json_link_integer_array( name, member_name )                                                                   \
	json_link_integer_array_fn( name, []( auto const &macro_var_obj ) { return macro_var_obj.member_name; },           \
	                            []( auto &macro_var_obj, auto const &macro_var_value ) {                               \
		                            daw::json::impl::copy_array( macro_var_value, macro_var_obj.member_name,           \
		                                                         []( daw::json::json_value_t const &macro_var_val2 ) { \
			                                                         return macro_var_val2.get_integer( );             \
		                                                         } );                                                  \
	                            } );

#define json_link_real_array( name, member_name )                                                                      \
	json_link_real_array_fn( name, []( auto const &macro_var_obj ) { return macro_var_obj.member_name; },              \
	                         []( auto &macro_var_obj, auto const &macro_var_value ) {                                  \
		                         daw::json::impl::copy_array( macro_var_value, macro_var_obj.member_name,              \
		                                                      []( daw::json::json_value_t const &macro_var_val2 ) {    \
			                                                      return macro_var_val2.get_real( );                   \
		                                                      } );                                                     \
	                         } );

#define json_link_boolean_array( name, member_name )                                                                   \
	json_link_boolean_array_fn( name, []( auto const &macro_var_obj ) { return macro_var_obj.member_name; },           \
	                            []( auto &macro_var_obj, auto const &macro_var_value ) {                               \
		                            daw::json::impl::copy_array( macro_var_value, macro_var_obj.member_name,           \
		                                                         []( daw::json::json_value_t const &macro_var_val2 ) { \
			                                                         return macro_var_val2.get_boolean( );             \
		                                                         } );                                                  \
	                            } );

#define json_link_string_array( name, member_name )                                                                    \
	json_link_string_array_fn( name, []( auto const &macro_var_obj ) { return macro_var_obj.member_name; },            \
	                           []( auto &macro_var_obj, auto const &macro_var_value ) {                                \
		                           daw::json::impl::copy_array( macro_var_value, macro_var_obj.member_name,            \
		                                                        []( daw::json::json_value_t const &macro_var_val2 ) {  \
			                                                        return macro_var_val2.get_string( );               \
		                                                        } );                                                   \
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
