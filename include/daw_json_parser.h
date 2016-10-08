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

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <daw/daw_common_mixins.h>
#include <daw/char_range/daw_char_range.h>
#include <daw/daw_variant.h>

#include "value_t.h"

namespace daw {
	namespace json {
		struct JsonParserException final {
			std::string message;

			JsonParserException( ) = default;

			JsonParserException( std::string msg );

			~JsonParserException( ) = default;

			JsonParserException( JsonParserException const & ) = default;

			JsonParserException( JsonParserException && ) = default;

			JsonParserException & operator=( JsonParserException const & ) = default;

			JsonParserException & operator=( JsonParserException && ) = default;
		};    // struct JsonParserException

		using json_obj = impl::value_t;

		json_obj parse_json( char const *Begin, char const *End );

		json_obj parse_json( boost::string_view const json_text );

		template<typename T>
			T get( impl::value_t const & );/*
											   static_assert(false, "Unsupported get type called");
											   }*/

		template<> int64_t get<int64_t>( impl::value_t const & val );

		template<> double get<double>( impl::value_t const & val );

		template<> std::string get<std::string>( impl::value_t const & val );

		template<> bool get<bool>( impl::value_t const & val );

		template<> impl::object_value get<impl::object_value>( impl::value_t const & val );

		template<> impl::array_value get<impl::array_value>( impl::value_t const & val );
	}    // namespace json
}    // namespace daw
