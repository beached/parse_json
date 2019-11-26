// The MIT License (MIT)
//
// Copyright (c) 2014-2019 Darrell Wright
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

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include <daw/daw_string_view.h>
#include <daw/daw_traits.h>

#include "daw_json_parser.h"

namespace daw {
	void localtime_s( std::time_t const *source, struct tm *result );

	namespace json {
		namespace details {
			std::string json_name( daw::string_view name );
			std::string enbrace( daw::string_view json_value );
		} // namespace details

		std::string enquote( daw::string_view value );

		namespace parse {
			void json_to_value( bool &to, daw::json::json_value_t const &from );

			void json_to_value( double &to, daw::json::json_value_t const &from );

			void json_to_value( float &to, daw::json::json_value_t const &from );

			void json_to_value( int64_t &to, daw::json::json_value_t const &from );

			void json_to_value( std::string &to,
			                    daw::json::json_value_t const &from );

			// Template json_to_value declarations
			template<typename Container,
			         typename std::enable_if_t<
			           daw::traits::is_vector_like_not_string_v<Container>, long> = 0>
			void json_to_value( Container &to, daw::json::json_value_t const &from );

			template<typename Key, typename Value>
			void json_to_value( std::pair<Key, Value> &to,
			                    daw::json::json_value_t const &from );

			template<typename MapContainer,
			         typename std::enable_if_t<
			           daw::traits::is_map_like_v<MapContainer>, long> = 0>
			void json_to_value( MapContainer &to,
			                    daw::json::json_value_t const &from );

			template<
			  typename T,
			  typename std::enable_if_t<
			    std::is_integral_v<T> && !std::is_same_v<T, int64_t>, long> = 0>
			void json_to_value( T &to, daw::json::json_value_t const &from );

			template<typename T>
			void json_to_value( std::optional<T> &to,
			                    daw::json::json_value_t const &from );

			template<typename T>
			void json_to_value( std::shared_ptr<T> &to,
			                    daw::json::json_value_t const &from );

			namespace impl {
				template<typename T>
				using has_decode_member =
				  decltype( std::declval<T>( ).decode( std::declval<std::string>( ) ) );
			}
			template<typename T>
			constexpr bool has_decode_member_v =
			  daw::is_detected_v<impl::has_decode_member, T>;

			template<typename T, std::enable_if_t<has_decode_member_v<T>,
			                                      std::nullptr_t> = nullptr>
			T decode_to_new( daw::string_view json_values );
		} // namespace parse
	}   // namespace json
} // namespace daw

template<typename T, typename EnableIf =
                       decltype( std::declval<T>( ).serialize_to_json( ) )>
std::ostream &operator<<( std::ostream &os, T const &data ) {
	os << data.serialize_to_json( );
	return os;
}
