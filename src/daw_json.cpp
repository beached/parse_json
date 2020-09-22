// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include "daw/json/daw_json.h"
#include "daw/json/daw_json_parser.h"

#include <daw/daw_exception.h>
#include <daw/daw_string_view.h>

#include <ctime>

namespace daw {
	void localtime_s( std::time_t const *source, struct tm *result ) {
#ifndef WIN32
		::localtime_r( source, result );
#else
		::localtime_s( result, source );
#endif // WIN32
	}

	namespace json {
		std::string ts_to_string( std::time_t const &timestamp,
		                          std::string format ) {
			char buffer[200];
			std::tm tm = { };
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
					static_assert( sizeof( c ) == 1,
					               "Src is assumed to be made of of bytes" );
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
			void json_to_value( std::string &to,
			                    daw::json::json_value_t const &from ) {
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
	}   // namespace json
} // namespace daw
