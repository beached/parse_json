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

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <daw/daw_exception.h>
#include <daw/daw_optional.h>
#include <daw/daw_optional_poly.h>
#include <daw/daw_range.h>
#include <daw/daw_traits.h>

#include "daw_json_interface.h"
#include "daw_json_parser.h"

namespace daw {
	namespace json {
		namespace parse {
			template<typename Container,
			         typename std::enable_if_t<daw::traits::is_vector_like_not_string<Container>::value, long>>
			void json_to_value( Container &to, ::daw::json::impl::value_t const &from ) {
				static_assert( !std::is_const<Container>::value, "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( from.is_array( ) );
				auto const &source_array = from.get_array( );
				to.clear( );
				std::transform( std::begin( source_array ), std::end( source_array ), std::back_inserter( to ),
				                [](::daw::json::impl::value_t const &value ) {
					                typename Container::value_type tmp;
					                json_to_value( tmp, value );
					                return tmp;
				                } );
			}

			template<typename Key, typename Value>
			void json_to_value( std::pair<Key, Value> &to, ::daw::json::impl::value_t const &from ) {
				static_assert( !std::is_const<decltype( to )>::value, "To parameter on json_to_value cannot be const" );
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
			         typename std::enable_if_t<daw::traits::is_map_like<MapContainer>::value, long>>
			void json_to_value( MapContainer &to, ::daw::json::impl::value_t const &from ) {
				static_assert( !std::is_const<MapContainer>::value, "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( from.is_array( ) );
				auto const &source_array = from.get_array( );
				to.clear( );
				std::transform( std::begin( source_array ), std::end( source_array ),
				                std::inserter( to, std::end( to ) ), [](::daw::json::impl::value_t const &value ) {
					                using key_t = typename std::decay<typename MapContainer::key_type>::type;
					                using value_t = typename std::decay<typename MapContainer::mapped_type>::type;
					                std::pair<key_t, value_t> tmp;
					                json_to_value( tmp, value );
					                return tmp;
				                } );
			}

			template<typename T,
			         typename std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, int64_t>::value, long>>
			void json_to_value( T &to, ::daw::json::impl::value_t const &from ) {
				static_assert( !std::is_const<decltype( to )>::value, "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( from.is_integer( ) );
				auto result = get<int64_t>( from );
				daw::exception::daw_throw_on_false( static_cast<int64_t>( std::numeric_limits<T>::max( ) ) >= result );
				daw::exception::daw_throw_on_false( static_cast<int64_t>( std::numeric_limits<T>::min( ) ) <= result );
				to = static_cast<T>( result );
			}

			template<typename T>
			void json_to_value( boost::optional<T> &to, ::daw::json::impl::value_t const &from ) {
				static_assert( !std::is_const<decltype( to )>::value, "To parameter on json_to_value cannot be const" );
				if( from.is_null( ) ) {
					to.reset( );
				} else {
					T result;
					json_to_value( result, from );
					*to = std::move( result );
				}
			}

			template<typename T>
			void json_to_value( std::shared_ptr<T> &to, ::daw::json::impl::value_t const &from ) {
				static_assert( !std::is_const<decltype( to )>::value, "To parameter on json_to_value cannot be const" );
				daw::exception::daw_throw_on_false( to );
				if( from.is_null( ) ) {
					to.reset( );
				} else {
					T result;
					json_to_value( result, from );
					*to = std::move( result );
				}
			}

			template<typename T, typename std::enable_if_t<has_decode_member<T>::value, long>>
			T decode_to_new( boost::string_view json_values ) {
				T result;
				result.decode( json_values );
				return result;
			}
		} // namespace parse
	}     // namespace json
} // namespace daw
