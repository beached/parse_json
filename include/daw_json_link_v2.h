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

namespace daw {
	namespace json {
		template<typename Derived>
		class json_link_v2 {
			Derived & get_derived( ) noexcept {
				return *static_cast<Derived*>(this);
			}
			Derived const & get_derived( ) const noexcept {
				return *static_cast<Derived*>(this);
			}
		public:
			json_link_v2( ) = default;
			~json_link_v2( ) { }
			json_link_v2( json_link_v2 const & ) = default;
			json_link_v2( json_link_v2 && ) = default;
			json_link_v2 & operator=( json_link_v2 const & ) = default;
			json_link_v2 & operator=( json_link_v2 && ) = default;

			auto from_string( boost::string_view data ) {
				auto const callbacks = Derived::json_link_callbacks( );

			}
		};	// json_link_v2
	}	// namespace json

}	// namespace daw
