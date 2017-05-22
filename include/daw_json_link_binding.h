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
#include <cstdlib>
#include <functional>
#include <string>

#include <daw/daw_exception.h>
#include <daw/not_null.h>

namespace daw {
	namespace json {
		namespace types {
			enum class expected_json_types : uint8_t {
				integer,
				real,
				string,
				boolean,
				integral_array,
				real_array,
				string_array,
				boolean_array,
				object,
				object_array
			};
		}

		template<typename Derived>
		struct json_binding_t {
			using to_json_function_t = std::function<std::string( Derived const & )>;
			using from_json_function_t = std::function<void( Derived &, boost::string_view )>;

			types::expected_json_types expected_json_type;
			bool is_optional;
			from_json_function_t from_json_function;
			to_json_function_t to_json_function;

			json_binding_t( types::expected_json_types ExpectedJsonType, bool IsOptional, from_json_function_t FromJson,
			                to_json_function_t ToJson )
			    : expected_json_type{ExpectedJsonType}
			    , is_optional{IsOptional}
			    , from_json_function{FromJson}
			    , to_json_function{ToJson} {}

			json_binding_t( ) = delete;
			~json_binding_t( ) = default;
			json_binding_t( json_binding_t const & ) = default;
			json_binding_t( json_binding_t && ) noexcept = default;
			json_binding_t &operator=( json_binding_t && ) noexcept = default;

			json_binding_t &operator=( json_binding_t const &rhs ) {
				return *this = json_binding_t{rhs};
			}

		}; // json_binding_t
	}      // namespace json
} // namespace daw
