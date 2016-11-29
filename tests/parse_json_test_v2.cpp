// The MIT License (MIT)
//
// Copyright (c) 2014-2016 Darrell Wright
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

//#define BOOST_TEST_MODULE parse_json_test
//#include <boost/test/unit_test.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

#include <daw/daw_exception.h>

#include "daw_json_parser_v2.h"

#include "value_t.h"

using namespace daw::json;

using daw::json::impl::value_t;

enum class current_state_t { none = 0, in_object_name, in_object_value, in_array, current_state_t_size };

struct state_t {
	state_t( ) = default;
	state_t( state_t const & ) = default;
	state_t( state_t && ) = default;
	state_t & operator=( state_t const & ) = default;
	state_t & operator=( state_t && ) = default;

	virtual ~state_t( );

	virtual void on_object_begin( );
	virtual void on_object_end( );
	virtual void on_array_begin( );
	virtual void on_array_end( );
	virtual void on_string( boost::string_view );
	virtual void on_integral( boost::string_view );
	virtual void on_real( boost::string_view );
	virtual void on_boolean( bool ); 
	virtual void on_null( );
	virtual std::string to_string( ) const = 0;
};	// state_t

struct state_in_object_name_t: public state_t {
	state_in_object_name_t( ) = default;
	state_in_object_name_t( state_in_object_name_t const & ) = default;
	state_in_object_name_t( state_in_object_name_t && ) = default;
	state_in_object_name_t & operator=( state_in_object_name_t const & ) = default;
	state_in_object_name_t & operator=( state_in_object_name_t && ) = default;

	~state_in_object_name_t( );
	std::string to_string( ) const override;
	void on_string( boost::string_view value ) override;
	void on_object_end( ) override;
};	// state_in_object_name

struct state_in_object_value_t: public state_t {
	state_in_object_value_t( ) = default;
	state_in_object_value_t( state_in_object_value_t const & ) = default;
	state_in_object_value_t( state_in_object_value_t && ) = default;
	state_in_object_value_t & operator=( state_in_object_value_t const & ) = default;
	state_in_object_value_t & operator=( state_in_object_value_t && ) = default;

	~state_in_object_value_t( );

	std::string to_string( ) const override;
	void on_object_begin( ) override;
	void on_array_begin( ) override;
	void on_null( ) override;
	static value_t::integral_t to_integral( boost::string_view value );
	static value_t::real_t to_real( boost::string_view value );
	void on_integral( boost::string_view value ) override;
	void on_real( boost::string_view value ) override;
	void on_string( boost::string_view value ) override;
	void on_boolean( bool value ) override;
};	// state_in_object_value_t

struct state_in_array_t: public state_t {
	state_in_array_t( ) = default;
	state_in_array_t( state_in_array_t const & ) = default;
	state_in_array_t( state_in_array_t && ) = default;
	state_in_array_t & operator=( state_in_array_t const & ) = default;
	state_in_array_t & operator=( state_in_array_t && ) = default;

	~state_in_array_t( );

	std::string to_string( ) const override;
	void on_object_begin( ) override;
	void on_array_begin( ) override;
	void on_array_end( ) override;
	void on_null( ) override;
	void on_integral( boost::string_view value ) override;
	void on_real( boost::string_view value ) override;
	void on_string( boost::string_view value ) override;
	void on_boolean( bool value ) override;
};	// state_in_array_t

struct state_none_t: public state_t {
	state_none_t( ) = default;
	state_none_t( state_none_t const & ) = default;
	state_none_t( state_none_t && ) = default;
	state_none_t & operator=( state_none_t const & ) = default;
	state_none_t & operator=( state_none_t && ) = default;

	~state_none_t( );

	std::string to_string( ) const override;
	void on_object_begin( ) override;
	void on_array_begin( ) override;
	void on_null( ) override;
	void on_integral( boost::string_view value ) override;
	void on_real( boost::string_view value ) override;
	void on_string( boost::string_view value ) override;
	void on_boolean( bool value ) override;
};	// state_none_t

state_t * get_state_fn( current_state_t s ) noexcept;
std::vector<state_t*> & state_stack( );
state_t & current_state( );
void push_and_set_next_state( current_state_t s );
void set_next_state( current_state_t s );
void pop_state( );
std::vector<daw::json::impl::value_t*> & value_stack( );
daw::json::impl::value_t & current_value( );
void push_and_set_next_value( daw::json::impl::value_t value );
void push_value( daw::json::impl::value_t * val );
void pop_value( ); 
state_t * get_state_fn( current_state_t s ) noexcept;

state_t::~state_t( ) { }

void state_t::on_object_begin( ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_object_begin" ); 
}

void state_t::on_object_end( ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_object_end" ); 
}

void state_t::on_array_begin( ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_array_begin" );
}

void state_t::on_array_end( ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_array_end" );
}

void state_t::on_string( boost::string_view ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_string" );
}

void state_t::on_integral( boost::string_view ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_integral" );
}

void state_t::on_real( boost::string_view ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_real" );
}

void state_t::on_boolean( bool ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_boolean" );
}

void state_t::on_null( ) { 
	throw std::runtime_error( this->to_string( ) + ": Unexpected state change: on_null" );
}

std::vector<state_t*> & state_stack( ) {
	static std::vector<state_t*> result = { get_state_fn( current_state_t::none ) };
	return result;
}

