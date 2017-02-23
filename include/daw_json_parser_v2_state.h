
// The MIT License (MIT)
//
// Copyright (c) 2016 Darrell Wright
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

#include <boost/utility/string_view.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "daw_json_parser_v2.h"

namespace daw {
	namespace json {
		namespace state {
			enum class current_state_t: uint8_t { none = 0, in_object_name, in_object_value, in_array, current_state_t_size };
			struct expected_state_t {
				uint8_t value;
				const uint8_t es_optional = 1;
				enum flags: uint8_t { es_any = 255, es_null = 0, es_number = 2, es_string = 4, es_boolean = 8, es_object = 16, es_array = 32 };
				constexpr expected_state_t( ) noexcept:
					value{ 0 } { }

				constexpr expected_state_t( flags flag, bool is_optional = false ):
						value{ is_optional ? es_optional : es_any } {

					switch( flag ) {
					case es_any:
					case es_number:
					case es_string:
					case es_boolean:
					case es_object:
					case es_array:
						value |= flag;
					default:
						throw std::invalid_argument{ "flag" };
					}
				}

				constexpr operator flags( ) const noexcept {
					return value;
				}

				~expected_state_t( ) = default;
				expected_state_t( expected_state_t const & ) = default;
				expected_state_t( expected_state_t && ) = default;
				expected_state_t & operator=( expected_state_t const & ) = default;
				expected_state_t & operator=( expected_state_t && ) = default;

				constexpr bool is_valid( flags flag ) const noexcept {
					switch( value ) {
					case es_any:
					case es_number:
					case es_string:
					case es_boolean:
					case es_object:
					case es_array:
						return (value & ~es_optional) & flag != 0;	
					default:
						return (value | es_optional) == es_optional;
					}
				}
			};	// expected_state_t

			struct state_t {
				expected_state_t expected_state;

				state_t( );
				virtual ~state_t( );

				state_t( state_t const & ) = default;
				state_t( state_t && ) = default;
				state_t & operator=( state_t const & ) = default;
				state_t & operator=( state_t && ) = default;


				virtual void on_object_begin( );
				virtual void on_object_end( );
				virtual void on_array_begin( );
				virtual void on_array_end( );
				virtual void on_string( boost::string_view );
				virtual void on_integral( boost::string_view );
				virtual void on_real( boost::string_view );
				virtual void on_boolean( bool ); 
				virtual void on_null( );
				virtual std::string to_string( ) const = 0;
			};	// state_t

			struct state_in_object_name_t: public state_t {
				state_in_object_name_t( ) = default;
				state_in_object_name_t( state_in_object_name_t const & ) = default;
				state_in_object_name_t( state_in_object_name_t && ) = default;
				state_in_object_name_t & operator=( state_in_object_name_t const & ) = default;
				state_in_object_name_t & operator=( state_in_object_name_t && ) = default;

				~state_in_object_name_t( );
				std::string to_string( ) const override;
				void on_string( boost::string_view value ) override;
				void on_object_end( ) override;
			};	// state_in_object_name

			struct state_in_object_value_t: public state_t {
				state_in_object_value_t( ) = default;
				state_in_object_value_t( state_in_object_value_t const & ) = default;
				state_in_object_value_t( state_in_object_value_t && ) = default;
				state_in_object_value_t & operator=( state_in_object_value_t const & ) = default;
				state_in_object_value_t & operator=( state_in_object_value_t && ) = default;

				~state_in_object_value_t( );

				std::string to_string( ) const override;
				void on_object_begin( ) override;
				void on_array_begin( ) override;
				void on_null( ) override;

				void on_integral( boost::string_view value ) override;
				void on_real( boost::string_view value ) override;
				void on_string( boost::string_view value ) override;
				void on_boolean( bool value ) override;
			};	// state_in_object_value_t

			struct state_in_array_t: public state_t {
				state_in_array_t( ) = default;
				state_in_array_t( state_in_array_t const & ) = default;
				state_in_array_t( state_in_array_t && ) = default;
				state_in_array_t & operator=( state_in_array_t const & ) = default;
				state_in_array_t & operator=( state_in_array_t && ) = default;

				~state_in_array_t( );

				std::string to_string( ) const override;
				void on_object_begin( ) override;
				void on_array_begin( ) override;
				void on_array_end( ) override;
				void on_null( ) override;
				void on_integral( boost::string_view value ) override;
				void on_real( boost::string_view value ) override;
				void on_string( boost::string_view value ) override;
				void on_boolean( bool value ) override;
			};	// state_in_array_t

			struct state_none_t: public state_t {
				state_none_t( ) = default;
				state_none_t( state_none_t const & ) = default;
				state_none_t( state_none_t && ) = default;
				state_none_t & operator=( state_none_t const & ) = default;
				state_none_t & operator=( state_none_t && ) = default;

				~state_none_t( );

				std::string to_string( ) const override;
				void on_object_begin( ) override;
				void on_array_begin( ) override;
				void on_null( ) override;
				void on_integral( boost::string_view value ) override;
				void on_real( boost::string_view value ) override;
				void on_string( boost::string_view value ) override;
				void on_boolean( bool value ) override;
			};	// state_none_t

			state_t * get_state_fn( current_state_t s ) noexcept;
			std::vector<state_t*> & state_stack( );
			state_t & current_state( );
			void push_and_set_next_state( current_state_t s );
			void set_next_state( current_state_t s );
			void pop_state( );
			state_t * get_state_fn( current_state_t s ) noexcept;

			struct state_control_t {
				std::string buffer;
				state_t const & current_state( ) const;
				void push( char c );
				void clear_buffer( );
				void on_object_begin( ) const;
				void on_object_end( ) const;
				void on_array_begin( ) const;
				void on_array_end( ) const;				
				void on_string( boost::string_view value ) const; 
				void on_integral( boost::string_view value ) const;
				void on_real( boost::string_view value ) const; 
				void on_boolean( bool value ) const; 
				void on_null( ) const; 
			};
		}	// namspace state
	}	// namespace json
}    // namespace daw

