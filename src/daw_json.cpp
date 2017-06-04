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

#include <boost/utility/string_view.hpp>
#include <iomanip>
#include <time.h>

#include <daw/daw_exception.h>

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
		using namespace generate;

		std::string ts_to_string( std::time_t const &timestamp, std::string format ) {
			char buffer[200];
			std::tm tm = {};
			::daw::localtime_s( &timestamp, &tm );
			auto count = std::strftime( buffer, 200, format.c_str( ), &tm );
			daw::exception::daw_throw_on_false( count < 200 );
			return std::string( buffer, buffer + count + 1 );
		}

		/*		std::string to_string( std::string const & value ) {
		return value;
		}
		*/
		namespace {
			std::string escape_string( boost::string_view src ) {
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

		std::string enquote( boost::string_view value ) {
			if( value.empty( ) ) {
				return "\"\"";
			}
			return "\"" + escape_string( value.to_string( ) ) + "\"";
		}

		namespace details {
			std::string json_name( boost::string_view name ) {
				if( !name.empty( ) ) {
					return enquote( name ) + ": ";
				}
				return std::string( );
			}

			std::string enbrace( boost::string_view json_value ) {
				return "{ " + json_value.to_string( ) + " }";
			}
		} // namespace details

		namespace generate {
			using namespace ::daw::json::details;

			// string
			std::string value_to_json( boost::string_view name, std::string const &value ) {
				return json_name( name ) + enquote( value );
			}

			std::string value_to_json( impl::string_value name, std::string const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// bool
			std::string value_to_json( boost::string_view name, bool value ) {
				return json_name( name ) + ( value ? "true" : "false" );
			}

			std::string value_to_json( impl::string_value name, bool value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// null
			std::string value_to_json( boost::string_view name ) {
				return json_name( name ) + "null";
			}

			std::string undefined_value_to_json( boost::string_view ) {
				return {};
			}

			std::string value_to_json( impl::string_value name ) {
				return value_to_json( to_string_view( name ) );
			}

			std::string value_to_json( boost::string_view name, int32_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( impl::string_value name, int32_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			std::string value_to_json( boost::string_view name, uint32_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( impl::string_value name, uint32_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			std::string value_to_json( boost::string_view name, int64_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( impl::string_value name, int64_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			std::string value_to_json( boost::string_view name, uint64_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( impl::string_value name, uint64_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			std::string value_to_json( boost::string_view name, double const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( impl::string_value name, double const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			std::string value_to_json( boost::string_view name, ::daw::json::impl::value_t const &value ) {
				using ::daw::json::impl::value_t;
				switch( value.type( ) ) {
				case value_t::value_types::array:
					return value_to_json( name, value.get_array( ) );
				case value_t::value_types::object:
					return value_to_json_object( name, value.get_object( ) );
				case value_t::value_types::boolean:
					return value_to_json( name, value.get_boolean( ) );
				case value_t::value_types::integral:
					return value_to_json( name, value.get_integral( ) );
				case value_t::value_types::real:
					return value_to_json( name, value.get_real( ) );
				case value_t::value_types::string:
					return value_to_json( name, value.get_string( ) );
				case value_t::value_types::null:
					return value_to_json( name );
				default:
					throw std::logic_error( "Unrecognized value_t type" );
				}
			}

			std::string value_to_json( daw::utf_string name, ::daw::json::impl::value_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			std::string value_to_json_object( boost::string_view name, ::daw::json::impl::object_value const &object ) {
				std::stringstream result;
				result << json_name( name ) << "{";
				auto range = ::daw::range::make_range( object.members_v.begin( ), object.members_v.end( ) );
				if( !range.empty( ) ) {
					result << value_to_json( range.front( ).first, range.front( ).second );
					range.move_next( );
					for( auto const &value : range ) {
						result << ", " << value_to_json( value.first, value.second );
					}
				}

				result << "}";
				return result.str( );
			}

			//			std::string value_to_json( daw::utf_string name, ::daw::json::impl::object_value const &object )
			//{ 				return value_to_json( to_string_view( name ), object );
			//			}

		} // namespace generate

		namespace parse {
			// String
			void json_to_value( std::string &to, ::daw::json::impl::value_t const &from ) {
				to = from.get_string( );
			}

			// Boolean
			void json_to_value( bool &to, ::daw::json::impl::value_t const &from ) {
				to = from.get_boolean( );
			}

			// Number, integer
			void json_to_value( int64_t &to, ::daw::json::impl::value_t const &from ) {
				to = from.get_integral( );
			}

			// Number, real
			void json_to_value( double &to, ::daw::json::impl::value_t const &from ) {
				to = from.get_real( );
			}

			void json_to_value( float &to, ::daw::json::impl::value_t const &from ) {
				to = static_cast<float>( from.get_real( ) );
			}
		} // namespace parse
	}     // namespace json
} // namespace daw

