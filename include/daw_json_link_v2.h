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
#include <limits>
#include <memory>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>

#include <daw/daw_bit_queues.h>
#include <daw/char_range/daw_char_range.h>
#include <daw/daw_exception.h>
#include <daw/daw_heap_value.h>
#include <daw/daw_memory_mapped_file.h>
#include <daw/daw_bit_queues.h>
#include <daw/daw_optional.h>
#include <daw/daw_optional_poly.h>

#include "daw_json.h"
#include "daw_json_parser.h"

namespace daw {
	namespace json {
		template<typename Derived>
		struct JsonLink;

		template<typename Derived>
		std::ostream & operator<<( std::ostream & os, JsonLink<Derived> const & data );

		template<typename Derived>
		std::istream & operator>>( std::istream & is, JsonLink<Derived> & data );

		template<typename Derived>
		void json_to_value( JsonLink<Derived> & to, impl::value_t const & from );

		template<typename Derived>
		std::string value_to_json( boost::string_view name, JsonLink<Derived> const & obj );

		template<typename Derived>
		::daw::json::impl::value_t get_schema( boost::string_view name, JsonLink<Derived> const & obj );


		template<typename Derived, typename GetFunction, typename = decltype( get_function( std::declval<Derived>( ) ) )>
		::daw::json::impl::value_t get_schema( boost::string_view name, GetFunction get_function );

		namespace impl {
			int64_t str_to_int( boost::string_view str, int64_t );
			uint64_t str_to_int( boost::string_view str, uint64_t );
			int32_t str_to_int( boost::string_view str, int32_t );
			uint32_t str_to_int( boost::string_view str, uint32_t );
			int16_t str_to_int( boost::string_view str, int16_t );
			uint16_t str_to_int( boost::string_view str, uint16_t );
			int8_t str_to_int( boost::string_view str, int8_t );
			uint8_t str_to_int( boost::string_view str, uint8_t );

			template<typename Derived>
			using encode_function_t = std::function<void( Derived & derived_obj, std::string & json_text )>;

			template<typename Derived>
			using decode_function_t = std::function<void( Derived & derived_obj, json_obj json_values )>;

			template<typename Derived>
			struct bind_functions_t final {
				encode_function_t<Derived> encode;
				decode_function_t<Derived> decode;

				bind_functions_t( );
				~bind_functions_t( ) = default;
				bind_functions_t( bind_functions_t const & ) = default;
				bind_functions_t( bind_functions_t && ) = default;

				bind_functions_t & operator=( bind_functions_t rhs ) {
					encode = std::move( rhs.encode );
					decode = std::move( rhs.decode );
					return *this;
				}
			};	// bind_functions_t

			template<typename Derived>
			struct data_description_t final {
				::daw::json::impl::value_t json_type;
				bind_functions_t<Derived> bind_functions;

				data_description_t( );
				~data_description_t( ) = default;
				data_description_t( data_description_t const & ) = default;
				data_description_t( data_description_t && ) = default;
				data_description_t & operator=( data_description_t rhs ) {
					json_type = std::move( rhs.json_type );
					bind_functions = std::move( rhs.bind_functions );
					return *this;		
				}
			};    // data_description

			template<typename Derived>
			struct data_t final {
				std::string m_name;
				std::map<impl::string_value, data_description_t<Derived>> m_data_map;

				data_t( ) = default;
				data_t( data_t const & ) = default;
				data_t( data_t && ) = default;

				data_t & operator=( data_t rhs ) {
					m_name = std::move( rhs.m_name );
					m_data_map = std::move( rhs.m_data_map );
					return *this;
				}

				~data_t( ) = default;

				data_t( boost::string_view name );
			};	// data_t
		}	// namespace impl

		template<typename Derived>
		struct JsonLink {
			JsonLink( ) = default; 
			JsonLink( JsonLink const & ) = default;
			JsonLink & operator=( JsonLink const & ) = default;
			JsonLink( JsonLink && ) noexcept = default;
			JsonLink & operator=( JsonLink && ) noexcept = default;

			std::string to_json_string( ) const;
			void to_json_file( boost::string_view file_name, bool overwrite = true ) const;
			void from_json_string( boost::string_view const json_text );
			void from_json_string( char const *json_text_begin, char const *json_text_end );
			void from_json_file( boost::string_view filename );
			void from_json_obj( json_obj const & json_values );
		protected:
			~JsonLink( ) noexcept; 

			///
			/// \param name - name of integer value to link
			/// \param get_function - a function returning a T, and taking a const ref to Derived
			template<typename GetFunction, bool is_optional = false>
			static void json_link_integer( boost::string_view name, GetFunction get_function );

			///
			/// \param name - name of real(float/double...) value to link
			/// \param get_function - a function returning a T, and taking a const ref to Derived
			template<typename GetFunction, bool is_optional = false>
			static void json_link_real( boost::string_view name, GetFunction get_function );

