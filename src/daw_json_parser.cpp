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

#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>

#include "daw_json_interface.h"
#include "daw_json_parser.h"
#include <daw/char_range/daw_char_range.h>
#include <daw/daw_range.h>

namespace daw {
	namespace json {
		JsonParserException::JsonParserException( std::string msg ) : message( std::move( msg ) ) {}

		namespace impl {
			template<typename Iterator>
			bool contains( Iterator first, Iterator last,
			               typename std::iterator_traits<Iterator>::value_type const &key ) {
				return std::find( first, last, key ) != last;
			}

			bool is_ws( range::UTFValType val ) {
				size_t result1 = static_cast<range::UTFValType>( 0x0009 ) - val == 0;
				size_t result2 = static_cast<range::UTFValType>( 0x000A ) - val == 0;
				size_t result3 = static_cast<range::UTFValType>( 0x000D ) - val == 0;
				size_t result4 = static_cast<range::UTFValType>( 0x0020 ) - val == 0;
				return result1 + result2 + result3 + result4 > 0;
			}

			range::UTFValType ascii_lower_case( range::UTFValType val ) {
				return val | static_cast<range::UTFValType>( ' ' );
			}

			bool is_equal( range::UTFIterator it, range::UTFValType val ) {
				return *it == val;
			}

			bool is_equal_nc( range::UTFIterator it, range::UTFValType val ) {
				return ascii_lower_case( *it ) == ascii_lower_case( val );
			}

			void skip_ws( range::CharRange &range ) {
				size_t last_inc = is_ws( *range.begin( ) );
				range.advance( last_inc );
				if( last_inc && range.size( ) > 2 ) {
					do {
						last_inc = is_ws( *range.begin( ) );
						range.advance( last_inc );
						last_inc = last_inc && is_ws( *range.begin( ) );
						range.advance( last_inc );
						last_inc = last_inc && is_ws( *range.begin( ) );
						range.advance( last_inc );
					} while( last_inc && range.size( ) > 2 );
				}
				if( last_inc && range.size( ) > 0 ) {
					last_inc = range.size( ) > 0 && is_ws( *range.begin( ) );
					range.advance( last_inc );
					last_inc = last_inc && range.size( ) > 0 && is_ws( *range.begin( ) );
					range.advance( last_inc );
					last_inc = last_inc && range.size( ) > 0 && is_ws( *range.begin( ) );
					range.advance( last_inc );
				}
			}

			bool move_range_forward_if_equal( range::CharRange &range, boost::string_view const value ) {
				auto const value_size = static_cast<size_t>( std::distance( value.begin( ), value.end( ) ) );
				auto result = std::equal( value.begin( ), value.end( ), range.begin( ) );
				if( result ) {
					range.safe_advance( value_size );
				}
				return result;
			}

			void move_to_quote( range::CharRange &range ) {
				size_t slash_count = 0;
				while( range.size( ) > 0 ) {
					auto const cur_val = *range.begin( );
					if( U'"' == cur_val && slash_count % 2 == 0 ) {
						break;
					}
					slash_count = U'\\' == cur_val ? slash_count + 1 : 0;
					++range;
				}
				if( range.size( ) == 0 ) {
					throw JsonParserException( "Not a valid JSON string" );
				}
			}

			// Strings are parsed here :)
			value_t parse_string( range::CharRange &range ) {
				if( !is_equal( range.begin( ), '"' ) ) {
					throw JsonParserException( "Not a valid JSON string" );
				}
				++range;
				auto const it_first = range.begin( );
				move_to_quote( range );
				auto const tmp = range::create_char_range( it_first, range.begin( ) );
				auto sv = tmp.to_string_view( );
				value_t result{std::move( sv )};
				++range;
				return result;
			}

			value_t parse_bool( range::CharRange &range ) {
				if( move_range_forward_if_equal( range, "true" ) ) {
					return value_t( true );
				} else if( move_range_forward_if_equal( range, "false" ) ) {
					return value_t( false );
				}
				throw JsonParserException( "Not a valid JSON bool" );
			}

			value_t parse_null( range::CharRange &range ) {
				if( !move_range_forward_if_equal( range, "null" ) ) {
					throw JsonParserException( "Not a valid JSON null" );
				}
				return value_t( nullptr );
			}

			bool is_digit( range::UTFIterator it ) {
				auto const &test = *it;
				return '0' <= test && test <= '9';
			}

