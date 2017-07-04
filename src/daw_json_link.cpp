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

#include "daw_json_link.h"

namespace daw {
	namespace json {
		namespace impl {
			std::string to_json_integer( json_value_t::integer_t i ) {
				using std::to_string;
				return to_string( i );
			}

			std::string to_json_real( json_value_t::real_t d ) {
				using std::to_string;
				return to_string( d );
			}

			std::string to_json_string( boost::string_view s ) {
				std::string result;
				result = "\"" + s.to_string( ) + "\"";
				return result;
			}

			std::string to_json_boolean( bool b ) {
				return b ? "true" : "false";
			}

			std::string to_json_null( ) {
				using namespace std::string_literals;
				return "null"s;
			}
		} // namespace impl
	}     // namespace json
} // namespace daw

