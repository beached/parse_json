// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include <daw/json/daw_json_interface.h>

#include <daw/daw_move.h>
#include <daw/daw_range.h>
#include <daw/daw_string_view.h>
#include <daw/utf_range/daw_utf_range.h>

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

namespace daw::json {
	JsonParserException::JsonParserException( std::string msg ) noexcept
	  : message( daw::move( msg ) ) {}
	JsonParserException::~JsonParserException( ) {}

	namespace impl {
		template<typename Iterator>
		constexpr bool
		contains( Iterator first, Iterator last,
		          typename std::iterator_traits<Iterator>::value_type const
		            &key ) noexcept {
			return std::find( first, last, key ) != last;
		}

		constexpr bool is_ws( range::utf_val_type val ) noexcept {
			auto const result1 =
			  static_cast<range::utf_val_type>( 0x0009 ) - val == 0;
			auto const result2 =
			  static_cast<range::utf_val_type>( 0x000A ) - val == 0;
			auto const result3 =
			  static_cast<range::utf_val_type>( 0x000D ) - val == 0;
			auto const result4 =
			  static_cast<range::utf_val_type>( 0x0020 ) - val == 0;
			return result1 | result2 | result3 | result4;
		}

		constexpr range::utf_val_type
		ascii_lower_case( range::utf_val_type val ) noexcept {
			return val | static_cast<range::utf_val_type>( ' ' );
		}

		bool is_equal( range::utf_iterator it, range::utf_val_type val ) noexcept {
			return *it == val;
		}

		bool is_equal_nc( range::utf_iterator it,
		                  range::utf_val_type val ) noexcept {
			return ascii_lower_case( *it ) == ascii_lower_case( val );
		}

		void skip_ws( range::utf_range &range ) {
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

		bool move_range_forward_if_equal( range::utf_range &range,
		                                  daw::string_view value ) {
			auto const value_size =
			  static_cast<size_t>( std::distance( value.begin( ), value.end( ) ) );
			auto result = std::equal( value.begin( ), value.end( ), range.begin( ) );
			if( result ) {
				range.safe_advance( value_size );
			}
			return result;
		}

		void move_to_quote( range::utf_range &range ) {
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
		json_value_t parse_string( range::utf_range &range ) {
			if( !is_equal( range.begin( ), '"' ) ) {
				throw JsonParserException( "Not a valid JSON string" );
			}
			++range;
			auto const it_first = range.begin( );
			move_to_quote( range );
			auto const tmp = range::create_char_range( it_first, range.begin( ) );
			auto sv = tmp.to_string_view( );
			json_value_t result{ daw::move( sv ) };
			++range;
			return result;
		}

		json_value_t parse_bool( range::utf_range &range ) {
			if( move_range_forward_if_equal( range, "true" ) ) {
				return json_value_t( true );
			} else if( move_range_forward_if_equal( range, "false" ) ) {
				return json_value_t( false );
			}
			throw JsonParserException( "Not a valid JSON bool" );
		}

		json_value_t parse_null( range::utf_range &range ) {
			if( !move_range_forward_if_equal( range, "null" ) ) {
				throw JsonParserException( "Not a valid JSON null" );
			}
			return json_value_t{ };
		}

		bool is_digit( range::utf_iterator it ) {
			auto const &test = *it;
			return '0' <= test && test <= '9';
		}

		json_value_t parse_number( range::utf_range &range ) {
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

			auto const number_range_size =
			  static_cast<size_t>( first_range_size - range.size( ) );
			auto number_range = std::make_unique<char[]>( number_range_size );
			std::transform(
			  first, range.begin( ), number_range.get( ),
			  [](
			    std::iterator_traits<range::utf_iterator>::value_type const &value ) {
				  return static_cast<char>( value );
			  } );
			if( is_float ) {
				try {
					auto result = json_value_t( boost::lexical_cast<double>(
					  number_range.get( ), number_range_size ) );
					return result;
				} catch( boost::bad_lexical_cast const & ) {
					throw JsonParserException( "Not a valid JSON number" );
				}
			}
			try {
				auto result = json_value_t{ boost::lexical_cast<intmax_t>(
				  number_range.get( ), number_range_size ) };
				return result;
			} catch( boost::bad_lexical_cast const & ) {
				throw JsonParserException( "Not a valid JSON number" );
			}
		}

		json_value_t parse_value( range::utf_range &range );

		json_object_value_item parse_object_item( range::utf_range &range ) {
			auto label = parse_string( range ).get_string_value( );
			skip_ws( range );
			if( !is_equal( range.begin( ), U':' ) ) {
				throw JsonParserException( "Not a valid JSON object item" );
			}
			skip_ws( ++range );
			auto value = parse_value( range );
			auto result =
			  make_object_value_item( daw::move( label ), daw::move( value ) );
			return result;
		}

		json_value_t parse_object( range::utf_range &range ) {
			if( !is_equal( range.begin( ), U'{' ) ) {
				throw JsonParserException( "Not a valid JSON object" );
			}
			++range;
			skip_ws( range );
			json_object_value result;
			if( !is_equal( range.begin( ), U'}' ) ) {
				do {
					skip_ws( range );
					if( is_equal( range.begin( ), U'"' ) ) {
						auto tmp = parse_object_item( range );
						result.push_back( daw::move( tmp ) );
						skip_ws( range );
					} else {
						throw JsonParserException( "Invalid JSON Object" );
					}
					if( !is_equal( range.begin( ), U',' ) ) {
						break;
					}
					++range;
					skip_ws( range );
				} while( !at_end( range ) );
			}
			if( !is_equal( range.begin( ), U'}' ) ) {
				throw JsonParserException( "Not a valid JSON object" );
			}
			++range;
			result.shrink_to_fit( );
			return json_value_t( daw::move( result ) );
		}

		json_value_t parse_array( range::utf_range &range ) {
			if( !is_equal( range.begin( ), U'[' ) ) {
				throw JsonParserException( "Not a valid JSON array" );
			}
			++range;
			json_array_value results;
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
			return json_value_t( results );
		}

		json_value_t parse_value( range::utf_range &range ) {
			json_value_t result;
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

	json_obj parse_json( range::char_iterator Begin, range::char_iterator End ) {
		try {
			range::utf_iterator it_begin( Begin );
			range::utf_iterator it_end( End );
			range::utf_range range{ it_begin, it_end };
			return impl::parse_value( range );
		} catch( JsonParserException const & ) {
			return daw::json::json_value_t{ };
		}
	}

	json_obj parse_json( daw::string_view json_text ) {
		return parse_json( json_text.begin( ), json_text.end( ) );
	}

} // namespace daw::json
