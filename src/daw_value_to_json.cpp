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

#include <sstream>
#include <string>
#include <utility>

#include <daw/daw_range.h>
#include <daw/daw_move.h>
#include <daw/daw_string_view.h>

#include "daw_json_interface.h"
#include "daw_value_to_json.h"

namespace daw {
	namespace json {
		namespace generate {
			using namespace daw::json::details;

			std::string undefined_value_to_json( daw::string_view ) noexcept {
				return {};
			}

			// null
			std::string value_to_json( daw::string_view name ) {
				return json_name( name ) + "null";
			}

			std::string value_to_json( json_string_value name ) {
				return value_to_json( to_string_view( name ) );
			}

			// bool
			std::string value_to_json( daw::string_view name, bool value ) {
				return json_name( name ) + ( value ? "true" : "false" );
			}

			std::string value_to_json( json_string_value name, bool value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// json_value_t
			std::string value_to_json( daw::string_view name,
			                           daw::json::json_value_t const &value ) {
				using daw::json::json_value_t;
				struct get_json_string_t {
					daw::string_view value_name;
					get_json_string_t( daw::string_view n )
					  : value_name{daw::move( n )} {}
					std::string operator( )( json_value_t::array_t const &v ) const {
						return value_to_json( value_name, v );
					}
					std::string operator( )( json_value_t::object_t const &v ) const {
						return value_to_json( value_name, v );
					}
					std::string operator( )( json_value_t::boolean_t const &v ) const {
						return value_to_json( value_name, v );
					}
					std::string operator( )( json_value_t::integer_t const &v ) const {
						return value_to_json( value_name, v );
					}
					std::string operator( )( json_value_t::real_t const &v ) const {
						return value_to_json( value_name, v );
					}
					std::string operator( )( json_value_t::string_t const &v ) const {
						return value_to_json( value_name, v );
					}
					std::string operator( )( json_value_t::null_t ) const {
						return value_to_json( value_name );
					}
				}; // get_json_string_t
				return value.apply_visitor( get_json_string_t{daw::move( name )} );
			}

			std::string value_to_json_value( json_string_value name,
			                                 daw::json::json_value_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// json_object_value
			std::string
			value_to_json_object( daw::string_view name,
			                      daw::json::json_object_value const &object ) {
				std::stringstream result;
				result << json_name( name ) << "{";
				auto range = daw::range::make_range( object.members_v.begin( ),
				                                     object.members_v.end( ) );
				if( !range.empty( ) ) {
					result << value_to_json_value( range.front( ).first,
					                               range.front( ).second );
					range.move_next( );
					for( auto const &value : range ) {
						result << ", " << value_to_json_value( value.first, value.second );
					}
				}

				result << "}";
				return result.str( );
			}

			std::string value_to_json( json_string_value name,
			                           daw::json::json_object_value const &object ) {
				return value_to_json( to_string_view( name ), object );
			}

			// double
			std::string value_to_json( daw::string_view name, double const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( json_string_value name, double const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// int32_t
			std::string value_to_json( daw::string_view name, int32_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( json_string_value name,
			                           int32_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// int64_t
			std::string value_to_json( daw::string_view name, int64_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( json_string_value name,
			                           int64_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// intmax_t
			std::string value_to_json( daw::string_view name,
			                           intmax_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( json_string_value name,
			                           intmax_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// string
			std::string value_to_json( daw::string_view name,
			                           std::string const &value ) {
				return json_name( name ) + enquote( value );
			}

			std::string value_to_json( json_string_value name,
			                           std::string const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// uint32_t
			std::string value_to_json( daw::string_view name,
			                           uint32_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( json_string_value name,
			                           uint32_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}

			// uint64_t
			std::string value_to_json( daw::string_view name,
			                           uint64_t const &value ) {
				return value_to_json_number( name, value );
			}

			std::string value_to_json( json_string_value name,
			                           uint64_t const &value ) {
				return value_to_json( to_string_view( name ), value );
			}
		} // namespace generate
	}   // namespace json
} // namespace daw
