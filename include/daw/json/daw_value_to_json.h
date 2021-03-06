// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#pragma once

#include "daw_json_interface.h"

#include <daw/daw_optional_poly.h>
#include <daw/daw_range.h>
#include <daw/daw_string_view.h>

#include <iomanip>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace daw::json {
	template<typename Derived>
	class JsonLink;

	namespace generate {
		std::string undefined_value_to_json( daw::string_view name ) noexcept;
		std::string value_to_json( daw::string_view name );
		std::string value_to_json( json_string_value name );

		std::string value_to_json( daw::string_view name, bool value );
		std::string value_to_json( json_string_value name, bool value );

		std::string value_to_json( daw::string_view name,
		                           daw::json::json_value_t const &value );
		std::string value_to_json_value( json_string_value name,
		                                 daw::json::json_value_t const &value );

		std::string
		value_to_json_object( daw::string_view name,
		                      daw::json::json_object_value const &object );
		std::string value_to_json( json_string_value name,
		                           daw::json::json_object_value const &object );

		std::string value_to_json( daw::string_view name,
		                           std::string const &value );
		std::string value_to_json( json_string_value name,
		                           std::string const &value );

		template<typename Container,
		         typename daw::enable_when_t<
		           daw::traits::is_container_not_string_v<Container>> = nullptr>
		std::string value_to_json( daw::string_view name, Container const &values );

		template<typename Derived>
		std::string value_to_json( daw::string_view name,
		                           JsonLink<Derived> const &obj );

		template<typename First, typename Second>
		std::string value_to_json( daw::string_view name,
		                           std::pair<First, Second> const &value );

		template<typename Number, typename std::enable_if_t<
		                            std::is_floating_point_v<Number>, int> = 0>
		std::string value_to_json_number( daw::string_view name,
		                                  Number const &value );

		template<typename Number,
		         typename std::enable_if_t<std::is_integral_v<Number>, int> = 0>
		std::string value_to_json_number( daw::string_view name,
		                                  Number const &value );

		template<typename T>
		std::string value_to_json( daw::string_view name,
		                           std::optional<T> const &value );

		template<typename T>
		std::string value_to_json( daw::string_view name,
		                           daw::optional_poly<T> const &value );

		template<typename T>
		void value_to_json( daw::string_view name,
		                    std::shared_ptr<T> const &value );

		template<typename T>
		void value_to_json( daw::string_view name, std::weak_ptr<T> const &value );

		template<typename Container,
		         typename daw::enable_when_t<
		           daw::traits::is_container_not_string_v<Container>>>
		std::string value_to_json( daw::string_view name,
		                           Container const &values ) {

			daw::string_view const empty_str{ "" };
			std::stringstream result;
			result << daw::json::details::json_name( name ) << "[ ";
			{
				auto values_range =
				  daw::range::make_range( values.begin( ), values.end( ) );
				if( !values_range.empty( ) ) {
					result << value_to_json( empty_str, *values_range.begin( ) );
					values_range.move_next( );
					for( auto item : values_range ) {
						result << "," << value_to_json( empty_str, item );
					}
				}
			}
			result << " ]";
			return result.str( );
		}

		template<typename Derived>
		std::string value_to_json( daw::string_view name,
		                           JsonLink<Derived> const &obj ) {
			return details::json_name( name ) + obj.to_string( );
		}

		template<typename First, typename Second>
		std::string value_to_json( daw::string_view name,
		                           std::pair<First, Second> const &value ) {
			std::string result = daw::json::details::json_name( name ) + "{ ";
			result += value_to_json( "key", value.first ) + ", ";
			result += value_to_json( "value", value.second ) + " }";
			return result;
		}

		template<typename Number,
		         typename std::enable_if_t<std::is_floating_point_v<Number>, int>>
		std::string value_to_json_number( daw::string_view name,
		                                  Number const &value ) {
			std::stringstream ss;
			ss << daw::json::details::json_name( name );
			ss << std::setprecision( std::numeric_limits<Number>::max_digits10 )
			   << value;
			return ss.str( );
		}

		template<typename Number,
		         typename std::enable_if_t<std::is_integral_v<Number>, int>>
		std::string value_to_json_number( daw::string_view name,
		                                  Number const &value ) {
			return daw::json::details::json_name( name ) + std::to_string( value );
		}

		// Number
		template<typename Number,
		         daw::enable_when_t<std::is_arithmetic_v<Number>> = nullptr>
		std::string value_to_json( daw::string_view name, Number value ) {
			return value_to_json_number( name, value );
		}
		template<typename Number,
		         daw::enable_when_t<std::is_arithmetic_v<Number>> = nullptr>
		std::string value_to_json( json_string_value name, Number value ) {
			return value_to_json_number( to_string_view( name ), value );
		}

		template<typename T>
		std::string value_to_json( daw::string_view name,
		                           std::optional<T> const &value ) {
			if( value ) {
				return value_to_json( name, *value );
			}
			return value_to_json( name );
		}

		template<typename T>
		std::string value_to_json( daw::string_view name,
		                           daw::optional_poly<T> const &value ) {
			if( value ) {
				return value_to_json( name, *value );
			}
			return value_to_json( name );
		}

		template<typename T>
		void value_to_json( daw::string_view name,
		                    std::shared_ptr<T> const &value ) {
			if( !value ) {
				value_to_json( name );
			}
			value_to_json( name, *value );
		}

		template<typename T>
		void value_to_json( daw::string_view name, std::weak_ptr<T> const &value ) {
			if( !value.expired( ) ) {
				auto const shared_value = value.lock( );
				if( !shared_value ) {
					value_to_json( name );
				}
				value_to_json( name, *shared_value );
			}
		}
	} // namespace generate
} // namespace daw::json