state_t & current_state( ) {
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

std::vector<daw::json::impl::value_t*> & value_stack( ) {
	static std::unique_ptr<daw::json::impl::value_t> root_value;
	static std::vector<daw::json::impl::value_t *> result = { root_value.get( ) };
	return result;
}

daw::json::impl::value_t & current_value( ) {
	return *value_stack( ).back( );
}

void push_and_set_next_value( daw::json::impl::value_t value ) {
	//value_stack( ).push_back( std::move( value ) );
}

void push_value( daw::json::impl::value_t * val ) {
	//value_stack( ).push_back( val );
}

void pop_value( ) {
	//value_stack( ).pop_back( );
}

using daw::json::impl::value_t;

value_t::integral_t to_integral( boost::string_view value ) {

	return 0;
}

value_t::real_t to_real( boost::string_view value ) {
	return 0.0;
}

//
// state_in_object_name
//

state_in_object_name_t::~state_in_object_name_t( ) { }

std::string state_in_object_name_t::to_string( ) const {
	return "state_in_object_name";
}

void state_in_object_name_t::on_string( boost::string_view value ) {
	value_t name{ value };		

	// Set current object name value_t
	set_next_state( current_state_t::in_object_value );
}

void state_in_object_name_t::on_object_end( ) {
	// Save value_t
	// Assumes state is not empty	
	pop_state( );
}

state_in_object_value_t::~state_in_object_value_t( ) { }

std::string state_in_object_value_t::to_string( ) const {
	return "state_in_object_value";
}

void state_in_object_value_t::on_object_begin( ) {
	// Save data
	push_and_set_next_value( value_t{ } );
	set_next_state( current_state_t::in_object_name );
	push_and_set_next_state( current_state_t::in_object_name );
}

void state_in_object_value_t::on_array_begin( ) {
	push_and_set_next_value( value_t{ } );
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

void state_in_object_value_t::on_integral( boost::string_view value ) {
	// Save data
	//current_value( ) = value_t{ to_integral( value ) };
	set_next_state( current_state_t::in_object_name );
}

void state_in_object_value_t::on_real( boost::string_view value ) {
	// Save data
	//current_value( ) = value_t{ to_real( value ) };
	set_next_state( current_state_t::in_object_name );
}

void state_in_object_value_t::on_string( boost::string_view value ) {
	// Save data
	//current_value( ) = value_t{ value.to_string( ) };
	set_next_state( current_state_t::in_object_name );
}

void state_in_object_value_t::on_boolean( bool value ) {
	// Save data
	//current_value( ) = value_t{ value };
	set_next_state( current_state_t::in_object_name );
}

//
// state_in_array_t
//

state_in_array_t::~state_in_array_t( ) { }

std::string state_in_array_t::to_string( ) const {
	return "state_in_array";
}

void state_in_array_t::on_object_begin( )  {
	// Save data
	push_and_set_next_state( current_state_t::in_object_name );
}

void state_in_array_t::on_array_begin( )  {
	// Save data
	push_and_set_next_state( current_state_t::in_array );
}

void state_in_array_t::on_array_end( )  {
	// Save data
	pop_state( );
}
void state_in_array_t::on_null( )  {
	// Save data
}

void state_in_array_t::on_integral( boost::string_view value )  {
	// Save data
}

void state_in_array_t::on_real( boost::string_view value )  {
	// Save data
}

void state_in_array_t::on_string( boost::string_view value )  {
	// Save data
}

void state_in_array_t::on_boolean( bool value )  {
	// Save data
}

//
// state_none_t
//

state_none_t::~state_none_t( ) { }

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

void state_none_t::on_integral( boost::string_view value ) {
	// Save data
}

void state_none_t::on_real( boost::string_view value ) {
	// Save data
}

void state_none_t::on_string( boost::string_view value ) {
	// Save data
}

void state_none_t::on_boolean( bool value ) {
	// Save data
}

state_t * get_state_fn( current_state_t s ) noexcept {
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
			std::abort( );
	}
}

struct state_contol_t {
	std::string buffer;
	//void clear_buffer( ) { buffer.resize( 0 ); }
	void clear_buffer( ) { buffer.clear( ); }
	void push( char c ) { buffer.push_back( c ); } 
	void on_object_begin( ) { current_state( ).on_object_begin( ); }
	void on_object_end( ) { current_state( ).on_object_end( ); }
	void on_array_begin( ) { current_state( ).on_array_begin( ); }
	void on_array_end( ) { current_state( ).on_array_end( ); }
	void on_string( boost::string_view value ) { current_state( ).on_string( value ); }
	void on_integral( boost::string_view value ) { current_state( ).on_integral( value ); }
	void on_real( boost::string_view value ) { current_state( ).on_real( value ); }
	void on_boolean( bool value ) { current_state( ).on_boolean( value ); }
	void on_null( ) { current_state( ).on_null( ); }
} state_proxy;	

int main( int argc, char** argv ) {
	boost::iostreams::mapped_file mmap( argv[1], boost::iostreams::mapped_file::readonly );

	try {
		auto const ts_start = std::chrono::system_clock::now( );
		json_parser( mmap.const_data( ), mmap.const_data( ) + mmap.size( ), state_proxy );
		auto const ts_end = std::chrono::system_clock::now( );

		auto const duration = std::chrono::duration<double>(ts_end-ts_start).count( );
		double const sz = mmap.size( );
		std::cout << duration << "s duration. " << current_state( ).to_string( ) << std::endl;
		double const sp = sz/duration;
		std::cout << "speed " << sp << "bytes/sec " << ((sp/1024.0)/1024.0) << "MB/s\n";
	} catch( std::runtime_error const & ex ) {
		std::cerr << "Exception '" << ex.what( ) << "'\n";
		exit( EXIT_FAILURE );
	}
	return EXIT_SUCCESS;
}

