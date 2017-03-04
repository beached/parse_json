// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
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
#include <boost/variant.hpp>

namespace daw {
	namespace json {
		namespace impl {
			struct linked_object
			struct encoder_decoder_t {
				using value_t = boost::variant<


			};
			template<typename LinkedType, typename Char=char>
			struct link_state_t {
				using string_t = std::basic_string<Char>;
				string_t current_element;
				string_t buffer;
				std::function<void(link_state_t &, boost::string_view)> current_state;

				static void state_expect_string( link_state_t & self, boost::string_view str ) {
					// In array, current_element will have a value
				}
				static state_fn_t const state_expect_real;
				static state_fn_t const state_expect_integral;
				static state_fn_t const state_expect_boolean;
				static state_fn_t const state_expect_array;
				static state_fn_t const state_expect_object;
				static state_fn_t const state_at_root {

				};

				link_state_t( ):
						current_element{ },
						buffer{ },
						current_state{ &state_at_root } { }

				void clear_buffer( ) {
					buffer.clear( );
				}

				void push( Char c ) {
					buffer.push_back( c );
				}

				void on_string( boost::string_view str );
				void on_real( boost::string_view str );
				void on_integral( boost::string_view str );
				void on_boolean( bool b );
				void on_null( );
				void on_array_begin( );
				void on_array_end( );
				void on_object_begin( );
				void on_object_end( );
			};	// link_state_t
		}	// namespace impl

		template<typename Derived>
		class json_link_v2 {
		public:
			json_link_v2( ) = default;
			~json_link_v2( ) { }
			json_link_v2( json_link_v2 const & ) = default;
			json_link_v2( json_link_v2 && ) = default;
			json_link_v2 & operator=( json_link_v2 const & ) = default;
			json_link_v2 & operator=( json_link_v2 && ) = default;

			template<typename InputIterator>
			auto from_iterator( InputIterator first, auto last ) {

			}

			auto from_string( boost::string_view data ) {
				return from_iterator( data.begin( ), data.end( ) );
			}
		};	// json_link_v2
	}	// namespace json

}	// namespace daw
