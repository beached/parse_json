// The MIT License (MIT)
//
// Copyright (c) 2014-2016 Darrell Wright
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

#include "daw_json_link.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace daw {
	namespace json {
		namespace impl {

			int64_t str_to_int( boost::string_view str, int64_t ) {
				return std::stoll( str.to_string( ) );	
			}

			uint64_t str_to_int( boost::string_view str, uint64_t ) {
				return std::stoull( str.to_string( ) );
			}

			int32_t str_to_int( boost::string_view str, int32_t ) {
				return std::stol( str.to_string( ) );	
			}

			uint32_t str_to_int( boost::string_view str, uint32_t ) {
				return std::stoul( str.to_string( ) );
			}

			int16_t str_to_int( boost::string_view str, int16_t ) {
				int32_t tmp = std::stol( str.to_string( ) );
				daw::exception::daw_throw_on_false( tmp >= std::numeric_limits<int16_t>::min( ) && tmp <= std::numeric_limits<int16_t>::max( ), "Value out of range for 16bit integral" ); 
				return static_cast<int16_t>(tmp);
			}

			uint16_t str_to_int( boost::string_view str, uint16_t ) {
				uint32_t tmp = std::stoul( str.to_string( ) );
				daw::exception::daw_throw_on_false( tmp <= std::numeric_limits<uint16_t>::max( ), "Value out of range for 16bit unsigned integral" ); 
				return static_cast<uint16_t>(tmp);
			}

			int8_t str_to_int( boost::string_view str, int8_t ) {
				int32_t tmp = std::stol( str.to_string( ) );
				daw::exception::daw_throw_on_false( tmp >= std::numeric_limits<int8_t>::min( ) && tmp <= std::numeric_limits<int8_t>::max( ), "Value out of range for 8bit integral" ); 
				return static_cast<int8_t>(tmp);
			}

			uint8_t str_to_int( boost::string_view str, uint8_t ) {
				uint32_t tmp = std::stoul( str.to_string( ) );
				daw::exception::daw_throw_on_false( tmp <= std::numeric_limits<uint8_t>::max( ), "Value out of range for 8bit unsigned integral" ); 
				return static_cast<uint8_t>(tmp);
			}
		}	// namespace impl
		namespace schema {
			using namespace ::daw::json::impl;

			value_t get_schema( boost::string_view name ) {
				return make_type_obj( name, value_t( std::string( "null" ) ) );
			}

			value_t get_schema( boost::string_view name, bool const & ) {
				return make_type_obj( name, value_t( std::string( "bool" ) ) );
			}

			value_t get_schema( boost::string_view name, std::nullptr_t ) {
				return make_type_obj( name, value_t( std::string( "null" ) ) );
			}

			value_t get_schema( boost::string_view name, std::string const & ) {
				return make_type_obj( name, value_t( std::string( "string" ) ) );
			}

			value_t get_schema( boost::string_view name, boost::posix_time::ptime const & ) {
				return make_type_obj( name, value_t( std::string( "string" ) ) );
			}

			value_t make_type_obj( boost::string_view name, value_t selected_type ) {
				object_value result;
				if( !name.empty( ) ) {
					result.push_back( make_object_value_item( range::create_char_range( "name" ), value_t( name ) ) );
				}
				result.push_back( make_object_value_item( range::create_char_range( "type" ), std::move( selected_type ) ) );

				return value_t( std::move( result ) );
			}
		}    // namespace schema
	}    // namespace json
}    // namespace daw
