// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include <daw/json/daw_json_interface.h>
#include <daw/json/daw_value_to_json.h>

#include <daw/daw_move.h>
#include <daw/daw_range.h>
#include <daw/daw_string_view.h>

#include <sstream>
#include <string>
#include <utility>

namespace daw::json::generate {
	using namespace daw::json::details;

	std::string undefined_value_to_json( daw::string_view ) noexcept {
		return { };
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
			  : value_name{ daw::move( n ) } {}
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
		return value.apply_visitor( get_json_string_t{ daw::move( name ) } );
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

	// string
	std::string value_to_json( daw::string_view name, std::string const &value ) {
		return json_name( name ) + enquote( value );
	}

	std::string value_to_json( json_string_value name,
	                           std::string const &value ) {
		return value_to_json( to_string_view( name ), value );
	}
} // namespace daw::json::generate
