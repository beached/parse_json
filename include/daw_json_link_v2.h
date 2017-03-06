// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
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
#include <boost/variant.hpp>
#include <cstdlib>

#include "daw_json_link_binding.h"

namespace daw {
	namespace json {
		namespace types {
			using string_view_t = boost::string_view;
			using optional_t = boost::optional;
		}
		namespace impl {
			std::string to_string( std::string val );

			template<typename LinkedType, typename Char=char>
			struct link_state_t {
				using string_t = std::basic_string<Char>;
				string_t current_element;
				string_t buffer;
				std::function<void(link_state_t &, types::string_view_t )> current_state;

				static void state_expect_string( link_state_t & self, types::string_view_t str ) {
					// In array, current_element will have a value
				}
				static state_fn_t const state_expect_real;
				static state_fn_t const state_expect_integral;
				static state_fn_t const state_expect_boolean;
				static state_fn_t const state_expect_array;
				static state_fn_t const state_expect_object;
				static state_fn_t const state_at_root {

				};

				link_state_t( ):
						current_element{ },
						buffer{ },
						current_state{ &state_at_root } { }

				void clear_buffer( ) {
					buffer.clear( );
				}

				void push( Char c ) {
					buffer.push_back( c );
				}

				void on_string( types::string_view_t str );
				void on_real( types::string_view_t str );
				void on_integral( types::string_view_t str );
				void on_boolean( bool b );
				void on_null( );
				void on_array_begin( );
				void on_array_end( );
				void on_object_begin( );
				void on_object_end( );
			};	// link_state_t
		}	// namespace impl

		template<typename Derived>
		struct json_link_v2 {
		private:
			static auto binding_map( ) {
				static std::map<std::string, json_binding_t<Derived>> result;
				return result;
			}
		protected:
			template<typname T, U>
			void json_expect_integral( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false, bool is_array = false ) {
				auto const from_json = [FromJson]( Derived & obj, types::string_view_t val ) {
					auto const tmp = strtoll( val.data( ) );
					FromJson( obj, tmp );
				};
				auto const to_json = [ToJson]( Derived const & obj ) {
					using std::to_string;
					auto const tmp = ToJson( obj );
					return to_string( tmp );
				};
				json_binding_t<Derived> new_binding{ is_array ? types::expected_json_::integral_array : types::expected_json_types::integral, is_optional, from_json, to_json };
				binding_map( )[json_name.to_string( )] = std::move( new_binding );
			}

			template<typename T, U>
			void json_expect_integral_array( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false ) {
				json_expect_integral( json_name, FromJson, ToJson, is_optional, true ); 
			}

			template<typename T, U>
			void json_expect_optional_integral( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_integral( json_name, FromJson, ToJson, true, false ); 
			}

			template<typename T, U>
			void json_expect_optional_integral_array( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_integral( json_name, FromJson, ToJson, true, true ); 
			}

			template<typname T, U>
			void json_expect_real( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false, bool is_array = false ) {
				auto const from_json = [FromJson]( Derived & obj, types::string_view_t val ) {
					auto const tmp = atof( val.data( ) );
					FromJson( obj, tmp );
				};
				auto const to_json = [ToJson]( Derived const & obj ) {
					using std::to_string;
					auto const tmp = ToJson( obj );
					return to_string( tmp );
				};
				json_binding_t<Derived> new_binding{ is_array ? types::expected_json_::real_array : types::expected_json_types::real, is_optional, from_json, to_json };
				binding_map( )[json_name.to_string( )] = std::move( new_binding );
			}

			template<typename T, U>
			void json_expect_real_array( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false ) {
				json_expect_real( json_name, FromJson, ToJson, is_optional, true ); 
			}

			template<typename T, U>
			void json_expect_optional_real( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_real( json_name, FromJson, ToJson, true, false ); 
			}

			template<typename T, U>
			void json_expect_optional_real_array( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_real( json_name, FromJson, ToJson, true, true ); 
			}

			template<typname T, U>
			void json_expect_string( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false, bool is_array = false ) {
				auto const from_json = [FromJson]( Derived & obj, types::string_view_t val ) {
					FromJson( obj, val.to_string( ) );
				};
				auto const to_json = [ToJson]( Derived const & obj ) {
					using std::to_string;
					using impl::to_string;
					auto const tmp = ToJson( obj );
					return to_string( tmp );
				};
				json_binding_t<Derived> new_binding{ is_array ? types::expected_json_::string_array : types::expected_json_types::string, is_optional, from_json, to_json };
				binding_map( )[json_name.to_string( )] = std::move( new_binding );
			}

			template<typename T, U>
			void json_expect_string_array( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false ) {
				json_expect_string( json_name, FromJson, ToJson, is_optional, true ); 
			}

			template<typename T, U>
			void json_expect_optional_string( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_string( json_name, FromJson, ToJson, true, false ); 
			}

			template<typename T, U>
			void json_expect_optional_string_array( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_string( json_name, FromJson, ToJson, true, true ); 
			}

			template<typname T, U>
			void json_expect_boolean( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false, bool is_array = false ) {
				auto const from_json = [FromJson]( Derived & obj, types::string_view_t val ) {
					FromJson( obj, val[0] == 't' );
				};
				auto const to_json = [ToJson]( Derived const & obj ) -> std::string {
					using std::to_string;
					if( ToJson( obj ) ) {
						return "true";
					}
					return "false";
				};
				json_binding_t<Derived> new_binding{ is_array ? types::expected_json_::boolean_array : types::expected_json_types::boolean, is_optional, from_json, to_json };
				binding_map( )[json_name.to_string( )] = std::move( new_binding );
			}

			template<typename T, U>
			void json_expect_boolean_array( types::boolean_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false ) {
				json_expect_boolean( json_name, FromJson, ToJson, is_optional, true ); 
			}

			template<typename T, U>
			void json_expect_optional_boolean( types::boolean_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_boolean( json_name, FromJson, ToJson, true, false ); 
			}

			template<typename T, U>
			void json_expect_optional_boolean_array( types::boolean_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_boolean( json_name, FromJson, ToJson, true, true ); 
			}

			template<typname T, U>
			void json_expect_object( types::string_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false, bool is_array = false ) {
				json_binding_t<Derived> new_binding{ is_array ? types::expected_json_types::object_array : types::expected_json_types::object, is_optional, from_json, to_json };
				binding_map( )[json_name.to_string( )] = std::move( new_binding );
				
			}

			template<typename T, U>
			void json_expect_object_array( types::object_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson, bool is_optional = false ) {
				json_expect_object( json_name, FromJson, ToJson, is_optional, true ); 
			}

			template<typename T, U>
			void json_expect_optional_object( types::object_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_object( json_name, FromJson, ToJson, true, false ); 
			}

			template<typename T, U>
			void json_expect_optional_object_array( types::object_view_t json_name, std::function<void( Derived &, T )> FromJson, std::function<U( Derived const & )> ToJson ) {
				json_expect_object( json_name, FromJson, ToJson, true, true ); 
			}

		public:
			json_link_v2( ) = default;
			~json_link_v2( ) { }
			json_link_v2( json_link_v2 const & ) = default;
			json_link_v2( json_link_v2 && ) = default;
			json_link_v2 & operator=( json_link_v2 const & ) = default;
			json_link_v2 & operator=( json_link_v2 && ) = default;

			template<typename InputIterator>
			auto from_iterator( InputIterator first, auto last ) {

			}

			auto from_string( types::string_view_t data ) {
				return from_iterator( data.begin( ), data.end( ) );
			}
		};	// json_link_v2
	}	// namespace json

}	// namespace daw
