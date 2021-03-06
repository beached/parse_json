// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include <daw/json/daw_json_parser_v2_state.h>

#include <daw/daw_exception.h>
#include <daw/daw_string_view.h>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace daw::json::state {
	void state_t::on_object_begin( ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_object_begin" );
	}

	void state_t::on_object_end( ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_object_end" );
	}

	void state_t::on_array_begin( ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_array_begin" );
	}

	void state_t::on_array_end( ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_array_end" );
	}

	void state_t::on_string( daw::string_view ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_string" );
	}

	void state_t::on_integer( daw::string_view ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_integer" );
	}

	void state_t::on_real( daw::string_view ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_real" );
	}

	void state_t::on_boolean( bool ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_boolean" );
	}

	void state_t::on_null( ) {
		daw::exception::daw_throw<std::runtime_error>(
		  this->to_string( ) + ": Unexpected state change: on_null" );
	}

	std::vector<state_t *> &state_stack( ) {
		static std::vector<state_t *> result = {
		  get_state_fn( current_state_t::none ) };
		return result;
	}

	state_t &current_state( ) {
		return *state_stack( ).back( );
	}

	void push_and_set_next_state( current_state_t s ) {
		state_stack( ).push_back( get_state_fn( s ) );
	}

	void set_next_state( current_state_t s ) {
		state_stack( ).back( ) = get_state_fn( s );
	}

	void pop_state( ) {
		state_stack( ).pop_back( );
	}

	void pop_value( ) {
		// value_stack( ).pop_back( );
	}

	//
	// state_in_object_name
	//

	state_in_object_name_t::~state_in_object_name_t( ) {}

	std::string state_in_object_name_t::to_string( ) const {
		return "state_in_object_name";
	}

	void state_in_object_name_t::on_string( daw::string_view ) {
		// json_value_t name{ value };

		// Set current object name json_value_t
		set_next_state( current_state_t::in_object_value );
	}

	void state_in_object_name_t::on_object_end( ) {
		// Save json_value_t
		// Assumes state is not empty
		pop_state( );
	}

	state_in_object_value_t::~state_in_object_value_t( ) {}

	std::string state_in_object_value_t::to_string( ) const {
		return "state_in_object_value";
	}

	void state_in_object_value_t::on_object_begin( ) {
		// Save data
		// push_and_set_next_value( json_value_t{ } );
		set_next_state( current_state_t::in_object_name );
		push_and_set_next_state( current_state_t::in_object_name );
	}

	void state_in_object_value_t::on_array_begin( ) {
		// push_and_set_next_value( json_value_t{ } );
		set_next_state( current_state_t::in_object_name );
		push_and_set_next_state( current_state_t::in_array );
	}

	void state_in_object_value_t::on_null( ) {
		// Value is already null
		set_next_state( current_state_t::in_object_name );
	}

	//
	// state_in_object_value
	//

	void state_in_object_value_t::on_integer( daw::string_view ) {
		// Save data
		// current_value( ) = json_value_t{ to_integer( value ) };
		set_next_state( current_state_t::in_object_name );
	}

	void state_in_object_value_t::on_real( daw::string_view ) {
		// Save data
		// current_value( ) = json_value_t{ to_real( value ) };
		set_next_state( current_state_t::in_object_name );
	}

	void state_in_object_value_t::on_string( daw::string_view ) {
		// Save data
		// current_value( ) = json_value_t{ value.to_string( ) };
		set_next_state( current_state_t::in_object_name );
	}

	void state_in_object_value_t::on_boolean( bool ) {
		// Save data
		// current_value( ) = json_value_t{ value };
		set_next_state( current_state_t::in_object_name );
	}

	//
	// state_in_array_t
	//

	state_in_array_t::~state_in_array_t( ) {}

	std::string state_in_array_t::to_string( ) const {
		return "state_in_array";
	}

	void state_in_array_t::on_object_begin( ) {
		// Save data
		push_and_set_next_state( current_state_t::in_object_name );
	}

	void state_in_array_t::on_array_begin( ) {
		// Save data
		push_and_set_next_state( current_state_t::in_array );
	}

	void state_in_array_t::on_array_end( ) {
		// Save data
		pop_state( );
	}
	void state_in_array_t::on_null( ) {
		// Save data
	}

	void state_in_array_t::on_integer( daw::string_view ) {
		// Save data
	}

	void state_in_array_t::on_real( daw::string_view ) {
		// Save data
	}

	void state_in_array_t::on_string( daw::string_view ) {
		// Save data
	}

	void state_in_array_t::on_boolean( bool ) {
		// Save data
	}

	//
	// state_none_t
	//

	state_none_t::~state_none_t( ) {}

	std::string state_none_t::to_string( ) const {
		return "state_none";
	}

	void state_none_t::on_object_begin( ) {
		// Save data
		push_and_set_next_state( current_state_t::in_object_name );
	}

	void state_none_t::on_array_begin( ) {
		// Save data
		push_and_set_next_state( current_state_t::in_array );
	}

	void state_none_t::on_null( ) {
		// Save data
	}

	void state_none_t::on_integer( daw::string_view ) {
		// Save data
	}

	void state_none_t::on_real( daw::string_view ) {
		// Save data
	}

	void state_none_t::on_string( daw::string_view ) {
		// Save data
	}

	void state_none_t::on_boolean( bool ) {
		// Save data
	}

	state_t *get_state_fn( current_state_t s ) noexcept {
		static state_none_t s_none{ };
		static state_in_object_name_t s_in_object_name{ };
		static state_in_object_value_t s_in_object_value{ };
		static state_in_array_t s_in_array{ };
		switch( s ) {
		case current_state_t::none:
			return &s_none;
		case current_state_t::in_object_name:
			return &s_in_object_name;
		case current_state_t::in_object_value:
			return &s_in_object_value;
		case current_state_t::in_array:
			return &s_in_array;
		case current_state_t::current_state_t_size:
		default:
			std::cerr << "Unknown state" << std::endl;
			std::terminate( );
		}
	}

	state_t const &state_control_t::current_state( ) const {
		return daw::json::state::current_state( );
	}

	void state_control_t::clear_buffer( ) {
		buffer.clear( );
	}

	void state_control_t::push( char c ) {
		buffer.push_back( c );
	}

	void state_control_t::on_object_begin( ) const {
		daw::json::state::current_state( ).on_object_begin( );
	}

	void state_control_t::on_object_end( ) const {
		daw::json::state::current_state( ).on_object_end( );
	}

	void state_control_t::on_array_begin( ) const {
		daw::json::state::current_state( ).on_array_begin( );
	}

	void state_control_t::on_array_end( ) const {
		daw::json::state::current_state( ).on_array_end( );
	}

	void state_control_t::on_string( daw::string_view value ) const {
		daw::json::state::current_state( ).on_string( value );
	}

	void state_control_t::on_integer( daw::string_view value ) const {
		daw::json::state::current_state( ).on_integer( value );
	}

	void state_control_t::on_real( daw::string_view value ) const {
		daw::json::state::current_state( ).on_real( value );
	}

	void state_control_t::on_boolean( bool value ) const {
		daw::json::state::current_state( ).on_boolean( value );
	}

	void state_control_t::on_null( ) const {
		daw::json::state::current_state( ).on_null( );
	}
} // namespace daw::json::state
