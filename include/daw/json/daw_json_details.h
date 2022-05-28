// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#pragma once

#include "daw_json_interface.h"
#include "daw_json_parser.h"

#include <daw/cpp_17.h>
#include <daw/daw_exception.h>
#include <daw/daw_move.h>
#include <daw/daw_range.h>
#include <daw/daw_string_view.h>
#include <daw/daw_traits.h>
#include <daw/daw_traits_is_a_type.h>

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace daw::json::parse {
	template<typename Container,
	         std::enable_if_t<daw::traits::is_vector_like_not_string_v<Container>,
	                          std::nullptr_t> = nullptr>
	void json_to_value( Container &to, daw::json::json_value_t const &from ) {
		static_assert( !std::is_const_v<Container>,
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
		static_assert( !std::is_const_v<decltype( to )>,
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
		to = std::make_pair<Key, Value>( daw::move( key ), daw::move( value ) );
	}

	template<typename MapContainer,
	         std::enable_if_t<daw::traits::is_map_like_v<MapContainer>,
	                          std::nullptr_t> = nullptr>
	void json_to_value( MapContainer &to, daw::json::json_value_t const &from ) {
		static_assert( !std::is_const_v<MapContainer>,
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
	                       (std::is_integral_v<T> and
	                        not std::is_same_v<T, json_value_t::integer_t>),
	                       long>>
	void json_to_value( T &to, daw::json::json_value_t const &from ) {
		static_assert( !std::is_const_v<decltype( to )>,
		               "To parameter on json_to_value cannot be const" );
		daw::exception::daw_throw_on_false( from.is_integer( ) );
		auto result = from.get_integer( );
		daw::exception::daw_throw_on_false(
		  static_cast<json_value_t::integer_t>( std::numeric_limits<T>::max( ) ) >=
		  result );
		daw::exception::daw_throw_on_false(
		  static_cast<json_value_t::integer_t>( std::numeric_limits<T>::min( ) ) <=
		  result );
		to = static_cast<T>( result );
	}

	template<typename T>
	void json_to_value( std::optional<T> &to,
	                    daw::json::json_value_t const &from ) {
		static_assert( !std::is_const_v<decltype( to )>,
		               "To parameter on json_to_value cannot be const" );
		if( from.is_null( ) ) {
			to.reset( );
		} else {
			T result;
			json_to_value( result, from );
			*to = daw::move( result );
		}
	}

	template<typename T>
	void json_to_value( std::shared_ptr<T> &to,
	                    daw::json::json_value_t const &from ) {
		static_assert( not std::is_const_v<decltype( to )>,
		               "To parameter on json_to_value cannot be const" );
		daw::exception::daw_throw_on_false( to );
		if( from.is_null( ) ) {
			to.reset( );
		} else {
			T result;
			json_to_value( result, from );
			*to = daw::move( result );
		}
	}

	template<typename T, std::enable_if_t<has_decode_member_v<T>, std::nullptr_t>>
	T decode_to_new( daw::string_view json_values ) {
		T result;
		result.decode( json_values );
		return result;
	}
} // namespace daw::json::parse
