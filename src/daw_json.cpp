// The MIT License (MIT)
//
// Copyright (c) 2014-2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iomanip>
#include <time.h>

#include <daw/daw_exception.h>
#include <daw/daw_string_view.h>

#include "daw_json.h"
#include "daw_json_parser.h"

namespace daw {
	void localtime_s( std::time_t const *source, struct tm *result ) {
#ifndef WIN32
		::localtime_r( source, result );
#else
		::localtime_s( result, source );
#endif // WIN32
	}

	namespace json {
		std::string ts_to_string( std::time_t const &timestamp, std::string format ) {
			char buffer[200];
			std::tm tm = {};
			daw::localtime_s( &timestamp, &tm );
			auto count = std::strftime( buffer, 200, format.c_str( ), &tm );
			daw::exception::daw_throw_on_false( count < 200 );
			return std::string( buffer, buffer + count + 1 );
		}

		/*		std::string to_string( std::string const & value ) {
		return value;
		}
		*/
		namespace {
			std::string escape_string( daw::string_view src ) {
				std::string result;
				for( auto c : src ) {
					static_assert( sizeof( c ) == 1, "Src is assumed to be made of of bytes" );
					switch( c ) {
					case '\b':
						result += "\\b";
						break;
					case '\f':
						result += "\\f";
						break;
					case '\n':
						result += "\\n";
						break;
					case '\r':
						result += "\\r";
						break;
					case '\t':
						result += "\\t";
						break;
					case '\"':
						result += "\\\"";
						break;
					case '\\':
						result += "\\\\";
						break;
					case '/':
						result += "\\/";
						break;
					default:
						result += c;
					}
				}

				return result;
			}

		} // namespace

		std::string enquote( daw::string_view value ) {
			if( value.empty( ) ) {
				return "\"\"";
			}
			return "\"" + escape_string( value ) + "\"";
		}

		namespace details {
			std::string json_name( daw::string_view name ) {
				if( !name.empty( ) ) {
					return enquote( name ) + ": ";
				}
				return std::string( );
			}

			std::string enbrace( daw::string_view json_value ) {
				return "{ " + json_value.to_string( ) + " }";
			}
		} // namespace details

		namespace parse {
			// String
			void json_to_value( std::string &to, daw::json::json_value_t const &from ) {
				to = from.get_string( );
			}

			// Boolean
			void json_to_value( bool &to, daw::json::json_value_t const &from ) {
				to = from.get_boolean( );
			}

			// Number, integer
			void json_to_value( int64_t &to, daw::json::json_value_t const &from ) {
				to = from.get_integer( );
			}

			// Number, real
			void json_to_value( double &to, daw::json::json_value_t const &from ) {
				to = from.get_real( );
			}

			void json_to_value( float &to, daw::json::json_value_t const &from ) {
				to = static_cast<float>( from.get_real( ) );
			}
		} // namespace parse
	}     // namespace json
} // namespace daw
