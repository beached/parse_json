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

#include <stdexcept>

namespace daw {
	namespace json {
		struct json_parser_exception final : public std::runtime_error {
			json_parser_exception( std::string message ) : std::runtime_error{std::move( message )} {}

			~json_parser_exception( );

			json_parser_exception( json_parser_exception const & ) = default;

			json_parser_exception( json_parser_exception && ) noexcept = default;
			json_parser_exception &operator=( json_parser_exception && ) noexcept = default;

			json_parser_exception &operator=( json_parser_exception const &rhs ) {
				return *this = json_parser_exception{rhs};
			}
		}; // struct json_parser_exception
	}      // namespace json
} // namespace daw
