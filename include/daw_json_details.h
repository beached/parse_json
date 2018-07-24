// The MIT License (MIT)
//
// Copyright (c) 2014-2018 Darrell Wright
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

#include <algorithm>
#include <boost/optional.hpp>
#include <cstddef>
#include <iomanip>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <daw/cpp_17.h>
#include <daw/daw_exception.h>
#include <daw/daw_optional.h>
#include <daw/daw_optional_poly.h>
#include <daw/daw_range.h>
#include <daw/daw_string_view.h>
#include <daw/daw_traits.h>

#include "daw_json_interface.h"
#include "daw_json_parser.h"

namespace daw {
	namespace json {
		namespace parse {
			template<
			  typename Container,
			  std::enable_if_t<daw::traits::is_vector_like_not_string_v<Container>,
			                   std::nullptr_t> = nullptr>
			void json_to_value( Container &to, daw::json::json_value_t const &from ) {
				static_assert( !daw::is_const_v<Container>,
				               "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( from.is_array( ) );
				auto const &source_array = from.get_array( );
				to.clear( );
				std::transform( std::begin( source_array ), std::end( source_array ),
				                std::back_inserter( to ),
				                []( daw::json::json_value_t const &value ) {
					                typename Container::value_type tmp;
					                json_to_value( tmp, value );
					                return tmp;
				                } );
			}

			template<typename Key, typename Value>
			void json_to_value( std::pair<Key, Value> &to,
			                    daw::json::json_value_t const &from ) {
				static_assert( !daw::is_const_v<decltype( to )>,
				               "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( from.is_object( ) );

				auto const &obj = from.get_object( );
				daw::exception::daw_throw_on_false( obj.members_v.size( ) == 2 );

				Key key;
				auto const &key_obj = obj["key"];
				json_to_value( key, key_obj );
				Value value;
				auto const &value_obj = obj["value"];
				json_to_value( value, value_obj );
				to = std::make_pair<Key, Value>( std::move( key ), std::move( value ) );
			}

			template<typename MapContainer,
			         std::enable_if_t<daw::traits::is_map_like_v<MapContainer>,
			                          std::nullptr_t> = nullptr>
			void json_to_value( MapContainer &to,
			                    daw::json::json_value_t const &from ) {
				static_assert( !daw::is_const_v<MapContainer>,
				               "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( from.is_array( ) );
				auto const &source_array = from.get_array( );
				to.clear( );
				std::transform( std::begin( source_array ), std::end( source_array ),
				                std::inserter( to, std::end( to ) ),
				                []( daw::json::json_value_t const &value ) {
					                using key_t =
					                  std::decay_t<typename MapContainer::key_type>;
					                using value_t =
					                  std::decay_t<typename MapContainer::mapped_type>;
					                std::pair<key_t, value_t> tmp;
					                json_to_value( tmp, value );
					                return tmp;
				                } );
			}

			template<typename T, typename std::enable_if_t<
			                       (daw::is_integral_v<T> &&
			                        !daw::is_same_v<T, json_value_t::integer_t>),
			                       long>>
			void json_to_value( T &to, daw::json::json_value_t const &from ) {
				static_assert( !daw::is_const_v<decltype( to )>,
				               "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( from.is_integer( ) );
				auto result = from.get_integer( );
				daw::exception::daw_throw_on_false(
				  static_cast<json_value_t::integer_t>(
				    std::numeric_limits<T>::max( ) ) >= result );
				daw::exception::daw_throw_on_false(
				  static_cast<json_value_t::integer_t>(
				    std::numeric_limits<T>::min( ) ) <= result );
				to = static_cast<T>( result );
			}

			template<typename T>
			void json_to_value( boost::optional<T> &to,
			                    daw::json::json_value_t const &from ) {
				static_assert( !daw::is_const_v<decltype( to )>,
				               "To parameter on json_to_value cannot be const" );
				if( from.is_null( ) ) {
					to.reset( );
				} else {
					T result;
					json_to_value( result, from );
					*to = std::move( result );
				}
			}

			template<typename T>
			void json_to_value( std::shared_ptr<T> &to,
			                    daw::json::json_value_t const &from ) {
				static_assert( !daw::is_const_v<decltype( to )>,
				               "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( to );
				if( from.is_null( ) ) {
					to.reset( );
				} else {
					T result;
					json_to_value( result, from );
					*to = std::move( result );
				}
			}

			template<typename T,
			         std::enable_if_t<has_decode_member_v<T>, std::nullptr_t>>
			T decode_to_new( daw::string_view json_values ) {
				T result;
				result.decode( json_values );
				return result;
			}
		} // namespace parse
	}   // namespace json
} // namespace daw
