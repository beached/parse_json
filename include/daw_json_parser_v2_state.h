// The MIT License (MIT)
//
// Copyright (c) 2016-2019 Darrell Wright
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

#pragma once

#include <string>
#include <vector>

#include <daw/daw_string_view.h>
#include <daw/daw_virtual_base.h>

#include "daw_json_parser_v2.h"

namespace daw {
	namespace json {
		namespace state {
			enum class current_state_t {
				none = 0,
				in_object_name,
				in_object_value,
				in_array,
				current_state_t_size
			};

			struct state_t : daw::virtual_base<state_t> {
				state_t( ) = default;

				virtual void on_object_begin( );
				virtual void on_object_end( );
				virtual void on_array_begin( );
				virtual void on_array_end( );
				virtual void on_string( daw::string_view );
				virtual void on_integer( daw::string_view );
				virtual void on_real( daw::string_view );
				virtual void on_boolean( bool );
				virtual void on_null( );
				virtual std::string to_string( ) const = 0;
			}; // state_t

			struct state_in_object_name_t : public state_t {
				state_in_object_name_t( ) = default;
				state_in_object_name_t( state_in_object_name_t const & ) = default;
				state_in_object_name_t( state_in_object_name_t && ) noexcept = default;
				state_in_object_name_t &
				operator=( state_in_object_name_t const & ) = default;
				state_in_object_name_t &
				operator=( state_in_object_name_t && ) noexcept = default;

				~state_in_object_name_t( ) override;
				std::string to_string( ) const override;
				void on_string( daw::string_view value ) override;
				void on_object_end( ) override;
			}; // state_in_object_name

			struct state_in_object_value_t : public state_t {
				state_in_object_value_t( ) = default;
				state_in_object_value_t( state_in_object_value_t const & ) = default;
				state_in_object_value_t( state_in_object_value_t && ) noexcept =
				  default;
				state_in_object_value_t &
				operator=( state_in_object_value_t const & ) = default;
				state_in_object_value_t &
				operator=( state_in_object_value_t && ) noexcept = default;

				~state_in_object_value_t( ) override;

				std::string to_string( ) const override;
				void on_object_begin( ) override;
				void on_array_begin( ) override;
				void on_null( ) override;

				void on_integer( daw::string_view value ) override;
				void on_real( daw::string_view value ) override;
				void on_string( daw::string_view value ) override;
				void on_boolean( bool value ) override;
			}; // state_in_object_value_t

			struct state_in_array_t : public state_t {
				state_in_array_t( ) = default;
				state_in_array_t( state_in_array_t const & ) = default;
				state_in_array_t( state_in_array_t && ) noexcept = default;
				state_in_array_t &operator=( state_in_array_t const & ) = default;
				state_in_array_t &operator=( state_in_array_t && ) noexcept = default;

				~state_in_array_t( ) override;

				std::string to_string( ) const override;
				void on_object_begin( ) override;
				void on_array_begin( ) override;
				void on_array_end( ) override;
				void on_null( ) override;
				void on_integer( daw::string_view value ) override;
				void on_real( daw::string_view value ) override;
				void on_string( daw::string_view value ) override;
				void on_boolean( bool value ) override;
			}; // state_in_array_t

			struct state_none_t : public state_t {
				state_none_t( ) = default;
				state_none_t( state_none_t const & ) = default;
				state_none_t( state_none_t && ) noexcept = default;
				state_none_t &operator=( state_none_t const & ) = default;
				state_none_t &operator=( state_none_t && ) noexcept = default;

				~state_none_t( ) override;

				std::string to_string( ) const override;
				void on_object_begin( ) override;
				void on_array_begin( ) override;
				void on_null( ) override;
				void on_integer( daw::string_view value ) override;
				void on_real( daw::string_view value ) override;
				void on_string( daw::string_view value ) override;
				void on_boolean( bool value ) override;
			}; // state_none_t

			state_t *get_state_fn( current_state_t s ) noexcept;
			std::vector<state_t *> &state_stack( );
			state_t &current_state( );
			void push_and_set_next_state( current_state_t s );
			void set_next_state( current_state_t s );
			void pop_state( );
			state_t *get_state_fn( current_state_t s ) noexcept;

			struct state_control_t {
				std::string buffer;
				state_t const &current_state( ) const;
				void push( char c );
				void clear_buffer( );
				void on_object_begin( ) const;
				void on_object_end( ) const;
				void on_array_begin( ) const;
				void on_array_end( ) const;
				void on_string( daw::string_view value ) const;
				void on_integer( daw::string_view value ) const;
				void on_real( daw::string_view value ) const;
				void on_boolean( bool value ) const;
				void on_null( ) const;
			};
		} // namespace state
	}   // namespace json
} // namespace daw
