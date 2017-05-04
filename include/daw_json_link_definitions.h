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
			template<typename Derived, typename GetFunction>
			struct standard_encoder_t final {
				std::string name_copy;
				GetFunction get_function;

				standard_encoder_t( boost::string_view n, GetFunction func ):
						name_copy{ n.to_string( ) },
						get_function{ std::move( func ) } { }

				standard_encoder_t( ) = delete;

				~standard_encoder_t( ) = default;
				standard_encoder_t( standard_encoder_t const & ) = default;
				standard_encoder_t( standard_encoder_t && ) noexcept = default;
				standard_encoder_t & operator=( standard_encoder_t && ) noexcept = default;

				standard_encoder_t & operator=( standard_encoder_t const & rhs ) {
					return *this = standard_encoder_t{ rhs };
				}

				void operator( )( std::string & json_text ) const;
			};	// standard_encoder_t
			

			template<typename Derived, typename GetFunction>
			standard_encoder_t<Derived, GetFunction> make_standard_encoder( boost::string_view name, GetFunction get_function ) {
				return standard_encoder_t<Derived, GetFunction>{ name, get_function };
			}

			namespace schema {
				namespace impl {
					value_t make_type_obj( boost::string_view name, value_t selected_type ) {
						object_value result;
						if( !name.empty( ) ) {
							result.push_back( make_object_value_item( range::create_char_range( "name" ), value_t( name ) ) );
						}
						result.push_back( make_object_value_item( range::create_char_range( "type" ), std::move( selected_type ) ) );

						return value_t( std::move( result ) );
					}

					template<typename T, typename = long>
					struct schema_t { };

					template<>
					struct schema_t<std::nullptr_t> { 
						value_t operator( )( boost::string_view name ) const {
							return make_type_obj( name, "null" );
						}
					};

					template<>
					struct schema_t<bool> { 
						value_t operator( )( boost::string_view name ) const {
							return make_type_obj( name, "bool" );
						}
					};

					template<>
					struct schema_t<std::string> { 
						value_t operator( )( boost::string_view name ) const {
							return make_type_obj( name, "string" );
						}
					};

					template<>
					struct schema_t<boost::posix_time::ptime> { 
						value_t operator( )( boost::string_view name ) const {
							return make_type_obj( name, "string" );
						}
					};

					template<typename Key, typename Value>
					struct schema_t<std::pair<Key, Value>> {
						value_t operator( )( boost::string_view name ) const {
							using ::daw::json::impl::make_object_value_item;
							::daw::json::impl::object_value result;
							result.push_back( impl::make_object_value_item( "key", get_schema<Key>( "key" ) ) );
							result.push_back( impl::make_object_value_item( "value", get_schema<Value>( "value"  ) ) );
							return make_type_obj( name, ::daw::json::impl::value_t{ std::move( result ) } );
						}
					};

					template<typename T, typename std::enable_if_t<daw::traits::is_container_not_string_v<T>, long>>
					struct schema_t { 
						value_t operator( )( boost::string_view name ) {
							using ::daw::json::impl::make_object_value_item;
							daw::json::impl::object_value result;

							auto const obj_type = std::make_pair( "type", daw::traits::is_map_like<T>::value ? "map": "array" );
							result.push_back( obj_type );
							result.push_back( impl::make_object_value_item( "element_type", get_schema<typename T::value_type>( "" ) ) ); 
							return make_type_obj( name, value_t{ std::move( result ) } );
						}
					};

					template<typename T, typename std::enable_if_t<std::is_floating_point<T>::value, long>>
					struct schema_t {
						value_t operator( )( boost::string_view name ) {
							return make_type_obj( name, "real"  );
						}
					};

					template<typename T, typename std::enable_if_t<std::is_integral<T>::value && !std::is_same<bool, T>::value, long>>
					::daw::json::impl::value_t get_schema( boost::string_view name, T const & ) {
						return make_type_obj( name, ::daw::json::impl::value_t( std::string( "integer" ) ) );
					}

					template<typename T>
					::daw::json::impl::value_t get_schema( boost::string_view name, boost::optional<T> const & ) {
						T t;
						auto result = get_schema( name, t );
						auto & obj = result.get_object( );
						obj.push_back( impl::make_object_value_item( range::create_char_range( "nullable" ),
									::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
						return result;
					}

					template<typename T>
					::daw::json::impl::value_t get_schema( boost::string_view name, daw::optional<T> const & ) {
						T t;
						auto result = get_schema( name, t );
						auto & obj = result.get_object( );
						obj.push_back( impl::make_object_value_item( range::create_char_range( "nullable" ),
									::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
						return result;
					}

					template<typename T>
					::daw::json::impl::value_t get_schema( boost::string_view name, daw::optional_poly<T> const & ) {
						T t;
						auto result = get_schema( name, t );
						auto & obj = result.get_object( );
						obj.push_back( impl::make_object_value_item( range::create_char_range( "nullable" ),
									::daw::json::impl::value_t( std::string( "nullable" ) ) ) );
						return result;
					}

					template<typename T, typename std::enable_if_t<daw::traits::is_streamable<T>::value && !daw::traits::is_numeric<T>::value && !std::is_same<std::string, T>::value, long>>
					::daw::json::impl::value_t get_schema( boost::string_view name, T const & ) {
						auto result = make_type_obj( name, ::daw::json::impl::value_t( std::string( "string" ) ) );
						auto & obj = result.get_object( );
						obj.push_back( impl::make_object_value_item( range::create_char_range( "string_object" ),
									::daw::json::impl::value_t( std::string( "string_object" ) ) ) );
						return result;
					}

					template<typename Derived, typename GetFunction>
					::daw::json::impl::value_t get_schema( boost::string_view name, GetFunction get_function ) {
						using value_t = std::decay_t<decltype( get_function( std::declval<Derived>( ) ) )>;
						value_t v;
						return get_schema( name, v );
					}


				}    // namespace impl


				template<typename GetFunction>
				auto get_schema( boost::string_view name, 
			}    // namespace schema

		}	// namespace impl

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

		/// DAW
		/*
		template<typename Derived> template<typename GetFunction, typename ResultOfGetFunction = std::decay_t<decltype(std::declval<GetFunction>( )( std::declval<Derived>( ) ) )>>
		impl::encode_function_t<Derived>::encode_function_t( boost::string_view n, GetFunction func ):
				name_copy{ n.to_string( ) },
				get_funcion{ std::move( func ) } { }
		template<typename Derived> 
		void impl::encode_function_t<Derived>::operator( )( std::string & json_text ) const {
			using namespace generate;
			json_text = value_to_json( name_copy, get_function );
		}
		*/

		template<typename Derived>
		impl::bind_functions_t<Derived>::bind_functions_t( ):
				encode{ nullptr },
				decode{ nullptr } { }

		template<typename Derived>
		impl::data_description_t<Derived>::data_description_t( ):
				json_type{ nullptr },
				bind_functions{ } { }

		template<typename Derived>
		impl::data_t<Derived>::data_t( boost::string_view name ):
				m_name{ name.to_string( ) },
				m_data_map{ } { }

		/*
		template<typename Derived> template<typename SerializeFunction, typename DeserializeFunction> 
		void JsonLink<Derived>::json_link_value( boost::string_view name, SerializeFunction serialize_function, DeserializeFunction deserialize_function ) {
			using source_value_t = std::result_of_t<DeserializeFunction( Derived & )>;
			set_name( value, name );
			impl::data_description_t<Derived> data_description;
			data_description.json_type = ::daw::json::impl::schema::get_schema( name, value );
			data_description.bind_functions = standard_bind_functions( name, value );
			add_to_data_map( name, std::move( data_description ) );
		}
		*/

		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_value( boost::string_view name, GetFunction get_function ) {
			using value_t = std::decay_t<decltype( std::declval<GetFunction>( )( std::declval<Derived>( ) ) )>;
			value_t value;	
			set_name( value, name );
			impl::data_description_t<Derived> data_description;
			data_description.json_type = ::daw::json::impl::schema::get_schema( name, value );
			data_description.bind_functions = standard_bind_functions<Derived>( name, value );
			add_to_data_map( name, std::move( data_description ) );

		}

		template<typename Derived>
		bool JsonLink<Derived>::is_linked( impl::string_value name ) {
			return m_data.m_data_map.count( name ) != 0;
		}

		template<typename Derived>
		JsonLink<Derived>::~JsonLink( ) noexcept { }

		template<typename Derived>
		std::string const & JsonLink<Derived>::json_object_name( ) {
			return m_data.m_name;
		}

		template<typename Derived>
		::daw::json::impl::value_t JsonLink<Derived>::get_schema_obj( ) {
			::daw::json::impl::object_value result;
			using mapped_value_t = typename decltype( m_data.m_data_map )::value_type;
			std::transform( std::begin( m_data.m_data_map ), std::end( m_data.m_data_map ), std::back_inserter( result ),
					[]( mapped_value_t const & value ) {
					return ::daw::json::impl::make_object_value_item( value.first,
							value.second.json_type );
					} );
			return ::daw::json::impl::value_t( std::move( result ) );
		}

		template<typename Derived>
		std::string JsonLink<Derived>::to_json_string( ) const {
			std::stringstream result;
			auto range = daw::range::make_range( m_data.m_data_map );
			std::string tmp;

			range.front( ).second.bind_functions.encode( tmp );
			result << tmp;
			range.move_next( );

			for( auto const & value : range ) {
				value.second.bind_functions.encode( tmp );
				result << ", " << tmp;
			}
			return details::json_name( m_data.m_name ) + details::enbrace( result.str( ) );
		}
			
		template<typename Derived>
		void JsonLink<Derived>::from_json_obj( json_obj const & json_values ) {
			for( auto & value : m_data.m_data_map ) {
				value.second.bind_functions.decode( json_values );
			}
		}

		template<typename Derived>
		void JsonLink<Derived>::from_json_string( boost::string_view const json_text ) {
			auto tmp = parse_json( json_text );
			from_json_obj( std::move( tmp ) );
		}

		template<typename Derived>
		void JsonLink<Derived>::from_json_string( char const *json_text_begin, char const *json_text_end ) {
			auto tmp = parse_json( json_text_begin, json_text_end );
			from_json_obj( std::move( tmp ) );
		}

		template<typename Derived>
		void JsonLink<Derived>::to_json_file( boost::string_view filename, bool overwrite ) const {
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
			out_file << to_json_string( );
			out_file.close( );
		}

		template<typename Derived>
		void JsonLink<Derived>::from_json_file( boost::string_view filename ) {
			std::ifstream in_file;
			in_file.open( filename.data( ) );
			if( !in_file ) {
				throw std::runtime_error( "Could not open file" );
			}
			std::string const data{ std::istreambuf_iterator<char>{ in_file }, std::istreambuf_iterator<char>{ } };
			in_file.close( );
			from_json_string( data );	
		}

		template<typename Derived> template<typename T>
		void JsonLink<Derived>::call_decode( T &, json_obj ) { }

		template<typename Derived>
		void JsonLink<Derived>::call_decode( JsonLink<Derived> & obj, json_obj json_values ) {
			obj.from_json_obj( std::move( json_values ) );
		}

		template<typename Derived>
		void JsonLink<Derived>::set_name( JsonLink & obj, boost::string_view name ) {
			obj.json_object_name( ) = name.to_string( );
		}

		template<typename Derived> template<typename T>
		T JsonLink<Derived>::decoder_helper( boost::string_view name, json_obj const & json_values ) {
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

		template<typename Derived> template<typename T>
		boost::optional<T> JsonLink<Derived>::nullable_decoder_helper( boost::string_view name, json_obj const & json_values ) {
			auto obj = json_values.get_object( );
			auto member = obj.find( name );
			if( obj.end( ) == member || member->second.is_null( ) ) {
				return boost::none;
			}
			return get<T>( member->second );
		}

		template<typename Derived> template<typename GetFunction>
		impl::decode_function_t<Derived> JsonLink<Derived>::standard_decoder( boost::string_view name, GetFunction get_function ) {
			return [func = std::move( get_function ), name_copy = name.to_string( )]( Derived & derived_obj, json_obj json_values ) mutable {
				auto new_val = decoder_helper<Derived, GetFunction>( name_copy, json_values );
				func( derived_obj ) = new_val;
			};
		}

		template<typename Derived> template<typename T>
		uint8_t hex_to_integer( T && value ) {
			if( 'A' <= value && value <= 'F' ) {
				return static_cast<uint8_t>((value - 'A') + 10);
			} else if( 'a' <= value && value <= 'a' ) {
				return static_cast<uint8_t>((value - 'a') + 10);
			} else if( '0' <= value && value <= '9' ) {
				return static_cast<uint8_t>(value - '0');
			}
			throw std::runtime_error( "Unicode escape sequence was not properly formed" );
		}

		template<typename Derived> template<typename ForwardIterator, typename T>
		ForwardIterator JsonLink<Derived>::get_cp( ForwardIterator first, ForwardIterator last, T & out ) {
			auto count = sizeof( out );
			daw::nibble_queue_gen<uint16_t, uint16_t> nibbles;
			auto it = first;
			for( ; it != (first + count) && it != last; ++it ) {
				nibbles.push_back( hex_to_integer( *it ) );
			}
			if( nibbles.full( ) ) {
				throw std::runtime_error( "Unicode escape sequence was not properly formed" );
			}
			out = nibbles.pop_front( );
			return it;
		}

		template<typename Derived>
		std::vector<uint8_t> JsonLink<Derived>::ucs2_to_utf8( uint16_t ucs2 ) {
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
			/* 32bit input
			} else if( ucs2 >= 0x10000 && ucs2 < 0x10FFFF ) {
				// http://tidy.sourceforge.net/cgi-bin/lxr/source/src/utf8.c#L380 
				result.push_back( static_cast<uint8_t>(0xF0 | (ucs2 >> 18)) );
				result.push_back( static_cast<uint8_t>(0x80 | ((ucs2 >> 12) & 0x3F)) );
				result.push_back( static_cast<uint8_t>(0x80 | ((ucs2 >> 6) & 0x3F)) );
				result.push_back( static_cast<uint8_t>(0x80 | ((ucs2 & 0x3F))) );
			*/
			} else {
				throw std::runtime_error( "Bad input" );
			}
			return result;
		}
		
		template<typename Derived>
		std::string JsonLink<Derived>::unescape_string( boost::string_view src ) {
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


		template<typename Derived> template<typename GetFunction>
		impl::decode_function_t<Derived> JsonLink<Derived>::string_decoder( boost::string_view name, GetFunction get_function ) {
			auto name_copy = name.to_string( );
			return [get_function, name_copy]( Derived & derived_obj, json_obj json_values ) mutable {
				auto new_val = decoder_helper<Derived, GetFunction>( name_copy, json_values );
				get_function( derived_obj ) = unescape_string( new_val );
			};
		}

		/*
		template<typename Derived> template<typename T, typename U>
		impl::decode_function_t<Derived> JsonLink<Derived>::standard_decoder( boost::string_view name, boost::optional<T> & value ) {
			auto value_ptr = &value;
			auto name_copy = name.to_string( );
			return [value_ptr, name_copy]( json_obj json_values ) mutable {
				daw::exception::daw_throw_on_false( value_ptr );
				auto new_val = nullable_decoder_helper<U>( name_copy, json_values );
				*value_ptr = new_val;
			};
		}

		template<typename Derived> template<typename T, typename U>
		impl::decode_function_t<Derived> JsonLink<Derived>::standard_decoder( boost::string_view name, daw::optional<T> & value ) {
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

		template<typename Derived> template<typename T, typename U>
		impl::decode_function_t<Derived> JsonLink<Derived>::standard_decoder( boost::string_view name, daw::optional_poly<T> & value ) {
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
		*/
		template<typename Derived> template<typename GetFunction>
		impl::bind_functions_t<Derived> JsonLink<Derived>::standard_bind_functions( boost::string_view name, GetFunction get_function ) {
			impl::bind_functions_t<Derived> bind_functions;
			bind_functions.encode = make_standard_encoder<Derived>( name, get_function );
			bind_functions.decode = make_standard_decoder<Derived>( name, get_function );
			return bind_functions;
		}
		
		template<typename Derived>
		void JsonLink<Derived>::add_to_data_map( boost::string_view name, impl::data_description_t<Derived> desc ) {
			auto key = range::create_char_range( name );
			daw::exception::daw_throw_on_false( m_data.m_data_map.count( key ) == 0 ); 
			auto result = m_data.m_data_map.emplace( std::move( key ), std::move( desc ) );
			daw::exception::daw_throw_on_false( result.second );
		}

		template<typename Derived>
		Derived & JsonLink<Derived>::as_derived( ) {
			return *static_cast<Derived*>( this ); 
		}

		template<typename Derived>
		Derived const & JsonLink<Derived>::as_derived( ) const {
			return *static_cast<Derived const *>( this ); 
		}

// DAW
		template<typename T, typename = std::enable_if_t<can_use_for_jsonlink_v<double, T>>>
		static void json_link_real( boost::string_view name, get_function_t<Derived, T> get_function );

		static void json_link_real( boost::string_view name, get_function2_t<Derived, double> get_function, set_function_t<Derived, double> set_function );

		template<typename T, typename = std::enable_if_t<can_use_for_jsonlink_v<std::string, T>>>
		static void json_link_string( boost::string_view name, get_function_t<Derived, T> get_function );

		static void json_link_string( boost::string_view name, get_function2_t<Derived, std::string> get_function, set_function_t<Derived, std::string> set_function );

		template<typename T, typename = std::enable_if_t<can_use_for_jsonlink_v<std::bool, T>>>
		static void json_link_boolean( boost::bool_view name, get_function_t<Derived, T> get_function );

		static void json_link_boolean( boost::bool_view name, get_function2_t<Derived, std::bool> get_function, set_function_t<Derived, std::bool> set_function );

		template<typename T>
		static void json_link_object( boost::bool_view name, get_function_t<Derived, JsonLink<T>> get_function );

		template<typename T>
		static void json_link_array( boost::bool_view name, get_function_t<Derived, T> get_function );

		template<typename T>
		static void json_link_map( boost::bool_view name, get_function_t<Derived, T> get_function );

		template<typename T>
		static void json_link_streamable( boost::bool_view name, get_function_t<Derived, T> get_function );

// DAW
		template<typename Derived> template<typename T>
		void JsonLink<Derived>::json_link_integer( boost::string_view name, get_function_t<Derived, T> get_function ) {
			impl::data_description_t<Derived> data_description;
			using namespace daw::json::impl::schema;
			data_description.json_type = get_schema<Derived>( name, get_function );
		}

		tempalte<typename Derived> template<typename T>
		void json_link_integer( boost::string_view name, Derived T::* data_ptr ) {
			json_link_integer( name, [data_ptr]( Derived & obj ) { return obj.*data_ptr; } ); 
		}

		template<typename Derived>
		void JsonLink<Derived>::json_link_integer( boost::string_view name, get_function2_t<Derived, int64_t> get_function, set_function_t<Derived, int64_t> set_function ) {

		}

		
		///
		/// \param name - name of integer value to link
		/// \param get_function - a function returning a T, and taking a const ref to Derived
		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_integer( boost::string_view name, GetFunction get_function ) {
			set_name( get_function, name.to_string( ) );
			impl::data_description_t<Derived> data_description;
			using namespace daw::json::impl::schema;
			data_description.json_type = get_schema<Derived>( name, get_function );

			data_description.bind_functions.encode = make_standard_encoder<Derived>( name, get_function );

			data_description.bind_functions.decode = [get_function, name]( Derived & derived_obj, json_obj const & json_values ) mutable {
				if( json_values.is_integer( ) ) {
					auto result = decoder_helper<int64_t>( name, json_values );
					using T = std::decay_t<decltype( get_function( derived_obj ) )>;
					daw::exception::daw_throw_on_false( result <= std::numeric_limits<T>::max( ) );
					daw::exception::daw_throw_on_false( result >= std::numeric_limits<T>::min( ) );
					get_function( derived_obj ) = static_cast<T>( result );
				}
				daw::exception::daw_throw_on_false( is_optional || json_values.is_integer( ) );
			};
			add_to_data_map( name, std::move( data_description ) );
		}

		///
		/// \param name - name of real(float/double...) value to link
		/// \param get_function - a function returning a T, and taking a const ref to Derived
		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_real( boost::string_view name, GetFunction get_function ) {
			set_name( get_function, name.to_string( ) );
			impl::data_description_t<Derived> data_description;
			using daw::json::impl::schema::get_schema;
			data_description.json_type = get_schema<Derived>( name, get_function );
			data_description.bind_functions.encode = make_standard_encoder<Derived>( name, get_function );

			data_description.bind_functions.decode = [get_function, name]( Derived & derived_obj, json_obj const & json_values ) mutable {
				if( json_values.is_numeric( ) ) {
					get_function( derived_obj ) = decoder_helper<double>( name, json_values );
				}
				daw::exception::daw_throw_on_false( is_optional || json_values.is_numeric( ) );
			};
			add_to_data_map( name, std::move( data_description ) );
		}

		///
		/// \param name - name of string value to link
		/// \param get_function - a function returning a T, and taking a const ref to Derived
		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_string( boost::string_view name, GetFunction get_function ) {
			json_link_value<is_optional>( name, get_function );
		}

		///
		/// \param name - name of boolean(true/false) value to link
		/// \param get_function - a function returning a T, and taking a const ref to Derived
		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_boolean( boost::string_view name, GetFunction get_function ) {
			json_link_value<is_optional>( name, get_function );
		}

		///
		/// \param name - name of JsonLink<type> obect value to link
		/// \param get_function - a function returning a T, and taking a const ref to Derived
		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_object( boost::string_view name, GetFunction get_function ) {
			set_name( get_function, name.to_string( ) );
			impl::data_description_t<Derived> data_description;
			using obj_t = decltype( get_function( std::declval<Derived>( ) ) ); 
			data_description.json_type = obj_t::get_schema_obj( );
			data_description.bind_functions.encode = make_standard_encoder<Derived>( name, get_function );
			data_description.bind_functions.decode = [get_function, name]( Derived & derived_obj, json_obj const & json_values ) mutable {
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
				daw::exception::daw_throw_on_false( is_optional || member->second.is_object( ) );
				if( member->second.is_object( ) ) {
					get_function( derived_obj ).from_json_obj( member->second );
				}
				daw::exception::daw_throw_on_false( is_optional || json_values.is_object( ) );
			};
			add_to_data_map( name, std::move( data_description ) );
		}
	
		/// \param name - name of array(vector) value to link
		/// \param get_function - a function returning a T, and taking a const ref to Derived
		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_array( boost::string_view name, GetFunction get_function ) {
			set_name( get_function, name.to_string( ) );
			impl::data_description_t<Derived> data_description;
			using ::daw::json::impl::schema::get_schema;
			data_description.json_type = get_schema<Derived>( name, get_function );
			data_description.bind_functions.encode = make_standard_encoder<Derived>( name, get_function );
			data_description.bind_functions.decode = [get_function, name]( Derived & derived_obj, json_obj const & json_values ) mutable {
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
				daw::exception::daw_throw_on_false( is_optional || member->second.is_array( ) );
				using namespace parse;
				if( member->second.is_array( ) ) {
					json_to_value( get_function( derived_obj ), member->second );
				}

			};
			add_to_data_map( name, std::move( data_description ) );
			
		}

		///
		/// \param name - name of map(unorderd_map/map) value to link.
		/// \param get_function - a function returning a T, and taking a const ref to Derived
		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_map( boost::string_view name, GetFunction get_function ) {
			set_name( get_function, name.to_string( ) );
			impl::data_description_t<Derived> data_description;
			using ::daw::json::impl::schema::get_schema;
			data_description.json_type = get_schema<Derived>( name, get_function );
			data_description.bind_functions.encode = make_standard_encoder<Derived>( name, get_function );
			data_description.bind_functions.decode = [get_function, name]( Derived & derived_obj, json_obj const & json_values ) mutable {
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
				daw::exception::daw_throw_on_false( is_optional || member->second.is_array( ) );
				using namespace parse;
				if( member->second.is_array( ) ) {
					json_to_value( get_function( derived_obj ), member->second );
				}
			};
			add_to_data_map( name, std::move( data_description ) );
			
		}

		///
		/// \param name - name of streamable value(operator<<, operator>>) to link.
		/// \param get_function - a function returning a T, and taking a const ref to Derived
		template<typename Derived> template<typename GetFunction, bool is_optional>
		void JsonLink<Derived>::json_link_streamable( boost::string_view name, GetFunction get_function ) {
			set_name( get_function, name );
			impl::data_description_t<Derived> data_description;
			using daw::json::impl::schema::get_schema;
			data_description.json_type = get_schema<Derived>( name, get_function );
			data_description.bind_functions.encode = [get_function, name]( Derived & derived_obj, std::string & json_text ) {
				json_text = generate::value_to_json( name.to_string( ), boost::lexical_cast<std::string>( get_function( derived_obj ) ) );
			};
			data_description.bind_functions.decode = [get_function, name]( Derived & derived_obj, json_obj const & json_values ) mutable {
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
				daw::exception::daw_throw_on_false( is_optional || member->second.is_string( ) );
				if( member->second.is_string( ) ) {
					std::stringstream ss( member->second.get_string( ) );
					ss >> get_function( derived_obj );
				}
			};
			add_to_data_map( name, std::move( data_description ) );
		}
			
		//
		// END OF JSONLINK
		//

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
		::daw::json::impl::value_t get_schema( boost::string_view name, JsonLink<Derived> const & obj ) {
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
		auto array_from_json_string( boost::string_view data, bool use_default_on_error ) {
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
			return array_from_json_string<Derived>( std::string{ std::istreambuf_iterator<char>{ in_file }, std::istreambuf_iterator<char>{ } }, use_default_on_error );
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
		std::ostream & operator<<( std::ostream & os, JsonLink<Derived> const & data ) {
			os << data.to_string( );
			return os;
		}

		template<typename Derived>
		std::istream & operator>>( std::istream & is, JsonLink<Derived> & data ) {
			std::string str{ std::istreambuf_iterator<char>{ is }, std::istreambuf_iterator<char>{ } };
			data.from_json_string( str );
			return is;
		}
	}    // namespace json
}    // namespace daw
