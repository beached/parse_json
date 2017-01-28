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

			template<typename Key, typename Value>
			auto get_schema( boost::string_view name, std::pair<Key, Value> const & ) {
				//::daw::json::impl::value_t get_schema( boost::string_view name, std::pair<Key, Value> const & ) {
				using ::daw::json::impl::make_object_value_item;

				::daw::json::impl::object_value result;
				using key_t = typename std::decay<Key>::type;
				using value_t = typename std::decay<Value>::type;
				key_t k;
				value_t v;
				result.push_back( make_object_value_item( range::create_char_range( "key" ), get_schema( "key", k ) ) );
				result.push_back(
						make_object_value_item( range::create_char_range( "value" ), get_schema( "value", v ) ) );
				return make_type_obj( name, ::daw::json::impl::value_t( std::move( result ) ) );
			}

				template<typename T, typename std::enable_if_t<daw::traits::is_container_not_string<T>::value, long>>
				auto get_schema( boost::string_view name, T const & ) {
					using ::daw::json::impl::make_object_value_item;
					daw::json::impl::object_value result;

					static daw::json::impl::object_value_item const obj_type = std::make_pair(
							range::create_char_range( "type" ), daw::json::impl::value_t(
								daw::traits::is_map_like<T>::value ? std::string( "map" ) : std::string( "array" ) ) );
					result.push_back( obj_type );
					typename T::value_type t;
					result.push_back(
							make_object_value_item( range::create_char_range( "element_type" ), get_schema( "", t ) ) );
					return make_type_obj( name, ::daw::json::impl::value_t( std::move( result ) ) );
				}

				template<typename T, typename std::enable_if_t<std::is_floating_point<T>::value, long>>
				auto get_schema( boost::string_view name, T const & ) {
					return make_type_obj( name, ::daw::json::impl::value_t( std::string( "real" ) ) );
				}

				template<typename T, typename std::enable_if_t<std::is_integral<T>::value && !std::is_same<bool, T>::value, long>>
				auto get_schema( boost::string_view name, T const & ) {
					return make_type_obj( name, ::daw::json::impl::value_t( std::string( "integer" ) ) );
				}

				template<typename T>
				auto get_schema( boost::string_view name, boost::optional<T> const & ) {
					T t;
					auto result = get_schema( name, t );
					auto & obj = result.get_object( );
					obj.push_back( make_object_value_item( range::create_char_range( "nullable" ),
								::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
					return result;
				}

				template<typename T>
				auto get_schema( boost::string_view name, daw::optional<T> const & ) {
					T t;
					auto result = get_schema( name, t );
					auto & obj = result.get_object( );
					obj.push_back( make_object_value_item( range::create_char_range( "nullable" ),
								::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
					return result;
				}

				template<typename T>
				auto get_schema( boost::string_view name, daw::optional_poly<T> const & ) {
					T t;
					auto result = get_schema( name, t );
					auto & obj = result.get_object( );
					obj.push_back( make_object_value_item( range::create_char_range( "nullable" ),
								::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
					return result;
				}

				template<typename T, typename std::enable_if_t<daw::traits::is_streamable<T>::value && !daw::traits::is_numeric<T>::value && !std::is_same<std::string, T>::value, long>>
				auto get_schema( boost::string_view name, T const & ) {
					auto result = make_type_obj( name, ::daw::json::impl::value_t( std::string( "string" ) ) );
					auto & obj = result.get_object( );
					obj.push_back( make_object_value_item( range::create_char_range( "string_object" ),
								::daw::json::impl::value_t( std::string( "string_object" ) ) ) );
					return result;
				}
			}    // namespace schema

			template<typename T>
			struct standard_encoder_t {
				std::string name_copy;
				T const * value_ptr;

				standard_encoder_t( boost::string_view n, T const & v ):
						name_copy{ n.to_string( ) },
						value_ptr{ &v } { }

				void operator( )( std::string & json_text ) const {
					daw::exception::daw_throw_on_false( value_ptr );
					using namespace generate;
					json_text = value_to_json( name_copy, *value_ptr );
				}
			};	// standard_encoder_t

			template<typename Derived>
			class JsonLink {
				using encode_function_t = std::function<void( std::string & json_text )>;
				using decode_function_t = std::function<void( json_obj json_values )>;

				struct bind_functions_t final {
					encode_function_t encode;
					decode_function_t decode;

					bind_functions_t( ):
						encode{ nullptr },
						decode{ nullptr } { }

					~bind_functions_t( ) = default;

					bind_functions_t( bind_functions_t const & ) = default;

					bind_functions_t( bind_functions_t && ) = default;

					bind_functions_t & operator=( bind_functions_t const & ) = default;

					bind_functions_t & operator=( bind_functions_t && ) = default;
				};	// bind_functions_t

				struct data_description_t final {
					::daw::json::impl::value_t json_type;
					bind_functions_t bind_functions;

					data_description_t( ):
						json_type{ nullptr },
						bind_functions{ } { }

					~data_description_t( ) = default;

					data_description_t( data_description_t const & ) = default;

					data_description_t( data_description_t && ) = default;

					data_description_t & operator=( data_description_t const & ) = default;

					data_description_t & operator=( data_description_t && ) = default;


				};    // data_description


				struct data_t {
					std::string m_name;
					std::map<impl::string_value, data_description_t> m_data_map;

					data_t( ) = default;
					data_t( data_t const & ) = default;
					data_t( data_t & ) = default;
					data_t & operator=( data_t const & ) = default;
					data_t & operator=( data_t & ) = default;
					~data_t( ) = default;

					template<typename T>
					data_t( T && name ):
						m_name{ std::forward<T>( name ) },
						m_data_map{ } { }
				};
				daw::heap_value<data_t> m_data;

				///
				/// \param name - name of integral value to link
				/// \param value - a reference to the linked value
				template<typename T> 
				void link_value( boost::string_view name, T & value ) {
					set_name( value, name );
					data_description_t data_description;
					data_description.json_type = ::daw::json::schema::get_schema( name, value );
					data_description.bind_functions = standard_bind_functions( name, value );
					add_to_data_map( name, std::move( data_description ) );
				}

			public:
				virtual ~JsonLink( );

				bool is_linked( impl::string_value name ) const {
					return m_data->m_data_map.count( name ) != 0;
				}

				JsonLink( std::string name = "" ):
					m_data{ std::move( name ) } { }

				JsonLink( JsonLink const & ) = delete;
				JsonLink( JsonLink && ) = delete;
				JsonLink & operator=( JsonLink const & ) = default;
				JsonLink & operator=( JsonLink && ) = default;

				std::string & json_object_name( ) {
					return m_data->m_name;
				}

				std::string const & json_object_name( ) const {
					return m_data->m_name;
				}

				auto get_schema_obj( ) const {
					::daw::json::impl::object_value result;
					using mapped_value_t = typename decltype( m_data->m_data_map )::value_type;
					std::transform( std::begin( m_data->m_data_map ), std::end( m_data->m_data_map ), std::back_inserter( result ),
							[]( mapped_value_t const & value ) {
							return ::daw::json::impl::make_object_value_item( value.first,
									value.second.json_type );
							} );
					return ::daw::json::impl::value_t( std::move( result ) );
				}

				std::string to_string( ) const {
					std::stringstream result;
					auto range = daw::range::make_range( m_data->m_data_map );
					std::string tmp;

					range.front( ).second.bind_functions.encode( tmp );
					result << tmp;
					range.move_next( );

					for( auto const & value : range ) {
						value.second.bind_functions.encode( tmp );
						result << ", " << tmp;
					}
					return details::json_name( m_data->m_name ) + details::enbrace( result.str( ) );
				}

				void write_to_file( boost::string_view filename, bool overwrite = true ) const {
					daw::exception::daw_throw_on_false( !filename.empty( ) );								
					auto fname = filename.to_string( );
					if( !overwrite && boost::filesystem::exists( fname.c_str( ) ) ) {
						throw std::runtime_error( "Overwrite not permitted and file exists" );
					}
					std::ofstream out_file;
					
					out_file.open( fname.c_str( ), std::ios::out | std::ios::trunc );

					if( !out_file.is_open( ) ) {
						throw std::runtime_error( "Could not open file for writing" );
					}
					out_file << to_string( );
					out_file.close( );
				}

			public:

				void from_json_obj( json_obj const & json_values ) {
					for( auto & value : m_data->m_data_map ) {
						value.second.bind_functions.decode( json_values );
					}
				}

				void from_string( boost::string_view const json_text ) {
					auto tmp = parse_json( json_text );
					from_json_obj( std::move( tmp ) );
				}

				void from_string( char const *json_text_begin, char const *json_text_end ) {
					auto tmp = parse_json( json_text_begin, json_text_end );
					from_json_obj( std::move( tmp ) );
				}

				void from_file( boost::string_view filename ) {
					std::ifstream in_file;
					in_file.open( filename.data( ) );
					if( !in_file ) {
						throw std::runtime_error( "Could not open file" );
					}
					std::string data{ std::istreambuf_iterator<char>{ in_file }, std::istreambuf_iterator<char>{ } };
					in_file.close( );
					parse_json( data );
				}

				void to_file( boost::string_view file_name, bool overwrite = true ) {
					daw::exception::daw_throw_on_true( !overwrite && boost::filesystem::exists( file_name.data( ) ), "File exists but overwrite not permitted" );
					std::ofstream out_file;
					out_file.open( file_name.data( ), std::ios::out | std::ios::trunc );
					out_file << to_string( ); 
				}

			public:
				template<typename T>
				static void call_decode( T &, json_obj ) { }

				static void call_decode( JsonLink & obj, json_obj json_values ) {
					obj.from_json_obj( std::move( json_values ) );
				}

				template<typename T>
				static void set_name( T &, boost::string_view ) { }

				static void set_name( JsonLink & obj, boost::string_view name ) {
					obj.json_object_name( ) = name.to_string( );
				}

				template<typename T>
				static encode_function_t standard_encoder( boost::string_view name, T const & value ) {
					encode_function_t result = standard_encoder_t<T>{ name, value };
					return result;
				}

				template<typename T>
				static T decoder_helper( boost::string_view name, json_obj const & json_values ) {
					auto obj = json_values.get_object( );
					auto member = obj.find( name );
					if( obj.end( ) == member ) {
						std::stringstream ss;
						ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
						ss << " available members { ";
						for( auto const & m: obj.container( ) ) {
							ss << "'" << m.first << "' ";
						}
						ss << "}";
						throw std::runtime_error( ss.str( ) );
					}
					return get<T>( member->second );
				}

				template<typename T>
				static boost::optional<T> nullable_decoder_helper( boost::string_view name, json_obj const & json_values ) {
					auto obj = json_values.get_object( );
					auto member = obj.find( name );
					if( obj.end( ) == member ) {
						return boost::none;
					} else if( member->second.is_null( ) ) {
						return boost::none;
					}
					return boost::optional<T>{ get<T>( member->second ) };
				}

				template<typename T, typename U = T>
				static decode_function_t standard_decoder( boost::string_view name, T & value ) {
					return [value_ptr = &value, name_copy = name.to_string( )]( json_obj json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto new_val = decoder_helper<U>( name_copy, json_values );
						*value_ptr = new_val;
					};
				}

				template<typename T>
				static uint8_t hex_to_integral( T && value ) {
					if( 'A' <= value && value <= 'F' ) {
						return static_cast<uint8_t>((value - 'A') + 10);
					} else if( 'a' <= value && value <= 'a' ) {
						return static_cast<uint8_t>((value - 'a') + 10);
					} else if( '0' <= value && value <= '9' ) {
						return static_cast<uint8_t>(value - '0');
					}
					throw std::runtime_error( "Unicode escape sequence was not properly formed" );
				}

				template<typename ForwardIterator, typename T>
				static ForwardIterator get_cp( ForwardIterator first, ForwardIterator last, T & out ) {
					auto count = sizeof( out );
					daw::nibble_queue_gen<uint16_t, uint16_t> nibbles;
					auto it = first;
					for( ; it != (first + count) && it != last; ++it ) {
						nibbles.push_back( hex_to_integral( *it ) );
					}
					if( nibbles.full( ) ) {
						throw std::runtime_error( "Unicode escape sequence was not properly formed" );
					}
					out = nibbles.pop_front( );
					return it;
				}

				static std::vector<uint8_t> ucs2_to_utf8( uint16_t ucs2 ) {
					std::vector<uint8_t> result;
					if( ucs2 < 0x0080 ) {
						result.push_back( static_cast<uint8_t>(ucs2) );
					} else if( ucs2 >= 0x0080 && ucs2 < 0x0800 ) {
						result.push_back( static_cast<uint8_t>((ucs2 >> 6) | 0xC0) );
						result.push_back( static_cast<uint8_t>((ucs2 & 0x3F) | 0x80) );
					} else if( ucs2 >= 0x0800 && ucs2 < 0xFFFF ) {
						if( ucs2 >= 0xD800 && ucs2 <= 0xDFFF ) {
							/* Ill-formed. */
							throw std::runtime_error( "Unicode Surrogate Pair" );
						}
						result.push_back( static_cast<uint8_t>((ucs2 >> 12) | 0xE0 ) );
						result.push_back( static_cast<uint8_t>(((ucs2 >> 6) & 0x3F) | 0x80 ) );
						result.push_back( static_cast<uint8_t>((ucs2 & 0x3F) | 0x80 ) );
					} else if( ucs2 >= 0x10000 && ucs2 < 0x10FFFF ) {
						/* http://tidy.sourceforge.net/cgi-bin/lxr/source/src/utf8.c#L380 */
						result.push_back( static_cast<uint8_t>(0xF0 | (ucs2 >> 18)) );
						result.push_back( static_cast<uint8_t>(0x80 | ((ucs2 >> 12) & 0x3F)) );
						result.push_back( static_cast<uint8_t>(0x80 | ((ucs2 >> 6) & 0x3F)) );
						result.push_back( static_cast<uint8_t>(0x80 | ((ucs2 & 0x3F))) );
					} else {
						throw std::runtime_error( "Bad input" );
					}
					return result;
				}
				
				static std::string unescape_string( boost::string_view src ) {
					auto rng = daw::range::create_char_range( src.begin( ), src.begin( ) + src.size( ) );
					std::string result;
					auto last_it = rng.begin( );
					for( auto it = rng.begin( ); it != rng.end( ); ++it ) {
						if( *it == U'\\' ) {
							utf8::unchecked::utf32to8( last_it, it, std::back_inserter( result ) );
							++it;
							last_it = std::next( it );
							switch( *it ) {
								case U'b': result.push_back( '\b' );
										   break;
								case U'f': result.push_back( '\f' );
										   break;
								case U'n': result.push_back( '\n' );
										   break;
								case U'r': result.push_back( '\r' );
										   break;
								case U't': result.push_back( '\t' );
										   break;
								case U'\"': result.push_back( '"' );
											break;
								case U'\\': result.push_back( '\\' );
											break;
								case U'/': result.push_back( '/' );
										   break;
								case U'u': {
											   uint16_t cp;
											   it = get_cp( it, rng.end( ), cp );
											   auto utf8 = ucs2_to_utf8( cp );
											   std::copy( utf8.begin( ), utf8.end( ), std::back_inserter( result ) );
											   if( it == rng.end( ) ) {
												   break;
											   }
										   }
										   break;
								default: throw std::runtime_error( "Unknown escape sequence" );
							}
						}
					}
					utf8::unchecked::utf32to8( last_it, rng.end( ), std::back_inserter( result ) );
					return result;
				}


				template<typename T, typename U = T>
				static decode_function_t string_decoder( boost::string_view name, T & value ) {
					auto value_ptr = &value;
					auto name_copy = name.to_string( );
					return [value_ptr, name_copy]( json_obj json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto new_val = decoder_helper<U>( name_copy, json_values );
						*value_ptr = unescape_string( new_val );
					};
				}

				template<typename T, typename U = T>
				static decode_function_t standard_decoder( boost::string_view name, boost::optional<T> & value ) {
					auto value_ptr = &value;
					auto name_copy = name.to_string( );
					return [value_ptr, name_copy]( json_obj json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto new_val = nullable_decoder_helper<U>( name_copy, json_values );
						*value_ptr = new_val;
					};
				}

				template<typename T, typename U = T>
				static decode_function_t standard_decoder( boost::string_view name, daw::optional<T> & value ) {
					auto value_ptr = &value;
					auto name_copy = name.to_string( );
					return [value_ptr, name_copy]( json_obj json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto new_val = nullable_decoder_helper<U>( name_copy, json_values );
						if( new_val ) {
							*value_ptr = std::move( new_val );
						} else {
							*value_ptr.reset( );
						}
					};
				}

				template<typename T, typename U = T>
				static decode_function_t standard_decoder( boost::string_view name, daw::optional_poly<T> & value ) {
					auto value_ptr = &value;
					auto name_copy = name.to_string( );
					return [value_ptr, name_copy]( json_obj json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto new_val = nullable_decoder_helper<U>( name_copy, json_values );
						if( new_val ) {
							*value_ptr = std::move( new_val );
						} else {
							*value_ptr.reset( );
						}
					};
				}

				template<typename T>
				static bind_functions_t standard_bind_functions( boost::string_view name, T & value ) {
					bind_functions_t bind_functions;
					bind_functions.encode = standard_encoder( name, value );
					bind_functions.decode = standard_decoder( name, value );
					return bind_functions;
				}
			
				void add_to_data_map( boost::string_view name, data_description_t desc ) {
					auto key = range::create_char_range( name );
					daw::exception::daw_throw_on_false( m_data->m_data_map.count( key ) == 0 ); 
					auto result = m_data->m_data_map.emplace( std::move( key ), std::move( desc ) );
					daw::exception::daw_throw_on_false( result.second );
				}

				///
				/// \param name - name of integral value to link
				/// \param value - a reference to the linked value
				template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
				void link_integral( boost::string_view name, T & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );

					data_description.bind_functions.encode = standard_encoder( name, value );

					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto result = decoder_helper<int64_t>( name, json_values );
						daw::exception::daw_throw_on_false( result <= std::numeric_limits<T>::max( ) );
						daw::exception::daw_throw_on_false( result >= std::numeric_limits<T>::min( ) );
						*value_ptr = static_cast<T>(result);
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of value to remove link from
				/// \return - whether the linked name was found
				bool unlink( boost::string_view name ) {
					return m_data->m_data_map.erase( range::create_char_range( name ) ) > 0;
				}


				///
				/// \param name - name of integral value to link
				/// \param value - a reference to the linked value
				template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
				void link_integral( boost::string_view name, boost::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );

					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto result = nullable_decoder_helper<int64_t>( name, json_values );
						if( result ) {
							daw::exception::daw_throw_on_false( *result <= std::numeric_limits<T>::max( ) );    // TODO determine if throwing is more appropriate
							daw::exception::daw_throw_on_false( *result >= std::numeric_limits<T>::min( ) );
							*value_ptr = static_cast<T>(*result);
						}
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of integral value to link
				/// \param value - a reference to the linked value
				template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
				void link_integral( boost::string_view name, daw::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );

					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto result = nullable_decoder_helper<int64_t>( name, json_values );
						if( result ) {
							daw::exception::daw_throw_on_false( *result <=
									std::numeric_limits<T>::max( ) );    // TODO determine if throwing is more appropriate
							daw::exception::daw_throw_on_false( *result >= std::numeric_limits<T>::min( ) );
						}
						*value_ptr = static_cast<T>(*result);
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of integral value to link
				/// \param value - a reference to the linked value
				template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
				void link_integral( boost::string_view name, daw::optional_poly<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );

					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto result = nullable_decoder_helper<int64_t>( name, json_values );
						if( result ) {
							daw::exception::daw_throw_on_false( *result <= std::numeric_limits<T>::max( ) );    // TODO determine if throwing is more appropriate
							daw::exception::daw_throw_on_false( *result >= std::numeric_limits<T>::min( ) );
						}
						*value_ptr = static_cast<T>(*result);
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of real(float/double...) value to link
				/// \param value - a reference to the linked value
				template<typename T>
				void link_real( boost::string_view name, T & value ) {
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = standard_decoder<double>( name, value );
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of real value to link
				/// \param value - a reference to the linked value
				template<typename T, typename std::enable_if_t<std::is_floating_point<T>::value, long> = 0>
				void link_real( boost::string_view name, boost::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );

					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto result = nullable_decoder_helper<double>( name, json_values );
						if( result ) {
							daw::exception::daw_throw_on_false( *result <= std::numeric_limits<T>::max( ) );    // TODO determine if throwing is more appropriate
							daw::exception::daw_throw_on_false( *result >= std::numeric_limits<T>::min( ) );
							*value_ptr = static_cast<T>(*result);
						}
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of string value to link
				/// \param value - a reference to the linked value
				void link_string( boost::string_view name, boost::optional<std::string> & value ) {
					return link_value( name, value );
				}

				///
				/// \param name - name of string value to link
				/// \param value - a reference to the linked value
				void link_string( boost::string_view name, daw::optional<std::string> & value ) {
					return link_value( name, value );
				}

				///
				/// \param name - name of string value to link
				/// \param value - a reference to the linked value
				void link_string( boost::string_view name, daw::optional_poly<std::string> & value ) {
					return link_value( name, value );
				}

				///
				/// \param name - name of string value to link
				/// \param value - a reference to the linked value
				void link_string( boost::string_view name, std::string & value ) {
					//return link_value( name, value );
					// Need to parse escaped values
					set_name( value, name );
					data_description_t data_description;
					data_description.json_type = ::daw::json::schema::get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = string_decoder( name, value );
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of boolean(true/false) value to link
				/// \param value - a reference to the linked value
				void link_boolean( boost::string_view name, bool & value ) {
					return link_value( name, value );
				}

				///
				/// \param name - name of boolean(true/false) value to link
				/// \param value - a reference to the linked value
				void link_boolean( boost::string_view name, boost::optional<bool> & value ) {
					return link_value( name, value );
				}
	
				///
				/// \param name - name of boolean(true/false) value to link
				/// \param value - a reference to the linked value
				void link_boolean( boost::string_view name, daw::optional<bool> & value ) {
					return link_value( name, value );
				}

				///
				/// \param name - name of boolean(true/false) value to link
				/// \param value - a reference to the linked value
				void link_boolean( boost::string_view name, daw::optional_poly<bool> & value ) {
					return link_value( name, value );
				}

				///
				/// \param name - name of JsonLink<type> obect value to link
				/// \param value - a reference to the linked value
				template<typename T>
				void link_object( boost::string_view name, JsonLink<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					data_description.json_type = value.get_schema_obj( );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						}
						daw::exception::daw_throw_on_false( member->second.is_object( ) );
						value_ptr->from_json_obj( member->second );
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of JsonLink<type> obect value to link
				/// \param value - a reference to the linked value
				template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
				void link_object( boost::string_view name, boost::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					data_description.json_type = (T { }).get_schema_obj( );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// do not overwrite value
							//*value_ptr = boost::none;
						} else if( member->second.is_null( ) ) {
							*value_ptr = boost::none;
						} else {
							(*value_ptr)->from_json_obj( member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of JsonLink<type> obect value to link
				/// \param value - a reference to the linked value
				template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
				void link_object( boost::string_view name, daw::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					data_description.json_type = (T { }).get_schema_obj( );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// throw std::runtime_error( "JSON object does not match expected object layout" );
							value_ptr->reset( );
						} else if( member->second.is_null( ) ) {
							value_ptr->reset( );
						} else {
							(*value_ptr)->from_json_obj( member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of JsonLink<type> obect value to link
				/// \param value - a reference to the linked value
				template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
				void link_object( boost::string_view name, daw::optional_poly<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					data_description.json_type = (T { }).get_schema_obj( );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// throw std::runtime_error( "JSON object does not match expected object layout" );
							value_ptr->reset( );
						} else if( member->second.is_null( ) ) {
							value_ptr->reset( );
						} else {
							(*value_ptr)->from_json_obj( member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
				}

				///
				/// \param name - name of array(vector) value to link
				/// \param value - a reference to the linked value
				template<typename T>
				void link_array( boost::string_view name, T & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using ::daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						}
						daw::exception::daw_throw_on_false( member->second.is_array( ) );
						using namespace parse;
						json_to_value( *value_ptr, member->second );
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				///
				/// \param name - name of array(vector) value to link
				/// \param value - a reference to the linked value
				template<typename T>
				void link_array( boost::string_view name, boost::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using ::daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// throw std::runtime_error( "JSON object does not match expected object layout" );
							// do not overwrite value
							// *value_ptr = boost::none;
						} else if( member->second.is_null( ) ) {
							*value_ptr = boost::none;
						} else {
							daw::exception::daw_throw_on_false( member->second.is_array( ) );
							using namespace parse;
							json_to_value( *value_ptr, member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}
				
				///
				/// \param name - name of array(vector) value to link
				/// \param value - a reference to the linked value
				template<typename T>
				void link_array( boost::string_view name, daw::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using ::daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// throw std::runtime_error( "JSON object does not match expected object layout" );
							value_ptr->reset( );
						} else if( member->second.is_null( ) ) {
							value_ptr->reset( );
						} else {
							daw::exception::daw_throw_on_false( member->second.is_array( ) );
							using namespace parse;
							json_to_value( *value_ptr, member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}


				///
				/// \param name - name of array(vector) value to link
				/// \param value - a reference to the linked value
				template<typename T>
				void link_array( boost::string_view name, daw::optional_poly<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using ::daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// throw std::runtime_error( "JSON object does not match expected object layout" );
							*value_ptr = daw::optional_poly<T>{ };
						} else if( member->second.is_null( ) ) {
							*value_ptr = daw::optional_poly<T>{ };
						} else {
							daw::exception::daw_throw_on_false( member->second.is_array( ) );
							using namespace parse;
							json_to_value( *value_ptr, member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				///
				/// \param name - name of map(unorderd_map/map) value to link.
				/// \param value - a reference to the linked value
				template<typename T>
				void link_map( boost::string_view name, T & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					using ::daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto val_obj = json_values.get_object( );
						auto member = val_obj.find( name );
						if( val_obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
							ss << " available members { ";
							for( auto const & m: val_obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						}
						daw::exception::daw_throw_on_false( member->second.is_array( ) );
						using namespace parse;
						json_to_value( *value_ptr, member->second );
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				///
				/// \param name - name of map(unorderd_map/map) value to link.
				/// \param value - a reference to the linked value
				template<typename T>
				void link_map( boost::string_view name, boost::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					data_description.json_type = "map?";
					using ::daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto val_obj = json_values.get_object( );
						auto member = val_obj.find( name );
						if( val_obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// throw std::runtime_error( "JSON object does not match expected object layout" );
							// do not overwrite value
							// *value_ptr = boost::none;
						} else if( member->second.is_null( ) ) {
							*value_ptr = boost::none;
						} else {
							daw::exception::daw_throw_on_false( member->second.is_array( ) );
							using namespace parse;
							json_to_value( *value_ptr, member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				///
				/// \param name - name of map(unorderd_map/map) value to link.
				/// \param value - a reference to the linked value
				template<typename T>
				void link_map( boost::string_view name, daw::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					data_description.json_type = "map?";
					using ::daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto val_obj = json_values.get_object( );
						auto member = val_obj.find( name );
						if( val_obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// throw std::runtime_error( "JSON object does not match expected object layout" );
							value_ptr->reset( );
						} else if( member->second.is_null( ) ) {
							value_ptr->reset( );
						} else {
							daw::exception::daw_throw_on_false( member->second.is_array( ) );
							using namespace parse;
							json_to_value( *value_ptr, member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}


				///
				/// \param name - name of map(unorderd_map/map) value to link.
				/// \param value - a reference to the linked value
				template<typename T>
				void link_map( boost::string_view name, daw::optional_poly<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name.to_string( ) );
					data_description_t data_description;
					data_description.json_type = "map?";
					using ::daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = standard_encoder( name, value );
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto val_obj = json_values.get_object( );
						auto member = val_obj.find( name );
						if( val_obj.end( ) == member ) {
							// TODO: determine if correct course of action
							// throw std::runtime_error( "JSON object does not match expected object layout" );
							*value_ptr = daw::optional_poly<T>{ };
						} else if( member->second.is_null( ) ) {
							*value_ptr = daw::optional_poly<T>{ };
						} else {
							daw::exception::daw_throw_on_false( member->second.is_array( ) );
							using namespace parse;
							json_to_value( *value_ptr, member->second );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				///
				/// \param name - name of streamable value(operator<<, operator>>) to link.
				/// \param value - a reference to the linked value
				template<typename T>
				void link_streamable( boost::string_view name, T & value ) {
					auto value_ptr = &value;
					set_name( value, name );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
						daw::exception::daw_throw_on_false( value_ptr );
						json_text = generate::value_to_json( name.to_string( ), boost::lexical_cast<std::string>( *value_ptr ) );
					};
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						}
						daw::exception::daw_throw_on_false( member->second.is_string( ) );
						std::stringstream ss( member->second.get_string( ) );
						auto str = ss.str( );
						ss >> *value_ptr;
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				template<typename T>
				void link_streamable( boost::string_view name, boost::optional<T> & value ) {
					auto value_ptr = &value;
					set_name( value, name );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
						daw::exception::daw_throw_on_false( value_ptr );
						if( *value_ptr ) {
							json_text = generate::value_to_json( name.to_string( ), boost::lexical_cast<std::string>( **value_ptr ) );
						} else {
							json_text = generate::value_to_json( name.to_string( ) );
						}
					};
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto const & obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							*value_ptr = boost::optional<T>{ };
						} else if( member->second.is_null( ) ) {
							*value_ptr = boost::optional<T>{ };
						} else {
							daw::exception::daw_throw_on_false( member->second.is_string( ) );
							std::stringstream ss( member->second.get_string( ) );
							auto str = ss.str( );
							ss >> **value_ptr;
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				/// Summary: Encoder Function has signature std::string( T const & ) and Decoder function has signature T( std::string const & )
				template<typename T, typename EncoderFunction, typename DecoderFunction>
				void link_jsonstring( boost::string_view name, T & value, EncoderFunction encode_function, DecoderFunction decode_function ) {
					set_name( value, name );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, std::string{ } );
					data_description.bind_functions.encode = [value_ptr = &value, name_copy = name.to_string( ), encode_function]( std::string & json_text ) {
						daw::exception::daw_throw_on_false( value_ptr );
						json_text = generate::value_to_json( name_copy, encode_function( *value_ptr ) );
					};
					data_description.bind_functions.decode = [value_ptr = &value, name_copy = name.to_string( ), decode_function]( json_obj const & json_values ) {
						daw::exception::daw_throw_on_false( value_ptr );
						auto const & obj = json_values.get_object( );
						auto member = obj.find( name_copy );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name_copy << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						}
						daw::exception::daw_throw_on_false( member->second.is_string( ) );
						*value_ptr = decode_function( member->second.get_string( ) );
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				template<typename T, typename EncoderFunction, typename DecoderFunction>
				void link_jsonintegral( boost::string_view name, T & value, EncoderFunction encode_function, DecoderFunction decode_function ) {
					set_name( value, name ); 
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, impl::value_t::integral_t{ } );
					data_description.bind_functions.encode = [value_ptr = &value, name_copy = name.to_string( ), encode_function]( std::string & json_text ) {
						daw::exception::daw_throw_on_false( value_ptr );
						json_text = generate::value_to_json( name_copy, encode_function( *value_ptr ) );
					};
					data_description.bind_functions.decode = [value_ptr = &value, name_copy = name.to_string( ), decode_function]( json_obj const & json_values ) {
						daw::exception::daw_throw_on_false( value_ptr );
						auto const & obj = json_values.get_object( );
						auto member = obj.find( name_copy );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name_copy << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						}
						daw::exception::daw_throw_on_false( member->second.is_integral( ) );
						*value_ptr = decode_function( member->second.get_integral( ) );
					};
					add_to_data_map( name, std::move( data_description ) );
				}

			public:
				template<typename Duration>
				void link_timestamp( boost::string_view name, std::chrono::time_point<std::chrono::system_clock, Duration> & ts, std::vector<std::string> const & fmts ) {
					using tp_t = std::chrono::time_point<std::chrono::system_clock, Duration>;
					auto const from_ts = [fmts]( std::string const & str ) {
						std::istringstream in;
						tp_t tp;

						for( auto const & fmt: fmts ) {
							in.str( str );
							date::parse( in, fmt, tp );
							if( !in.fail( ) ) {
								break;
							}
							in.clear();
							in.exceptions( std::ios::failbit );
						}   
						if( in.fail( ) ) {
							tp = tp_t{ };
						}
						return tp; 
					};   
					
					auto const to_ts = [fmt=fmts.front( )]( tp_t const & tp ) -> std::string {
						return date::format( fmt, tp );
					};

					return link_jsonstring( name, ts, to_ts, from_ts );
				}

			private:
				static uint8_t to_nibble( uint8_t c ) noexcept {
					// Assumes that '0' <= c <= '9' or 'a'|'A' <= c <= 'z'|'Z'
					uint8_t result = 0;
					if( c <= '9' ) {
						result = c - '0';
					} else {
						c = c | ' ';	// ensure lowercase
						result = 10 + (c - 'a');
					}
					assert( result < 16 );
					return result;
				}
			
				template<typename T>
				static std::string value_to_hex( T const & value ) {
					std::string result;
					daw::nibble_queue_gen<T, char> nq{ value };

					while( nq.can_pop( 1 ) ) {
						auto const nibble = nq.pop_front( 1 );
						assert( nibble < 16 );
						if( nibble < 10 ) {
							result += static_cast<char>('0' + nibble);
						} else {
							result += static_cast<char>('a' + (nibble-10));
						}
					}
					return result;
				}

			public:
				template<typename T>
				void link_hex_value( boost::string_view name, T & value ) {
					auto const from_hexstring = []( std::string const & str ) {
						daw::exception::daw_throw_on_false( (str.size( )/2)/sizeof( T ) == 1 );
						daw::nibble_queue_gen<T> nq;
						for( auto c: str ) {
							nq.push_back( to_nibble( c ) );
						}
						return nq.value( );
					};

					auto const to_hexstring = []( T const & v ) {
						return value_to_hex( v );
					};

					return link_jsonstring( name, value, to_hexstring, from_hexstring );
				}

				template<typename T>
				void link_hex_array( boost::string_view name, std::vector<T> & values ) {
					auto const from_hexstring = []( std::string const & str ) {
						daw::exception::daw_throw_on_false( str.size( )%(sizeof( T )*2) == 0 );
						std::vector<T> result;
						daw::nibble_queue_gen<T> nq;
						for( auto c: str ) {
							nq.push_back( to_nibble( c ), 1 );
							if( nq.full( ) ) {
								auto const v = nq.value( );
								result.push_back( v );
								nq.clear( );
							}
						}
						return result;
					};

					auto const to_hexstring = []( std::vector<T> const & arry ) {
						std::string result;
						for( auto const & v: arry ) {
							result += value_to_hex( v );	
						}
						return result;
					};
					return link_jsonstring( name, values, to_hexstring, from_hexstring );
				}
			
				template<typename Duration>
				void link_iso8601_timestamp( boost::string_view name, std::chrono::time_point<std::chrono::system_clock, Duration> & ts ) {
					static std::vector<std::string> const fmts = { "%FT%TZ", "%FT%T%Ez" };
					return link_timestamp( name, ts, fmts  );
				}

				template<typename Duration>
				void link_epoch_milliseconds_timestamp( boost::string_view name, std::chrono::time_point<std::chrono::system_clock, Duration> & ts ) {
					static auto const to_ts = []( impl::value_t::integral_t const & i ) {
						using namespace date;
						using namespace std::chrono;
						static std::chrono::system_clock::time_point const epoch{ };
						auto result = epoch + milliseconds{ i };
						return result;
					};

					static auto const from_ts = []( std::chrono::time_point<std::chrono::system_clock, Duration> const & t ) -> impl::value_t::integral_t {
						using namespace date;
						using namespace std::chrono;
						static std::chrono::system_clock::time_point const epoch{ };
						auto result = std::chrono::duration_cast<std::chrono::milliseconds>( t - epoch ).count( );
						return result;
					};
					return link_jsonintegral( name, ts, from_ts, to_ts );
				}

				///
				/// \param name - name of timestamp value(boost ptime) to link.
				/// \param value - a reference to the linked value
				void link_timestamp( boost::string_view name, boost::posix_time::ptime & value ) {
					auto value_ptr = &value;
					set_name( value, name );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, value );
					data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
						daw::exception::daw_throw_on_false( value_ptr );
						json_text = generate::value_to_json( name.to_string( ),	boost::posix_time::to_iso_extended_string( *value_ptr ) + 'Z' );
					};
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						}
						daw::exception::daw_throw_on_false( member->second.is_string( ) );
						*value_ptr = boost::posix_time::from_iso_string( member->second.get_string( ) );
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}

				///
				/// \param name - name of timestamp value(boost ptime) to link.
				/// \param value - a reference to the linked value
				void link_timestamp( boost::string_view name, boost::optional<boost::posix_time::ptime> & value ) {
					auto value_ptr = &value;
					set_name( value, name );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, boost::posix_time::ptime{ } );
					data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
						daw::exception::daw_throw_on_false( value_ptr );
						if( *value_ptr ) {
							json_text = generate::value_to_json( name.to_string( ),	boost::posix_time::to_iso_extended_string( *(*value_ptr) ) + 'Z' );
						} else {
							json_text = generate::value_to_json( name.to_string( ) );
						}
					};
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						} else if( member->second.is_null( ) ) {
							*value_ptr = boost::optional<boost::posix_time::ptime> { };
						} else {
							daw::exception::daw_throw_on_false( member->second.is_string( ) );
							*value_ptr = boost::posix_time::from_iso_string( member->second.get_string( ) );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}
				
				///
				/// \param name - name of timestamp value(boost ptime) to link.
				/// \param value - a reference to the linked value
				void link_timestamp( boost::string_view name, daw::optional<boost::posix_time::ptime> & value ) {
					auto value_ptr = &value;
					set_name( value, name );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, boost::posix_time::ptime{ } );
					data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
						daw::exception::daw_throw_on_false( value_ptr );
						if( *value_ptr ) {
							json_text = generate::value_to_json( name.to_string( ),
									boost::posix_time::to_iso_extended_string(
										*(*value_ptr) ) +
									'Z' );
						} else {
							json_text = generate::value_to_json( name.to_string( ) );
						}
					};
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						} else if( member->second.is_null( ) ) {
							*value_ptr = daw::optional<boost::posix_time::ptime>{ };
						} else {
							daw::exception::daw_throw_on_false( member->second.is_string( ) );
							*value_ptr = boost::posix_time::from_iso_string( member->second.get_string( ) );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}


				///
				/// \param name - name of timestamp value(boost ptime) to link.
				/// \param value - a reference to the linked value
				void link_timestamp( boost::string_view name, daw::optional_poly<boost::posix_time::ptime> & value ) {
					auto value_ptr = &value;
					set_name( value, name );
					data_description_t data_description;
					using daw::json::schema::get_schema;
					data_description.json_type = get_schema( name, boost::posix_time::ptime{ } );
					data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
						daw::exception::daw_throw_on_false( value_ptr );
						if( *value_ptr ) {
							json_text = generate::value_to_json( name.to_string( ),	boost::posix_time::to_iso_extended_string( *(*value_ptr) ) + 'Z' );
						} else {
							json_text = generate::value_to_json( name.to_string( ) );
						}
					};
					data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
						daw::exception::daw_throw_on_false( value_ptr );
						auto obj = json_values.get_object( );
						auto member = obj.find( name );
						if( obj.end( ) == member ) {
							std::stringstream ss;
							ss << "JSON object does not match expected object layout.  Missing member '" << name.to_string( ) << "'";
							ss << " available members { ";
							for( auto const & m: obj.container( ) ) {
								ss << "'" << m.first << "' ";
							}
							ss << "}";
							throw std::runtime_error( ss.str( ) );
						} else if( member->second.is_null( ) ) {
							*value_ptr = daw::optional_poly<boost::posix_time::ptime>{ };
						} else {
							daw::exception::daw_throw_on_false( member->second.is_string( ) );
							*value_ptr = boost::posix_time::from_iso_string( member->second.get_string( ) );
						}
					};
					add_to_data_map( name, std::move( data_description ) );
					
				}
			};    // JsonLink

			template<typename Derived>
			void json_to_value( JsonLink<Derived> & to, impl::value_t const & from ) {
				auto val = from;
				to.from_json_obj( val );
			}

			template<typename Derived>
			std::string value_to_json( boost::string_view name, JsonLink<Derived> const & obj ) {
				return details::json_name( name ) + obj.to_string( );
			}

			template<typename Derived>
			auto get_schema( boost::string_view name, JsonLink<Derived> const & obj ) {
				return obj.get_schema_obj( );
			}

			template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
			auto from_file( boost::string_view file_name, bool use_default_on_error ) {
				Derived result;
				if( !boost::filesystem::exists( file_name.data( ) ) ) {
					if( use_default_on_error ) {
						return result;
					}
					throw std::runtime_error( "file not found" );
				}
				result.from_file( file_name );
				return result;
			}

			template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
			auto array_from_string( boost::string_view data, bool use_default_on_error ) {
				std::vector<Derived> result;
				auto json = parse_json( data );
				if( !json.is_array( ) ) {
					throw std::runtime_error( "File is not an array" );
				}
				for( auto const & d: json.get_array( ) ) {
					Derived tmp;
					tmp.from_json_obj( d );
					result.push_back( std::move( tmp ) );
				}
				return result;
			}

			template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
			auto array_from_file( boost::string_view file_name, bool use_default_on_error ) {
				std::vector<Derived> result;
				if( !boost::filesystem::exists( file_name.data( ) ) ) {
					if( use_default_on_error ) {
						return result;
					}
					throw std::runtime_error( "file not found" );
				}
				std::ifstream in_file;
				in_file.open( file_name.data( ) );
				if( !in_file ) {
					throw std::runtime_error( "Could not open file" );
				}
				return array_from_string<Derived>( std::string{ std::istreambuf_iterator<char>{ in_file }, std::istreambuf_iterator<char>{ } }, use_default_on_error );
			}

			template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
			auto from_file( boost::string_view file_name ) {
				return from_file<Derived>( file_name, false );
			}
	
			template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
			auto array_from_file( boost::string_view file_name ) {
				return array_from_file<Derived>( file_name, false );
			}
			
			template<typename Derived>
			void to_file( boost::string_view file_name, JsonLink<Derived> const & obj, bool overwrite ) {
				obj.to_file( file_name, overwrite );
			}

			template<typename Derived>
			JsonLink<Derived>::~JsonLink( ) { }


			template<typename Derived>
			std::ostream & operator<<( std::ostream & os, JsonLink<Derived> const & data ) {
				os << data.to_string( );
				return os;
			}

			template<typename Derived>
			std::istream & operator>>( std::istream & is, JsonLink<Derived> & data ) {
				std::string str{ std::istreambuf_iterator<char>{ is }, std::istreambuf_iterator<char>{ } };
				data.from_string( str );
				return is;
			}

		}    // namespace json
	}    // namespace daw
