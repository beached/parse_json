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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>
#include <chrono>
#include <date/date.h>
#include <date/tz.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <daw/char_range/daw_char_range.h>
#include <daw/daw_bit_queues.h>
#include <daw/daw_exception.h>
#include <daw/daw_heap_value.h>
#include <daw/daw_memory_mapped_file.h>
#include <daw/daw_optional.h>
#include <daw/daw_optional_poly.h>
#include <daw/daw_traits.h>

#include "daw_json.h"
#include "daw_json_parser.h"

namespace daw {
	namespace json {
		template<typename Derived>
		struct JsonLink;

		template<typename Derived>
		std::ostream &operator<<( std::ostream &os, JsonLink<Derived> const &data );

		template<typename Derived>
		std::istream &operator>>( std::istream &is, JsonLink<Derived> &data );

		template<typename Derived>
		void json_to_value( JsonLink<Derived> &to, impl::value_t const &from );

		template<typename Derived>
		std::string value_to_json( boost::string_view name, JsonLink<Derived> const &obj );

		template<typename Derived>
		::daw::json::impl::value_t get_schema( boost::string_view name, JsonLink<Derived> const &obj );

		namespace impl {
			int64_t str_to_int( boost::string_view str, int64_t );
			uint64_t str_to_int( boost::string_view str, uint64_t );
			int32_t str_to_int( boost::string_view str, int32_t );
			uint32_t str_to_int( boost::string_view str, uint32_t );
			int16_t str_to_int( boost::string_view str, int16_t );
			uint16_t str_to_int( boost::string_view str, uint16_t );
			int8_t str_to_int( boost::string_view str, int8_t );
			uint8_t str_to_int( boost::string_view str, uint8_t );

			namespace json_conv {
				template<typename Derived>
				void from_json_string( Derived &json_obj ) {
					return json_obj.get_string( );
				}
				json_obj to_json_obj( boost::string_view value ) {
					return json_obj{value};
				}
				int64_t from_json_int( json_obj const &json_value ) {
					return json_value.get_integer( );
				}
				json_obj to_json_obj( int64_t value ) {
					return json_obj{std::move( value )};
				}
			} // namespace json_conv

			template<typename Derived>
			using encode_function_t = std::function<void( Derived &derived_obj, std::string &json_text )>;

			template<typename Derived>
			using decode_function_t = std::function<void( Derived &derived_obj, json_obj json_values )>;

			template<typename Derived>
			struct bind_functions_t final {
				encode_function_t<Derived> encode;
				decode_function_t<Derived> decode;

				bind_functions_t( );
				~bind_functions_t( ) = default;
				bind_functions_t( bind_functions_t const & ) = default;
				bind_functions_t( bind_functions_t && ) = default;

				bind_functions_t &operator=( bind_functions_t rhs ) {
					encode = std::move( rhs.encode );
					decode = std::move( rhs.decode );
					return *this;
				}
			}; // bind_functions_t

			template<typename Derived>
			struct data_description_t final {
				::daw::json::impl::value_t json_type;
				bind_functions_t<Derived> bind_functions;

				data_description_t( );
				~data_description_t( ) = default;
				data_description_t( data_description_t const & ) = default;
				data_description_t( data_description_t && ) = default;
				data_description_t &operator=( data_description_t rhs ) {
					json_type = std::move( rhs.json_type );
					bind_functions = std::move( rhs.bind_functions );
					return *this;
				}
			}; // data_description

			template<typename Derived>
			struct data_t final {
				std::string m_name;
				std::map<impl::string_value, data_description_t<Derived>> m_data_map;

				data_t( ) = default;
				data_t( data_t const & ) = default;
				data_t( data_t && ) = default;

				data_t &operator=( data_t rhs ) {
					m_name = std::move( rhs.m_name );
					m_data_map = std::move( rhs.m_data_map );
					return *this;
				}

				~data_t( ) = default;

				data_t( boost::string_view name );
			}; // data_t

			template<typename T, typename U>
			constexpr bool can_use_for_jsonlink_v =
			    std::integral_constant<bool,
			                           std::is_convertible<T, U>::value && std::is_convertible<U, T>::value>::value;

		} // namespace impl

		template<typename Derived, typename T>
		using get_function_t = daw::make_fp<T &( Derived & )>;

		template<typename Derived, typename T>
		using get_function2_t = daw::make_fp<T( Derived const & )>;

		template<typename Derived, typename T>
		using set_function_t = daw::make_fp<void( Derived &, T )>;

		template<typename Derived>
		struct JsonLink {
			JsonLink( ) = default;
			JsonLink( JsonLink const & ) = default;
			JsonLink &operator=( JsonLink const & ) = default;
			JsonLink( JsonLink && ) noexcept = default;
			JsonLink &operator=( JsonLink && ) noexcept = default;

			std::string to_json_string( ) const;
			void to_json_file( boost::string_view file_name, bool overwrite = true ) const;
			void from_json_string( boost::string_view const json_text );
			void from_json_string( char const *json_text_begin, char const *json_text_end );
			void from_json_file( boost::string_view filename );
			void from_json_obj( json_obj const &json_values );
			~JsonLink( ) noexcept;

