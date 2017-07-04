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

#include <string>

#include "daw_json_parser_exception.h"
#include "daw_json_parser_impl.h"

namespace daw {
	namespace json {
		template<typename InputIteratorFirst, typename InputIteratorLast, typename State>
		void json_parser( InputIteratorFirst first, InputIteratorLast last, State &state ) {
			first = impl::skip_whitespace( first, last );
			while( first != last ) {
				switch( *first ) {
				case '{':
					first = impl::parse_object( first, last, state );
					break;
				case '[':
					first = impl::parse_array( first, last, state );
					break;
				default:
					throw json_parser_exception( "Expected an array '[' or object '{' at start of json file" );
				}
				first = impl::skip_whitespace( first, last );
			}
		}
	} // namespace json
} // namespace daw
