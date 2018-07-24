// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Darrell Wright
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

#include <iomanip>
#include <limits>
#include <memory>
#include <string>
#include <sstream>
#include <type_traits>
#include <utility>

#include <daw/daw_range.h>
#include <daw/daw_string_view.h>

#include "daw_json_interface.h"

namespace daw {
	namespace json {
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

			std::string value_to_json( daw::string_view name, double const &value );
			std::string value_to_json( json_string_value name, double const &value );

			std::string value_to_json( daw::string_view name, int32_t const &value );
			std::string value_to_json( json_string_value name, int32_t const &value );

			std::string value_to_json( daw::string_view name, int64_t const &value );
			std::string value_to_json( json_string_value name, int64_t const &value );

			std::string value_to_json( daw::string_view name, intmax_t const &value );
			std::string value_to_json( json_string_value name,
			                           intmax_t const &value );

			std::string value_to_json( daw::string_view name,
			                           std::string const &value );
			std::string value_to_json( json_string_value name,
			                           std::string const &value );

			std::string value_to_json( daw::string_view name, uint32_t const &value );
			std::string value_to_json( json_string_value name,
			                           uint32_t const &value );

			std::string value_to_json( daw::string_view name, uint64_t const &value );
			std::string value_to_json( json_string_value name,
			                           uint64_t const &value );

			template<typename Container,
			         typename std::enable_if_t<
			           daw::traits::is_container_not_string_v<Container>, long> = 0>
			std::string value_to_json( daw::string_view name,
			                           Container const &values );

			template<typename Derived>
			std::string value_to_json( daw::string_view name,
			                           JsonLink<Derived> const &obj );

			template<typename First, typename Second>
			std::string value_to_json( daw::string_view name,
			                           std::pair<First, Second> const &value );

			template<typename Number, typename std::enable_if_t<
			                            daw::is_floating_point_v<Number>, int> = 0>
			std::string value_to_json_number( daw::string_view name,
			                                  Number const &value );

			template<typename Number,
			         typename std::enable_if_t<daw::is_integral_v<Number>, int> = 0>
			std::string value_to_json_number( daw::string_view name,
			                                  Number const &value );

			template<typename T>
			std::string value_to_json( daw::string_view name,
			                           boost::optional<T> const &value );

			template<typename T>
			std::string value_to_json( daw::string_view name,
			                           daw::optional_poly<T> const &value );

			template<typename T>
			void value_to_json( daw::string_view name,
			                    std::shared_ptr<T> const &value );

			template<typename T>
			void value_to_json( daw::string_view name,
			                    std::weak_ptr<T> const &value );

			template<typename Container,
			         typename std::enable_if_t<
			           daw::traits::is_container_not_string_v<Container>, long>>
			std::string value_to_json( daw::string_view name,
			                           Container const &values ) {
				daw::string_view const empty_str{""};
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
			         typename std::enable_if_t<daw::is_floating_point_v<Number>, int>>
			std::string value_to_json_number( daw::string_view name,
			                                  Number const &value ) {
				std::stringstream ss;
				ss << daw::json::details::json_name( name );
				ss << std::setprecision( std::numeric_limits<Number>::max_digits10 )
				   << value;
				return ss.str( );
			}

			template<typename Number,
			         typename std::enable_if_t<daw::is_integral_v<Number>, int>>
			std::string value_to_json_number( daw::string_view name,
			                                  Number const &value ) {
				return daw::json::details::json_name( name ) + std::to_string( value );
			}

			template<typename T>
			std::string value_to_json( daw::string_view name,
			                           boost::optional<T> const &value ) {
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
			void value_to_json( daw::string_view name,
			                    std::weak_ptr<T> const &value ) {
				if( !value.expired( ) ) {
					auto const shared_value = value.lock( );
					if( !shared_value ) {
						value_to_json( name );
					}
					value_to_json( name, *shared_value );
				}
			}
		} // namespace generate
	}   // namespace json
} // namespace daw