			value_t parse_number( range::CharRange &range ) {
				auto const first = range.begin( );
				auto const first_range_size = range.size( );
				move_range_forward_if_equal( range, "-" );

				while( !at_end( range ) && is_digit( range.begin( ) ) ) {
					++range;
				}
				auto const is_float = !at_end( range ) && '.' == *range.begin( );
				if( is_float ) {
					++range;
					while( !at_end( range ) && is_digit( range.begin( ) ) ) {
						++range;
					};
					if( is_equal_nc( range.begin( ), 'e' ) ) {
						++range;
						if( '-' == *range.begin( ) || '+' == *range.begin( ) ) {
							++range;
						}
						while( !at_end( range ) && is_digit( range.begin( ) ) ) {
							++range;
						};
					}
				}
				if( first == range.begin( ) ) {
					throw JsonParserException( "Not a valid JSON number" );
				}

				auto const number_range_size = static_cast<size_t>( first_range_size - range.size( ) );
				auto number_range = std::make_unique<char[]>( number_range_size );
				std::transform( first, range.begin( ), number_range.get( ),
				                []( std::iterator_traits<range::UTFIterator>::value_type const &value ) {
					                return static_cast<char>( value );
				                } );
				if( is_float ) {
					try {

						auto result = value_t( boost::lexical_cast<double>( number_range.get( ), number_range_size ) );
						return result;
					} catch( boost::bad_lexical_cast const & ) {
						throw JsonParserException( "Not a valid JSON number" );
					}
				}
				try {
					auto result = value_t( boost::lexical_cast<int64_t>( number_range.get( ), number_range_size ) );
					return result;
				} catch( boost::bad_lexical_cast const & ) { throw JsonParserException( "Not a valid JSON number" ); }
			}

			value_t parse_value( range::CharRange &range );

			object_value_item parse_object_item( range::CharRange &range ) {
				auto label = parse_string( range ).get_string_value( );
				skip_ws( range );
				if( !is_equal( range.begin( ), U':' ) ) {
					throw JsonParserException( "Not a valid JSON object item" );
				}
				skip_ws( ++range );
				auto value = parse_value( range );
				auto result = std::make_pair( std::move( label ), std::move( value ) );
				return result;
			}

			value_t parse_object( range::CharRange &range ) {
				if( !is_equal( range.begin( ), U'{' ) ) {
					throw JsonParserException( "Not a valid JSON object" );
				}
				++range;
				object_value result;
				do {
					skip_ws( range );
					if( is_equal( range.begin( ), U'"' ) ) {
						auto tmp = parse_object_item( range );
						result.push_back( std::move( tmp ) );
						skip_ws( range );
					} else {
						throw JsonParserException( "Invalid JSON Object" );
					}
					if( !is_equal( range.begin( ), U',' ) ) {
						break;
					}
					++range;
				} while( !at_end( range ) );
				if( !is_equal( range.begin( ), U'}' ) ) {
					throw JsonParserException( "Not a valid JSON object" );
				}
				++range;
				result.shrink_to_fit( );
				return value_t( std::move( result ) );
			}

			value_t parse_array( range::CharRange &range ) {
				if( !is_equal( range.begin( ), U'[' ) ) {
					throw JsonParserException( "Not a valid JSON array" );
				}
				++range;
				array_value results;
				do {
					skip_ws( range );
					if( !is_equal( range.begin( ), U']' ) ) {
						results.push_back( parse_value( range ) );
						skip_ws( range );
					}
					if( !is_equal( range.begin( ), U',' ) ) {
						break;
					}
					++range;
				} while( !at_end( range ) );
				if( !is_equal( range.begin( ), U']' ) ) {
					throw JsonParserException( "Not a valid JSON array" );
				}
				++range;
				results.shrink_to_fit( );
				return value_t( results );
			}

			value_t parse_value( range::CharRange &range ) {
				value_t result;
				skip_ws( range );
				switch( *range.begin( ) ) {
				case U'{':
					result = parse_object( range );
					break;
				case U'[':
					result = parse_array( range );
					break;
				case U'"':
					result = parse_string( range );
					break;
				case U't':
				case U'f':
					result = parse_bool( range );
					break;
				case U'n':
					result = parse_null( range );
					break;
				default:
					result = parse_number( range );
				}
				skip_ws( range );
				return result;
			}

		} // namespace impl

		json_obj parse_json( range::CharIterator Begin, range::CharIterator End ) {
			try {
				range::UTFIterator it_begin( Begin );
				range::UTFIterator it_end( End );
				range::CharRange range{it_begin, it_end};
				return impl::parse_value( range );
			} catch( JsonParserException const & ) { return ::daw::json::impl::value_t( nullptr ); }
		}

		json_obj parse_json( boost::string_view const json_text ) {
			return parse_json( json_text.begin( ), json_text.end( ) );
		}

		template<>
		int64_t get<int64_t>(::daw::json::impl::value_t const &val ) {
			return val.get_integral( );
		}

		template<>
		double get<double>(::daw::json::impl::value_t const &val ) {
			return val.get_real( );
		}

		template<>
		std::string get<std::string>(::daw::json::impl::value_t const &val ) {
			return val.get_string( );
		}

		template<>
		bool get<bool>(::daw::json::impl::value_t const &val ) {
			return val.get_boolean( );
		}

		template<>
		impl::object_value get<impl::object_value>(::daw::json::impl::value_t const &val ) {
			return val.get_object( );
		}

		template<>
		impl::array_value get<impl::array_value>(::daw::json::impl::value_t const &val ) {
			return val.get_array( );
		}

	} // namespace json
} // namespace daw