		  protected:
			template<typename T, typename = std::enable_if_t<impl::can_use_for_jsonlink_v<int64_t, T>>>
			static void json_link_integer( boost::string_view name, get_function_t<Derived, T> get_function );

			template<typename T>
			static void json_link_integer( boost::string_view name, Derived T::*data_ptr );

			static void json_link_integer( boost::string_view name, get_function2_t<Derived, int64_t> get_function,
			                               set_function_t<Derived, int64_t> set_function );

			template<typename T, typename = std::enable_if_t<impl::can_use_for_jsonlink_v<double, T>>>
			static void json_link_real( boost::string_view name, get_function_t<Derived, T> get_function );

			static void json_link_real( boost::string_view name, get_function2_t<Derived, double> get_function,
			                            set_function_t<Derived, double> set_function );

			template<typename T, typename = std::enable_if_t<impl::can_use_for_jsonlink_v<std::string, T>>>
			static void json_link_string( boost::string_view name, get_function_t<Derived, T> get_function );

			static void json_link_string( boost::string_view name, get_function2_t<Derived, std::string> get_function,
			                              set_function_t<Derived, std::string> set_function );

			template<typename T, typename = std::enable_if_t<impl::can_use_for_jsonlink_v<bool, T>>>
			static void json_link_boolean( boost::string_view name, get_function_t<Derived, T> get_function );

			static void json_link_boolean( boost::string_view name, get_function2_t<Derived, bool> get_function,
			                               set_function_t<Derived, bool> set_function );

			template<typename T>
			static void json_link_object( boost::string_view name, get_function_t<Derived, JsonLink<T>> get_function );

			template<typename T>
			static void json_link_array( boost::string_view name, get_function_t<Derived, T> get_function );

			template<typename T>
			static void json_link_map( boost::string_view name, get_function_t<Derived, T> get_function );

			template<typename T>
			static void json_link_streamable( boost::string_view name, get_function_t<Derived, T> get_function );

		  private:
			Derived &as_derived( );
			Derived const &as_derived( ) const;

			static bool is_linked( impl::string_value name );
			static std::string const &json_object_name( );
			static ::daw::json::impl::value_t get_schema_obj( );

			template<typename GetFunction, bool is_optional = false>
			static void json_link_value( boost::string_view name, GetFunction get_function );

			template<typename T>
			static void call_decode( T &, json_obj );

			static void call_decode( JsonLink &obj, json_obj json_values );

			template<typename T>
			static void set_name( T &, boost::string_view );

			static void set_name( JsonLink &obj, boost::string_view name );

			template<typename T>
			static T decoder_helper( boost::string_view name, json_obj const &json_values );

			template<typename T>
			static boost::optional<T> nullable_decoder_helper( boost::string_view name, json_obj const &json_values );

			template<typename GetFunction>
			static impl::decode_function_t<Derived> standard_decoder( boost::string_view name,
			                                                          GetFunction get_function );

			template<typename ForwardIterator, typename T>
			static ForwardIterator get_cp( ForwardIterator first, ForwardIterator last, T &out );

			template<typename T, typename U = T>
			static impl::decode_function_t<Derived> string_decoder( boost::string_view name, T &value );

			template<typename GetFunction>
			static impl::decode_function_t<Derived> string_decoder( boost::string_view name, GetFunction get_function );

			template<typename GetFunction>
			static impl::bind_functions_t<Derived> standard_bind_functions( boost::string_view name,
			                                                                GetFunction get_function );

			static void add_to_data_map( boost::string_view name, impl::data_description_t<Derived> desc );

			static impl::data_t<Derived> m_data;
		}; // JsonLink

		constexpr uint8_t to_nibble( uint8_t c ) noexcept;

		template<typename T>
		std::string value_to_hex( T const &value );

		template<typename T>
		uint8_t hex_to_integer( T &&value );

		std::vector<uint8_t> ucs2_to_utf8( uint16_t ucs2 );

		std::string unescape_string( boost::string_view src );

		template<typename Derived>
		void json_to_value( JsonLink<Derived> &to, impl::value_t const &from );

		template<typename Derived>
		std::string value_to_json( boost::string_view name, JsonLink<Derived> const &obj );

		template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
		Derived from_file( boost::string_view file_name, bool use_default_on_error );

		template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
		std::vector<Derived> array_from_string( boost::string_view data, bool use_default_on_error );

		template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
		std::vector<Derived> array_from_file( boost::string_view file_name, bool use_default_on_error );

		template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
		Derived from_file( boost::string_view file_name );

		template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
		std::vector<Derived> array_from_file( boost::string_view file_name );

		template<typename Derived>
		void to_file( boost::string_view file_name, JsonLink<Derived> const &obj, bool overwrite );

		template<typename Derived>
		std::ostream &operator<<( std::ostream &os, JsonLink<Derived> const &data );

		template<typename Derived>
		std::istream &operator>>( std::istream &is, JsonLink<Derived> &data );
	} // namespace json
} // namespace daw

#include "daw_json_link_definitions.h"

#define LINK_JSON( json_type, derived_type, json_name, value_name )                                                    \
	json_link_##json_type( #json_name, []( derived_type & obj ) -> auto & { return obj.value_name; } )
