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
#include <boost/utility/string_ref.hpp>
#include <fstream>
#include <functional>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>

#include <daw/char_range/daw_char_range.h>
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
			void json_to_value( JsonLink<Derived> & to, impl::value_t const & from );

		template<typename Derived>
			std::string value_to_json( boost::string_ref name, JsonLink<Derived> const & obj );

		template<typename Derived>
			::daw::json::impl::value_t get_schema( boost::string_ref name, JsonLink<Derived> const & obj );

		namespace schema {
			::daw::json::impl::value_t get_schema( boost::string_ref name );

			::daw::json::impl::value_t get_schema( boost::string_ref name, bool const & );

			::daw::json::impl::value_t get_schema( boost::string_ref name, std::nullptr_t );

			::daw::json::impl::value_t get_schema( boost::string_ref name, std::string const & );

			::daw::json::impl::value_t get_schema( boost::string_ref name, boost::posix_time::ptime const & );

			::daw::json::impl::value_t
				make_type_obj( boost::string_ref name, ::daw::json::impl::value_t selected_type );

			template<typename Key, typename Value>
				auto get_schema( boost::string_ref name, std::pair<Key, Value> const & );
			//::daw::json::impl::value_t get_schema( boost::string_ref name, std::pair<Key, Value> const & );

			template<typename T, typename std::enable_if_t<daw::traits::is_container_not_string<T>::value, long> = 0>
				auto get_schema( boost::string_ref name, T const & );

			template<typename T, typename std::enable_if_t<std::is_floating_point<T>::value, long> = 0>
				auto get_schema( boost::string_ref name, T const & );

			template<typename T, typename std::enable_if_t<
				std::is_integral<T>::value && !std::is_same<bool, T>::value, long> = 0>
				auto get_schema( boost::string_ref name, T const & );

			template<typename T>
				auto get_schema( boost::string_ref name, boost::optional<T> const & );

			template<typename T>
				auto get_schema( boost::string_ref name, daw::optional<T> const & );

			template<typename T>
				auto get_schema( boost::string_ref name, daw::optional_poly<T> const & );

			template<typename T, typename std::enable_if_t<
				daw::traits::is_streamable<T>::value && !daw::traits::is_numeric<T>::value &&
				!std::is_same<std::string, T>::value, long> = 0>
				auto get_schema( boost::string_ref name, T const & );

			template<typename Key, typename Value>
				auto get_schema( boost::string_ref name, std::pair<Key, Value> const & ) {
					//::daw::json::impl::value_t get_schema( boost::string_ref name, std::pair<Key, Value> const & ) {
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

					auto get_schema( boost::string_ref name, T const & ) {
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

					auto get_schema( boost::string_ref name, T const & ) {
						return make_type_obj( name, ::daw::json::impl::value_t( std::string( "real" ) ) );
					}

				template<typename T, typename std::enable_if_t<
					std::is_integral<T>::value && !std::is_same<bool, T>::value, long>>

					auto get_schema( boost::string_ref name, T const & ) {
						return make_type_obj( name, ::daw::json::impl::value_t( std::string( "integer" ) ) );
					}

				template<typename T>
					auto get_schema( boost::string_ref name, boost::optional<T> const & ) {
						T t;
						auto result = get_schema( name, t );
						auto & obj = result.get_object( );
						obj.push_back( make_object_value_item( range::create_char_range( "nullable" ),
									::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
						return result;
					}

				template<typename T>
					auto get_schema( boost::string_ref name, daw::optional<T> const & ) {
						T t;
						auto result = get_schema( name, t );
						auto & obj = result.get_object( );
						obj.push_back( make_object_value_item( range::create_char_range( "nullable" ),
									::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
						return result;
					}

				template<typename T>
					auto get_schema( boost::string_ref name, daw::optional_poly<T> const & ) {
						T t;
						auto result = get_schema( name, t );
						auto & obj = result.get_object( );
						obj.push_back( make_object_value_item( range::create_char_range( "nullable" ),
									::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
						return result;
					}

				template<typename T, typename std::enable_if_t<
					daw::traits::is_streamable<T>::value && !daw::traits::is_numeric<T>::value &&
					!std::is_same<std::string, T>::value, long>>

					auto get_schema( boost::string_ref name, T const & ) {
						auto result = make_type_obj( name, ::daw::json::impl::value_t( std::string( "string" ) ) );
						auto & obj = result.get_object( );
						obj.push_back( make_object_value_item( range::create_char_range( "string_object" ),
									::daw::json::impl::value_t( std::string( "string_object" ) ) ) );
						return result;
					}
				}    // namespace schema

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
					};

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


					};    // struct data_description


					std::string m_name;
					std::map<impl::string_value, data_description_t> m_data_map;

					///
					/// \param name - name of integral value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T> JsonLink & link_value( boost::string_ref name, T & value ) {
						set_name( value, name );
						data_description_t data_description;
						data_description.json_type = ::daw::json::schema::get_schema( name, value );
						data_description.bind_functions = standard_bind_functions( name, value );
						m_data_map[range::create_char_range( name )] = std::move( data_description );
						return *this;
					}

					public:
					virtual ~JsonLink( );

					JsonLink( std::string name = "" ):
						m_name( std::move( name ) ),
						m_data_map( ) { }    // TODO: look into moving set_links call into here

					JsonLink( JsonLink const & ) = default;

					JsonLink & operator=( JsonLink const & ) = default;

					JsonLink( JsonLink && ) = default;

					JsonLink & operator=( JsonLink && ) = default;

					std::string & json_object_name( ) {
						return m_name;
					}

					std::string const & json_object_name( ) const {
						return m_name;
					}

					auto get_schema_obj( ) const {
						::daw::json::impl::object_value result;
						using mapped_value_t = typename decltype( m_data_map )::value_type;
						std::transform( std::begin( m_data_map ), std::end( m_data_map ), std::back_inserter( result ),
								[]( mapped_value_t const & value ) {
								return ::daw::json::impl::make_object_value_item( value.first,
										value.second.json_type );
								} );
						return ::daw::json::impl::value_t( std::move( result ) );
					}

					std::string encode( ) const {
						std::stringstream result;
						std::string tmp;
						auto range = daw::range::make_range( m_data_map );
						if( !range.empty( ) ) {
							auto const & enc = range.front( ).second.bind_functions.encode;
							enc( tmp );
							result << tmp;
							range.move_next( );
							for( auto const & value : range ) {
								value.second.bind_functions.encode( tmp );
								result << ", " << tmp;
							}
						}
						return details::json_name( m_name ) + details::enbrace( result.str( ) );
					}

					void encode_file( boost::string_ref filename ) const {
						std::ofstream out_file{ filename.data( ) };
						if( !out_file.is_open( ) ) {
							throw std::runtime_error( "Could not open file for writing" );
						}
						out_file << encode( );
						out_file.close( );
					}

					private:
					Derived & derived( ) {
						return *static_cast<Derived *>(this);
					}

					Derived const & derived( ) const {
						return *static_cast<Derived *>(this);
					}

					public:

					auto & decode( json_obj const & json_values ) {
						for( auto & value : m_data_map ) {
							value.second.bind_functions.decode( json_values );
						}
						return derived( );
					}

					auto & decode( boost::string_ref const json_text ) {
						auto tmp = parse_json( json_text );
						decode( std::move( tmp ) );
						return derived( );
					}

					auto & decode( char const *json_text_begin, char const *json_text_end ) {
						decode( parse_json( json_text_begin, json_text_end ) );
						return derived( );
					}

					auto & decode_file( boost::string_ref filename ) {
						daw::filesystem::MemoryMappedFile<char> const test_data( filename );
						decode( test_data.begin( ), test_data.end( ) );
						return derived( );
					}

					void reset_jsonlink( ) {
						m_data_map.clear( );
						m_name.clear( );
					}

					template<typename T>
						static void call_decode( T &, json_obj ) { }

					static void call_decode( JsonLink & obj, json_obj json_values ) {
						obj.decode( std::move( json_values ) );
					}

					template<typename T>
						static void set_name( T &, boost::string_ref ) { }

					static void set_name( JsonLink & obj, boost::string_ref name ) {
						obj.json_object_name( ) = name.to_string( );
					}

					template<typename T>
						static encode_function_t standard_encoder( boost::string_ref name, T const & value ) {
							auto value_ptr = &value;
							auto name_copy = name.to_string( );
							return [value_ptr, name_copy]( std::string & json_text ) {
								assert( value_ptr );
								using namespace generate;
								json_text = value_to_json( name_copy, *value_ptr );
							};
						}

					template<typename T>
						static T decoder_helper( boost::string_ref name, json_obj const & json_values ) {
							auto obj = json_values.get_object( );
							auto member = obj.find( name );
							if( obj.end( ) == member ) {
								// TODO: determine if correct course of action
								throw std::runtime_error( "JSON object does not match expected object layout" );
							}
							return get<T>( member->second );
						}

					template<typename T>
						static boost::optional<T> nullable_decoder_helper( boost::string_ref name, json_obj const & json_values ) {
							auto obj = json_values.get_object( );
							auto member = obj.find( name );
							if( obj.end( ) == member ) {
								// TODO: determine if correct course of action
								throw std::runtime_error( "JSON object does not match expected object layout" );
							}
							if( member->second.is_null( ) ) {
								return boost::optional<T>( );
							}
							return boost::optional<T>( get<T>( member->second ) );
						}

					template<typename T, typename U = T>
						static decode_function_t standard_decoder( boost::string_ref name, T & value ) {
							auto value_ptr = &value;
							auto name_copy = name.to_string( );
							return [value_ptr, name_copy]( json_obj json_values ) mutable {
								assert( value_ptr );
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

					static std::string unescape_string( boost::string_ref src ) {
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
						static decode_function_t string_decoder( boost::string_ref name, T & value ) {
							auto value_ptr = &value;
							auto name_copy = name.to_string( );
							return [value_ptr, name_copy]( json_obj json_values ) mutable {
								assert( value_ptr );
								auto new_val = decoder_helper<U>( name_copy, json_values );
								*value_ptr = unescape_string( new_val );
							};
						}

					template<typename T, typename U = T>
						static decode_function_t standard_decoder( boost::string_ref name, boost::optional<T> & value ) {
							auto value_ptr = &value;
							auto name_copy = name.to_string( );
							return [value_ptr, name_copy]( json_obj json_values ) mutable {
								assert( value_ptr );
								auto new_val = nullable_decoder_helper<U>( name_copy, json_values );
								*value_ptr = new_val;
							};
						}

					template<typename T, typename U = T>
						static decode_function_t standard_decoder( boost::string_ref name, daw::optional<T> & value ) {
							auto value_ptr = &value;
							auto name_copy = name.to_string( );
							return [value_ptr, name_copy]( json_obj json_values ) mutable {
								assert( value_ptr );
								auto new_val = nullable_decoder_helper<U>( name_copy, json_values );
								if( new_val ) {
									*value_ptr = std::move( new_val );
								} else {
									*value_ptr.reset( );
								}
							};
						}

					template<typename T, typename U = T>
						static decode_function_t standard_decoder( boost::string_ref name, daw::optional_poly<T> & value ) {
							auto value_ptr = &value;
							auto name_copy = name.to_string( );
							return [value_ptr, name_copy]( json_obj json_values ) mutable {
								assert( value_ptr );
								auto new_val = nullable_decoder_helper<U>( name_copy, json_values );
								if( new_val ) {
									*value_ptr = std::move( new_val );
								} else {
									*value_ptr.reset( );
								}
							};
						}

					template<typename T>
						static bind_functions_t standard_bind_functions( boost::string_ref name, T & value ) {
							bind_functions_t bind_functions;
							bind_functions.encode = standard_encoder( name, value );
							bind_functions.decode = standard_decoder( name, value );
							return bind_functions;
						}

					///
					/// \param name - name of integral value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
						JsonLink & link_integral( boost::string_ref name, T & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );

							data_description.bind_functions.encode = standard_encoder( name, value );

							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto result = decoder_helper<int64_t>( name, json_values );
								assert( result <= std::numeric_limits<T>::max( ) );
								assert( result >= std::numeric_limits<T>::min( ) );
								*value_ptr = static_cast<T>(result);
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of value to remove link from
					/// \return - whether the linked name was found
					bool unlink( boost::string_ref name ) {
						return m_data_map.erase( range::create_char_range( name ) ) > 0;
					}


					///
					/// \param name - name of integral value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
						JsonLink & link_integral( boost::string_ref name, boost::optional<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );

							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto result = nullable_decoder_helper<int64_t>( name, json_values );
								if( result ) {
									assert( *result <=
											std::numeric_limits<T>::max( ) );    // TODO determine if throwing is more appropriate
									assert( *result >= std::numeric_limits<T>::min( ) );
								}
								*value_ptr = static_cast<T>(*result);
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}


					///
					/// \param name - name of integral value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
						JsonLink & link_integral( boost::string_ref name, daw::optional<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );

							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto result = nullable_decoder_helper<int64_t>( name, json_values );
								if( result ) {
									assert( *result <=
											std::numeric_limits<T>::max( ) );    // TODO determine if throwing is more appropriate
									assert( *result >= std::numeric_limits<T>::min( ) );
								}
								*value_ptr = static_cast<T>(*result);
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of integral value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T, typename std::enable_if_t<std::is_integral<T>::value, long> = 0>
						JsonLink & link_integral( boost::string_ref name, daw::optional_poly<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );

							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto result = nullable_decoder_helper<int64_t>( name, json_values );
								if( result ) {
									assert( *result <=
											std::numeric_limits<T>::max( ) );    // TODO determine if throwing is more appropriate
									assert( *result >= std::numeric_limits<T>::min( ) );
								}
								*value_ptr = static_cast<T>(*result);
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of real(float/double...) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_real( boost::string_ref name, T & value ) {
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = standard_decoder<double>( name, value );
							//m_data_map[name.to_string( )] = std::move( data_description );
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of string value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_string( boost::string_ref name, boost::optional<std::string> & value ) {
						return link_value( name, value );
					}

					///
					/// \param name - name of string value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_string( boost::string_ref name, daw::optional<std::string> & value ) {
						return link_value( name, value );
					}

					///
					/// \param name - name of string value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_string( boost::string_ref name, daw::optional_poly<std::string> & value ) {
						return link_value( name, value );
					}

					///
					/// \param name - name of string value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_string( boost::string_ref name, std::string & value ) {
						//return link_value( name, value );
						// Need to parse escaped values
						set_name( value, name );
						data_description_t data_description;
						data_description.json_type = ::daw::json::schema::get_schema( name, value );
						data_description.bind_functions.encode = standard_encoder( name, value );
						data_description.bind_functions.decode = string_decoder( name, value );
						m_data_map[range::create_char_range( name )] = std::move( data_description );
						return *this;
					}

					///
					/// \param name - name of boolean(true/false) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_boolean( boost::string_ref name, bool & value ) {
						return link_value( name, value );
					}

					///
					/// \param name - name of boolean(true/false) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_boolean( boost::string_ref name, boost::optional<bool> & value ) {
						return link_value( name, value );
					}
		
					///
					/// \param name - name of boolean(true/false) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_boolean( boost::string_ref name, daw::optional<bool> & value ) {
						return link_value( name, value );
					}

					///
					/// \param name - name of boolean(true/false) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_boolean( boost::string_ref name, daw::optional_poly<bool> & value ) {
						return link_value( name, value );
					}

					///
					/// \param name - name of JsonLink<type> obect value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_object( boost::string_ref name, JsonLink<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							data_description.json_type = value.get_schema_obj( );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								assert( member->second.is_object( ) );
								value_ptr->decode( member->second );
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of JsonLink<type> obect value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
						JsonLink & link_object( boost::string_ref name, boost::optional<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							data_description.json_type = (T { }).get_schema_obj( );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								if( member->second.is_null( ) ) {
									*value_ptr = boost::optional<T>( );
								} else {
									(*value_ptr)->decode( member->second );
								}
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of JsonLink<type> obect value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
						JsonLink & link_object( boost::string_ref name, daw::optional<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							data_description.json_type = (T { }).get_schema_obj( );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								if( member->second.is_null( ) ) {
									value_ptr->reset( );
								} else {
									(*value_ptr)->decode( member->second );
								}
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of JsonLink<type> obect value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T, typename std::enable_if_t<std::is_base_of<JsonLink<T>, T>::value, long> = 0>
						JsonLink & link_object( boost::string_ref name, daw::optional_poly<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							data_description.json_type = (T { }).get_schema_obj( );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								if( member->second.is_null( ) ) {
									value_ptr->reset( );
								} else {
									(*value_ptr)->decode( member->second );
								}
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of array(vector) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_array( boost::string_ref name, T & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using ::daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								assert( member->second.is_array( ) );
								using namespace parse;
								json_to_value( *value_ptr, member->second );
							};
							//m_data_map[name.to_string( )] = std::move( data_description );
							m_data_map[range::create_char_range( name )] = std::move( data_description );

							return *this;
						}

					///
					/// \param name - name of array(vector) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_array( boost::string_ref name, boost::optional<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using ::daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								if( member->second.is_null( ) ) {
									*value_ptr = boost::optional<T>( );
								} else {
									assert( member->second.is_array( ) );
									using namespace parse;
									json_to_value( *value_ptr, member->second );
								}
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}
					
					///
					/// \param name - name of array(vector) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_array( boost::string_ref name, daw::optional<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using ::daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								if( member->second.is_null( ) ) {
									*value_ptr = daw::optional<T>{ };
								} else {
									assert( member->second.is_array( ) );
									using namespace parse;
									json_to_value( *value_ptr, member->second );
								}
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}


					///
					/// \param name - name of array(vector) value to link
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_array( boost::string_ref name, daw::optional_poly<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using ::daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								if( member->second.is_null( ) ) {
									*value_ptr = daw::optional_poly<T>{ };
								} else {
									assert( member->second.is_array( ) );
									using namespace parse;
									json_to_value( *value_ptr, member->second );
								}
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of map(unorderd_map/map) value to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_map( boost::string_ref name, T & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							using ::daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto val_obj = json_values.get_object( );
								auto member = val_obj.find( name );
								if( val_obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								assert( member->second.is_array( ) );
								using namespace parse;
								json_to_value( *value_ptr, member->second );
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of map(unorderd_map/map) value to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_map( boost::string_ref name, boost::optional<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							data_description.json_type = "map?";
							using ::daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto val_obj = json_values.get_object( );
								auto member = val_obj.find( name );
								if( val_obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								} else {
									if( member->second.is_null( ) ) {
										*value_ptr = boost::optional<T>( );
									} else {
										assert( member->second.is_array( ) );
										using namespace parse;
										json_to_value( *value_ptr, member->second );
									}
								}
							};
							//m_data_map[name.to_string( )] = std::move( data_description );
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of map(unorderd_map/map) value to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_map( boost::string_ref name, daw::optional<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							data_description.json_type = "map?";
							using ::daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto val_obj = json_values.get_object( );
								auto member = val_obj.find( name );
								if( val_obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								} else {
									if( member->second.is_null( ) ) {
										*value_ptr = daw::optional<T>{ };
									} else {
										assert( member->second.is_array( ) );
										using namespace parse;
										json_to_value( *value_ptr, member->second );
									}
								}
							};
							//m_data_map[name.to_string( )] = std::move( data_description );
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}


					///
					/// \param name - name of map(unorderd_map/map) value to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_map( boost::string_ref name, daw::optional_poly<T> & value ) {
							auto value_ptr = &value;
							set_name( value, name.to_string( ) );
							data_description_t data_description;
							data_description.json_type = "map?";
							using ::daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = standard_encoder( name, value );
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto val_obj = json_values.get_object( );
								auto member = val_obj.find( name );
								if( val_obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								} else {
									if( member->second.is_null( ) ) {
										*value_ptr = daw::optional_poly<T>{ };
									} else {
										assert( member->second.is_array( ) );
										using namespace parse;
										json_to_value( *value_ptr, member->second );
									}
								}
							};
							//m_data_map[name.to_string( )] = std::move( data_description );
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of streamable value(operator<<, operator>>) to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					template<typename T>
						JsonLink & link_streamable( boost::string_ref name, T & value ) {
							auto value_ptr = &value;
							set_name( value, name );
							data_description_t data_description;
							using daw::json::schema::get_schema;
							data_description.json_type = get_schema( name, value );
							data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
								assert( value_ptr );
								json_text = generate::value_to_json( name.to_string( ), boost::lexical_cast<std::string>( *value_ptr ) );
							};
							data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
								assert( value_ptr );
								auto obj = json_values.get_object( );
								auto member = obj.find( name );
								if( obj.end( ) == member ) {
									// TODO: determine if correct course of action
									throw std::runtime_error( "JSON object does not match expected object layout" );
								}
								assert( member->second.is_string( ) );
								std::stringstream ss( member->second.get_string( ) );
								auto str = ss.str( );
								ss >> *value_ptr;
							};
							m_data_map[range::create_char_range( name )] = std::move( data_description );
							return *this;
						}

					///
					/// \param name - name of timestamp value(boost ptime) to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_timestamp( boost::string_ref name, boost::posix_time::ptime & value ) {
						auto value_ptr = &value;
						set_name( value, name );
						data_description_t data_description;
						using daw::json::schema::get_schema;
						data_description.json_type = get_schema( name, value );
						data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
							assert( value_ptr );
							json_text = generate::value_to_json( name.to_string( ),
									boost::posix_time::to_iso_extended_string( *value_ptr ) +
									'Z' );
						};
						data_description.bind_functions.decode = [value_ptr, name]( json_obj const & json_values ) mutable {
							assert( value_ptr );
							auto obj = json_values.get_object( );
							auto member = obj.find( name );
							if( obj.end( ) == member ) {
								// TODO: determine if correct course of action
								throw std::runtime_error( "JSON object does not match expected object layout" );
							}
							assert( member->second.is_string( ) );
							*value_ptr = boost::posix_time::from_iso_string( member->second.get_string( ) );
						};
						m_data_map[range::create_char_range( name )] = std::move( data_description );
						return *this;
					}

					///
					/// \param name - name of timestamp value(boost ptime) to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_timestamp( boost::string_ref name, boost::optional<boost::posix_time::ptime> & value ) {
						auto value_ptr = &value;
						set_name( value, name );
						data_description_t data_description;
						using daw::json::schema::get_schema;
						data_description.json_type = get_schema( name, boost::posix_time::ptime{ } );
						data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
							assert( value_ptr );
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
							assert( value_ptr );
							auto obj = json_values.get_object( );
							auto member = obj.find( name );
							if( obj.end( ) == member ) {
								// TODO: determine if correct course of action
								throw std::runtime_error( "JSON object does not match expected object layout" );
							}
							if( member->second.is_null( ) ) {
								*value_ptr = boost::optional<boost::posix_time::ptime> { };
							} else {
								assert( member->second.is_string( ) );
								*value_ptr = boost::posix_time::from_iso_string( member->second.get_string( ) );
							}
						};
						m_data_map[range::create_char_range( name )] = std::move( data_description );
						return *this;
					}
					
					///
					/// \param name - name of timestamp value(boost ptime) to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_timestamp( boost::string_ref name, daw::optional<boost::posix_time::ptime> & value ) {
						auto value_ptr = &value;
						set_name( value, name );
						data_description_t data_description;
						using daw::json::schema::get_schema;
						data_description.json_type = get_schema( name, boost::posix_time::ptime{ } );
						data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
							assert( value_ptr );
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
							assert( value_ptr );
							auto obj = json_values.get_object( );
							auto member = obj.find( name );
							if( obj.end( ) == member ) {
								// TODO: determine if correct course of action
								throw std::runtime_error( "JSON object does not match expected object layout" );
							}
							if( member->second.is_null( ) ) {
								*value_ptr = daw::optional<boost::posix_time::ptime>{ };
							} else {
								assert( member->second.is_string( ) );
								*value_ptr = boost::posix_time::from_iso_string( member->second.get_string( ) );
							}
						};
						m_data_map[range::create_char_range( name )] = std::move( data_description );
						return *this;
					}


					///
					/// \param name - name of timestamp value(boost ptime) to link.
					/// \param value - a reference to the linked value
					/// \return - Returns a reference to self
					JsonLink & link_timestamp( boost::string_ref name, daw::optional_poly<boost::posix_time::ptime> & value ) {
						auto value_ptr = &value;
						set_name( value, name );
						data_description_t data_description;
						using daw::json::schema::get_schema;
						data_description.json_type = get_schema( name, boost::posix_time::ptime{ } );
						data_description.bind_functions.encode = [value_ptr, name]( std::string & json_text ) {
							assert( value_ptr );
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
							assert( value_ptr );
							auto obj = json_values.get_object( );
							auto member = obj.find( name );
							if( obj.end( ) == member ) {
								// TODO: determine if correct course of action
								throw std::runtime_error( "JSON object does not match expected object layout" );
							}
							if( member->second.is_null( ) ) {
								*value_ptr = daw::optional_poly<boost::posix_time::ptime>{ };
							} else {
								assert( member->second.is_string( ) );
								*value_ptr = boost::posix_time::from_iso_string( member->second.get_string( ) );
							}
						};
						m_data_map[range::create_char_range( name )] = std::move( data_description );
						return *this;
					}


				};    // class JsonLink

			template<typename Derived>
				void json_to_value( JsonLink<Derived> & to, impl::value_t const & from ) {
					auto val = from;
					to.decode( val );
				}

			template<typename Derived>
				std::string value_to_json( boost::string_ref name, JsonLink<Derived> const & obj ) {
					return details::json_name( name ) + obj.encode( );
				}

			template<typename Derived>
				auto get_schema( boost::string_ref name, JsonLink<Derived> const & obj ) {
					return obj.get_schema_obj( );
				}

			template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
			auto from_file( boost::string_ref file_name, bool use_default_on_error ) {
				Derived result;
				if( !boost::filesystem::exists( file_name.data( ) ) ) {
					if( use_default_on_error ) {
						return result;
					}
					throw std::runtime_error( "file not found" );
				}
				result.decode_file( file_name );
				return result;
			}

			template<typename Derived, typename = std::enable_if<std::is_base_of<JsonLink<Derived>, Derived>::value>>
			auto from_file( boost::string_ref file_name ) {
				return from_file<Derived>( file_name, false );
			}

			template<typename Derived>
				JsonLink<Derived>::~JsonLink( ) { }
		}    // namespace json
	}    // namespace daw
