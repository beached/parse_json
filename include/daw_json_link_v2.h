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
		namespace impl {
			int64_t str_to_int( boost::string_view str, int64_t );
			uint64_t str_to_int( boost::string_view str, uint64_t );
			int32_t str_to_int( boost::string_view str, int32_t );
			uint32_t str_to_int( boost::string_view str, uint32_t );
			int16_t str_to_int( boost::string_view str, int16_t );
			uint16_t str_to_int( boost::string_view str, uint16_t );
			int8_t str_to_int( boost::string_view str, int8_t );
			uint8_t str_to_int( boost::string_view str, uint8_t );
		}	// namespace impl

		template<typename Derived>
		class JsonLink;

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

		namespace schema {
			::daw::json::impl::value_t get_schema( boost::string_view name );

			::daw::json::impl::value_t get_schema( boost::string_view name, bool const & );

			::daw::json::impl::value_t get_schema( boost::string_view name, std::nullptr_t );

			::daw::json::impl::value_t get_schema( boost::string_view name, std::string const & );

			::daw::json::impl::value_t get_schema( boost::string_view name, boost::posix_time::ptime const & );

			::daw::json::impl::value_t make_type_obj( boost::string_view name, ::daw::json::impl::value_t selected_type );

			template<typename Key, typename Value>
			auto get_schema( boost::string_view name, std::pair<Key, Value> const & );
			//::daw::json::impl::value_t get_schema( boost::string_view name, std::pair<Key, Value> const & );

			template<typename T, typename std::enable_if_t<daw::traits::is_container_not_string<T>::value, long> = 0>
			auto get_schema( boost::string_view name, T const & );

			template<typename T, typename std::enable_if_t<std::is_floating_point<T>::value, long> = 0>
			auto get_schema( boost::string_view name, T const & );

			template<typename T, typename std::enable_if_t<std::is_integral<T>::value && !std::is_same<bool, T>::value, long> = 0>
			auto get_schema( boost::string_view name, T const & );

			template<typename T>
			auto get_schema( boost::string_view name, boost::optional<T> const & );

			template<typename T>
			auto get_schema( boost::string_view name, daw::optional<T> const & );

			template<typename T>
			auto get_schema( boost::string_view name, daw::optional_poly<T> const & );

			template<typename T, typename std::enable_if_t<daw::traits::is_streamable<T>::value && !daw::traits::is_numeric<T>::value && !std::is_same<std::string, T>::value, long> = 0>
			auto get_schema( boost::string_view name, T const & );
		}    // namespace schema

		template<typename T>
		struct standard_encoder_t final {
			std::string name_copy;
			T const * value_ptr;

			standard_encoder_t( boost::string_view n, T const & v );
			standard_encoder_t( ) = delete;

			~standard_encoder_t( ) = default;
			standard_encoder_t( standard_encoder_t const & ) = default;
			standard_encoder_t( standard_encoder_t && ) = default;
			standard_encoder_t & operator=( standard_encoder_t const & ) = default;
			standard_encoder_t & operator=( standard_encoder_t && ) = default;

			void operator( )( std::string & json_text ) const;
		};	// standard_encoder_t

		using encode_function_t = std::function<void( std::string & json_text )>;
		using decode_function_t = std::function<void( json_obj json_values )>;

		struct bind_functions_t final {
			encode_function_t encode;
			decode_function_t decode;

			bind_functions_t( );
			~bind_functions_t( ) = default;
			bind_functions_t( bind_functions_t const & ) = default;
			bind_functions_t( bind_functions_t && ) = default;
			bind_functions_t & operator=( bind_functions_t const & ) = default;
			bind_functions_t & operator=( bind_functions_t && ) = default;
		};	// bind_functions_t

		struct data_description_t final {
			::daw::json::impl::value_t json_type;
			bind_functions_t bind_functions;

			data_description_t( );
			~data_description_t( ) = default;
			data_description_t( data_description_t const & ) = default;
			data_description_t( data_description_t && ) = default;
			data_description_t & operator=( data_description_t const & ) = default;
			data_description_t & operator=( data_description_t && ) = default;
		};    // data_description

		struct data_t final {
			std::string m_name;
			std::map<impl::string_value, data_description_t> m_data_map;

			data_t( ) = default;
			data_t( data_t const & ) = default;
			data_t( data_t & ) = default;
			data_t & operator=( data_t const & ) = default;
			data_t & operator=( data_t & ) = default;
			~data_t( ) = default;

			data_t( boost::string_view name );
		};	// data_t

		template<typename Derived>
		class JsonLink {
			static data_t m_data;

			template<typename SerializeFunction, typename DeserializeFunction> 
			void link_value( boost::string_view name, SerializeFunction serialize_function, DeserializeFunction deserialize_function );

			constexpr static uint8_t to_nibble( uint8_t c ) noexcept;
		
			template<typename T>
			static std::string value_to_hex( T const & value );

		protected:
			constexpr JsonLink( ) noexcept { }
			~JsonLink( ) noexcept; 
		public:
			JsonLink( JsonLink const & ) = default;
			JsonLink( JsonLink && ) = default;
			JsonLink & operator=( JsonLink const & ) = default;
			JsonLink & operator=( JsonLink && ) = default;

			bool is_linked( impl::string_value name ) const;
			std::string & json_object_name( );
			std::string const & json_object_name( ) const;
			::daw::json::impl::value_t get_schema_obj( ) const;
			std::string to_string( ) const;
			void write_to_file( boost::string_view filename, bool overwrite = true ) const;
			void from_json_obj( json_obj const & json_values );
			void from_string( boost::string_view const json_text );
			void from_string( char const *json_text_begin, char const *json_text_end );
			void from_file( boost::string_view filename );
			void to_file( boost::string_view file_name, bool overwrite = true );

		protected:
			template<typename T>
			static void call_decode( T &, json_obj );

			static void call_decode( JsonLink & obj, json_obj json_values );

			template<typename T>
			static void set_name( T &, boost::string_view );

			static void set_name( JsonLink & obj, boost::string_view name );

			template<typename T>
			static encode_function_t standard_encoder( boost::string_view name, T const & value );

			template<typename T>
			static T decoder_helper( boost::string_view name, json_obj const & json_values );

			template<typename T>
			static boost::optional<T> nullable_decoder_helper( boost::string_view name, json_obj const & json_values );

			template<typename T, typename U = T>
			static decode_function_t standard_decoder( boost::string_view name, T & value );

			template<typename T>
			static uint8_t hex_to_integral( T && value );

			template<typename ForwardIterator, typename T>
			static ForwardIterator get_cp( ForwardIterator first, ForwardIterator last, T & out );

			static std::vector<uint8_t> ucs2_to_utf8( uint16_t ucs2 );
			
			static std::string unescape_string( boost::string_view src );

			template<typename T, typename U = T>
			static decode_function_t string_decoder( boost::string_view name, T & value );

			template<typename T, typename U = T>
			static decode_function_t standard_decoder( boost::string_view name, boost::optional<T> & value );

			template<typename T, typename U = T>
			static decode_function_t standard_decoder( boost::string_view name, daw::optional<T> & value );

			template<typename T, typename U = T>
			static decode_function_t standard_decoder( boost::string_view name, daw::optional_poly<T> & value );

			template<typename T>
			static bind_functions_t standard_bind_functions( boost::string_view name, T & value );
		
			void add_to_data_map( boost::string_view name, data_description_t desc );

			///
			/// \param name - name of integral value to link
			/// \param value - a reference to the linked value
			template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
			void link_integral( boost::string_view name, T & value );

			///
			/// \param name - name of value to remove link from
			/// \return - whether the linked name was found
			bool unlink( boost::string_view name );

			///
			/// \param name - name of integral value to link
			/// \param value - a reference to the linked value
			template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
			void link_integral( boost::string_view name, boost::optional<T> & value );

			///
			/// \param name - name of integral value to link
			/// \param value - a reference to the linked value
			template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
			void link_integral( boost::string_view name, daw::optional<T> & value );

			///
			/// \param name - name of integral value to link
			/// \param value - a reference to the linked value
			template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
			void link_integral( boost::string_view name, daw::optional_poly<T> & value );

			///
			/// \param name - name of real(float/double...) value to link
			/// \param value - a reference to the linked value
			template<typename T>
			void link_real( boost::string_view name, T & value );

			///
			/// \param name - name of real value to link
			/// \param value - a reference to the linked value
			template<typename T, typename std::enable_if_t<std::is_floating_point<T>::value, long> = 0>
			void link_real( boost::string_view name, boost::optional<T> & value );

			///
			/// \param name - name of string value to link
			/// \param value - a reference to the linked value
			void link_string( boost::string_view name, boost::optional<std::string> & value );

			///
			/// \param name - name of string value to link
			/// \param value - a reference to the linked value
			void link_string( boost::string_view name, daw::optional<std::string> & value );

			///
			/// \param name - name of string value to link
			/// \param value - a reference to the linked value
			void link_string( boost::string_view name, daw::optional_poly<std::string> & value );

			///
			/// \param name - name of string value to link
			/// \param value - a reference to the linked value
			void link_string( boost::string_view name, std::string & value );

			///
			/// \param name - name of boolean(true/false) value to link
			/// \param value - a reference to the linked value
			void link_boolean( boost::string_view name, bool & value );

			///
			/// \param name - name of boolean(true/false) value to link
			/// \param value - a reference to the linked value
			void link_boolean( boost::string_view name, boost::optional<bool> & value );

			///
			/// \param name - name of boolean(true/false) value to link
			/// \param value - a reference to the linked value
			void link_boolean( boost::string_view name, daw::optional<bool> & value );

			///
			/// \param name - name of boolean(true/false) value to link
			/// \param value - a reference to the linked value
			void link_boolean( boost::string_view name, daw::optional_poly<bool> & value );

			///
			/// \param name - name of JsonLink<type> obect value to link
			/// \param value - a reference to the linked value
			template<typename T>
			void link_object( boost::string_view name, JsonLink<T> & value );

			///
			/// \param name - name of JsonLink<type> obect value to link
			/// \param value - a reference to the linked value
			template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
			void link_object( boost::string_view name, boost::optional<T> & value );

			///
			/// \param name - name of JsonLink<type> obect value to link
			/// \param value - a reference to the linked value
			template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
			void link_object( boost::string_view name, daw::optional<T> & value );

			///
			/// \param name - name of JsonLink<type> obect value to link
			/// \param value - a reference to the linked value
			template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
			void link_object( boost::string_view name, daw::optional_poly<T> & value );

			///
			/// \param name - name of array(vector) value to link
			/// \param value - a reference to the linked value
			template<typename T>
			void link_array( boost::string_view name, T & value );

			///
			/// \param name - name of array(vector) value to link
			/// \param value - a reference to the linked value
			template<typename T>
			void link_array( boost::string_view name, boost::optional<T> & value );
			
			///
			/// \param name - name of array(vector) value to link
			/// \param value - a reference to the linked value
			template<typename T>
			void link_array( boost::string_view name, daw::optional<T> & value );

			///
			/// \param name - name of array(vector) value to link
			/// \param value - a reference to the linked value
			template<typename T>
			void link_array( boost::string_view name, daw::optional_poly<T> & value );

			///
			/// \param name - name of map(unorderd_map/map) value to link.
			/// \param value - a reference to the linked value
			template<typename T>
			void link_map( boost::string_view name, T & value );

			///
			/// \param name - name of map(unorderd_map/map) value to link.
			/// \param value - a reference to the linked value
			template<typename T>
			void link_map( boost::string_view name, boost::optional<T> & value );

			///
			/// \param name - name of map(unorderd_map/map) value to link.
			/// \param value - a reference to the linked value
			template<typename T>
			void link_map( boost::string_view name, daw::optional<T> & value );

			///
			/// \param name - name of map(unorderd_map/map) value to link.
			/// \param value - a reference to the linked value
			template<typename T>
			void link_map( boost::string_view name, daw::optional_poly<T> & value );

			///
			/// \param name - name of streamable value(operator<<, operator>>) to link.
			/// \param value - a reference to the linked value
			template<typename T>
			void link_streamable( boost::string_view name, T & value );

			template<typename T>
			void link_streamable( boost::string_view name, boost::optional<T> & value );

			/// Summary: Encoder Function has signature std::string( T const & ) and Decoder function has signature T( std::string const & )
			template<typename T, typename EncoderFunction, typename DecoderFunction>
			void link_jsonstring( boost::string_view name, T & value, EncoderFunction encode_function, DecoderFunction decode_function );

			template<typename T, typename EncoderFunction, typename DecoderFunction>
			void link_jsonstring( boost::string_view name, boost::optional<T> & value, EncoderFunction encode_function, DecoderFunction decode_function );

			template<typename T, typename EncoderFunction, typename DecoderFunction>
			void link_jsonintegral( boost::string_view name, T & value, EncoderFunction encode_function, DecoderFunction decode_function );

			template<typename T, typename EncoderFunction, typename DecoderFunction>
			void link_jsonintegral( boost::string_view name, boost::optional<T> & value, EncoderFunction encode_function, DecoderFunction decode_function );

			template<typename T, typename EncoderFunction, typename DecoderFunction>
			void link_jsonreal( boost::string_view name, T & value, EncoderFunction encode_function, DecoderFunction decode_function );

			template<typename T, typename EncoderFunction, typename DecoderFunction>
			void link_jsonreal( boost::string_view name, boost::optional<T> & value, EncoderFunction encode_function, DecoderFunction decode_function );

			template<typename Duration>
			void link_timestamp( boost::string_view name, std::chrono::time_point<std::chrono::system_clock, Duration> & ts, std::vector<std::string> const & fmts );

			template<typename T>
			void link_hex_value( boost::string_view name, T & value );

			/// @brief Link an vector of Values that will be encoded as a hex string( e.g. 5 -> 0x05 )
			/// @tparam T data element type in vector
			/// @param name Name of class member
			/// @param values vector of values to encode/decode
			template<typename T>
			void link_hex_array( boost::string_view name, std::vector<T> & values );

			template<typename Integral>
			void link_json_string_to_integral( boost::string_view name, Integral & i );

			template<typename Integral>
			void link_json_string_to_integral( boost::string_view name, boost::optional<Integral> & i );

			template<typename Real>
			void link_json_string_to_real( boost::string_view name, Real & r );

			template<typename T, typename ToReal, typename FromReal>
			void link_json_string_to_real( boost::string_view name, T & value, ToReal to_real, FromReal from_real );

			template<typename T>
			void link_json_string_to_real( boost::string_view name, boost::optional<T> & r );

			template<typename T, typename ToReal, typename FromReal>
			void link_json_string_to_real( boost::string_view name, boost::optional<T> & value, ToReal to_real, FromReal from_real );
			
			template<typename Duration>
			void link_iso8601_timestamp( boost::string_view name, std::chrono::time_point<std::chrono::system_clock, Duration> & ts );

			template<typename Duration>
			void link_epoch_milliseconds_timestamp( boost::string_view name, std::chrono::time_point<std::chrono::system_clock, Duration> & ts );

			///
			/// \param name - name of timestamp value(boost ptime) to link.
			/// \param value - a reference to the linked value
			void link_timestamp( boost::string_view name, boost::posix_time::ptime & value );

			///
			/// \param name - name of timestamp value(boost ptime) to link.
			/// \param value - a reference to the linked value
			void link_timestamp( boost::string_view name, boost::optional<boost::posix_time::ptime> & value );
			
			///
			/// \param name - name of timestamp value(boost ptime) to link.
			/// \param value - a reference to the linked value
			void link_timestamp( boost::string_view name, daw::optional<boost::posix_time::ptime> & value );

			///
			/// \param name - name of timestamp value(boost ptime) to link.
			/// \param value - a reference to the linked value
			void link_timestamp( boost::string_view name, daw::optional_poly<boost::posix_time::ptime> & value );
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