			///
			/// \param name - name of string value to link
			/// \param get_function - a function returning a T, and taking a const ref to Derived
			template<typename GetFunction, bool is_optional = false>
			static void json_link_string( boost::string_view name, GetFunction get_function );

			///
			/// \param name - name of boolean(true/false) value to link
			/// \param get_function - a function returning a T, and taking a const ref to Derived
			template<typename GetFunction, bool is_optional = false>
			static void json_link_boolean( boost::string_view name, GetFunction get_function );

			///
			/// \param name - name of JsonLink<type> obect value to link
			/// \param get_function - a function returning a T, and taking a const ref to Derived
			template<typename GetFunction, bool is_optional = false>
			static void json_link_object( boost::string_view name, GetFunction get_function );

			///
			/// \param name - name of array(vector) value to link
			/// \param get_function - a function returning a T, and taking a const ref to Derived
			template<typename GetFunction, bool is_optional = false>
			static void json_link_array( boost::string_view name, GetFunction get_function );

			///
			/// \param name - name of map(unorderd_map/map) value to link.
			/// \param get_function - a function returning a T, and taking a const ref to Derived
			template<typename GetFunction, bool is_optional = false>
			static void json_link_map( boost::string_view name, GetFunction get_function );

			///
			/// \param name - name of streamable value(operator<<, operator>>) to link.
			/// \param get_function - a function returning a T, and taking a const ref to Derived
			template<typename GetFunction, bool is_optional = false>
			static void json_link_streamable( boost::string_view name, GetFunction get_function );
		private:
			Derived & as_derived( );
			Derived const & as_derived( ) const;

			static bool is_linked( impl::string_value name );
			static std::string const & json_object_name( );
			static ::daw::json::impl::value_t get_schema_obj( );

			template<typename SerializeFunction, typename DeserializeFunction> 
			static void json_link_value( boost::string_view name, SerializeFunction serialize_function, DeserializeFunction deserialize_function );

			constexpr static uint8_t to_nibble( uint8_t c ) noexcept;
		
			template<typename T>
			static std::string value_to_hex( T const & value );

			template<typename T>
			static void call_decode( T &, json_obj );

			static void call_decode( JsonLink & obj, json_obj json_values );

			template<typename T>
			static void set_name( T &, boost::string_view );

			static void set_name( JsonLink & obj, boost::string_view name );

			template<typename T>
			static impl::encode_function_t<Derived> standard_encoder( boost::string_view name, T const & value );

			template<typename T>
			static T decoder_helper( boost::string_view name, json_obj const & json_values );

			template<typename T>
			static boost::optional<T> nullable_decoder_helper( boost::string_view name, json_obj const & json_values );

			template<typename T, typename U = T>
			static impl::decode_function_t<Derived> standard_decoder( boost::string_view name, T & value );

			template<typename T>
			static uint8_t hex_to_integer( T && value );

			template<typename ForwardIterator, typename T>
			static ForwardIterator get_cp( ForwardIterator first, ForwardIterator last, T & out );

			static std::vector<uint8_t> ucs2_to_utf8( uint16_t ucs2 );
			
			static std::string unescape_string( boost::string_view src );

			template<typename T, typename U = T>
			static impl::decode_function_t<Derived> string_decoder( boost::string_view name, T & value );

			template<typename T, typename U = T>
			static impl::decode_function_t<Derived> standard_decoder( boost::string_view name, boost::optional<T> & value );

			template<typename T, typename U = T>
			static impl::decode_function_t<Derived> standard_decoder( boost::string_view name, daw::optional<T> & value );

			template<typename T, typename U = T>
			static impl::decode_function_t<Derived> standard_decoder( boost::string_view name, daw::optional_poly<T> & value );

			template<typename T>
			static impl::bind_functions_t<Derived> standard_bind_functions( boost::string_view name, T & value );
		
			static void add_to_data_map( boost::string_view name, impl::data_description_t<Derived> desc );

			static impl::data_t<Derived> m_data;
		};    // JsonLink

		template<typename Derived>
		void json_to_value( JsonLink<Derived> & to, impl::value_t const & from );

		template<typename Derived>
		std::string value_to_json( boost::string_view name, JsonLink<Derived> const & obj );

		template<typename Derived>
		::daw::json::impl::value_t get_schema( boost::string_view name, JsonLink<Derived> const & obj );

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
		void to_file( boost::string_view file_name, JsonLink<Derived> const & obj, bool overwrite );

		template<typename Derived>
		std::ostream & operator<<( std::ostream & os, JsonLink<Derived> const & data );

		template<typename Derived>
		std::istream & operator>>( std::istream & is, JsonLink<Derived> & data );
	}    // namespace json
}    // namespace daw

#include "daw_json_link_definitions.h"

#define LINK_JSON( json_type, derived_type, json_name, value_name )\
	json_link_ ## json_type( #json_name, []( derived_type & obj ) -> auto& { return obj.value_name ; } )

