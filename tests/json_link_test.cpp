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

//#define BOOST_TEST_MODULE parse_json_test
//#include <boost/test/unit_test.hpp>

#include <boost/optional.hpp>
#include <chrono>
#include <cstdlib>
#include <date/date.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "daw_json.h"
#include "daw_json_link.h"

using namespace daw::json;
using namespace date;
using namespace std::chrono;

struct Streamable {
	std::string a;

	Streamable( ) : a( "This is a test!" ) {}

	bool operator==( Streamable const &rhs ) const {
		return a == rhs.a;
	}

	bool operator!=( Streamable const &rhs ) const {
		return a != rhs.a;
	}
};

std::istream &operator>>( std::istream &is, Streamable &value ) {
	auto const data_size = static_cast<size_t>( is.rdbuf( )->in_avail( ) );
	value.a.reserve( data_size );
	is.read( &value.a[0], static_cast<long>( data_size ) );
	return is;
}

std::ostream &operator<<( std::ostream &os, Streamable const &value ) {
	os << value.a;
	return os;
}

struct A : public daw::json::JsonLink<A> {
	int integer_01;
	boost::optional<int> integer_02;
	double real_01;
	boost::optional<double> real_02;
	std::vector<int> array_01;
	boost::optional<std::vector<int>> array_02;
	bool boolean_01;
	boost::optional<bool> boolean_02;
	Streamable streamable_01;
	boost::optional<Streamable> streamable_02;
	std::string string_01;
	boost::optional<std::string> string_02;
	std::vector<int> custom_01;
	std::chrono::system_clock::time_point timestamp_01;
	std::chrono::system_clock::time_point timestamp_02;
	std::vector<unsigned int> hexstring_01;
	std::vector<unsigned int> hexstring_02;
	std::vector<uint8_t> hexstring_03;
	uint32_t hexvalue_01;

	A( )
	    : JsonLink<A>{}
	    , integer_01{}
	    , integer_02{}
	    , real_01{}
	    , real_02{}
	    , array_01{}
	    , array_02{}
	    , boolean_01{}
	    , boolean_02{}
	    , streamable_01{}
	    , streamable_02{}
	    , string_01{}
	    , string_02{}
	    , custom_01{{2, 4, 6}}
	    , timestamp_01{}
	    , timestamp_02{std::chrono::system_clock::now( )}
	    , hexstring_01{{0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536}}
	    , hexstring_02{{0xFF, 0x1111, 0xABCDEF, 0x12345}}
	    , hexstring_03{{0x11, 0xAA, 0xAB, 0xFF, 0x00, 0x01}}
	    , hexvalue_01{0x12345678} {

		set_jsonlinks( );
	}

	~A( ) final;

	void set_jsonlinks( ) {
		link_integer( "integer_01", integer_01 );
		link_integer( "integer_02", integer_02 );
		link_real( "real_01", real_01 );
		link_real( "real_02", real_02 );
		link_array( "array_01", array_01 );
		link_array( "array_02", array_02 );
		link_boolean( "boolean_01", boolean_01 );
		link_boolean( "boolean_02", boolean_02 );
		link_streamable( "streamable_01", streamable_01 );
		link_streamable( "streamable_02", streamable_02 );
		link_string( "string_01", string_01 );
		link_string( "string_02", string_02 );
		link_jsonstring( "custom_01", custom_01,
		                 []( std::vector<int> const &v ) -> std::string {
			                 std::stringstream result;
			                 std::copy( v.begin( ), v.end( ), std::ostream_iterator<int>{result, " "} );
			                 return result.str( );
		                 },
		                 []( std::string const &str ) -> std::vector<int> {
			                 std::vector<int> result;
			                 std::istringstream iss{str};
			                 std::copy( std::istream_iterator<int>{iss}, std::istream_iterator<int>{},
			                            std::back_inserter( result ) );
			                 return result;
		                 } );

		link_iso8601_timestamp( "timestamp_01", timestamp_01 );
		link_epoch_milliseconds_timestamp( "timestamp_02", timestamp_02 );
		link_hex_array( "hex_string_01", hexstring_01 );
		link_hex_array( "hex_string_02", hexstring_02 );
		link_hex_array( "hex_string_03", hexstring_03 );
		link_hex_value( "hex_value_01", hexvalue_01 );
	}
}; // A

A::~A( ) = default;

struct B : public daw::json::JsonLink<B> {
	A object_01;
	boost::optional<A> object_02;

	B( ) : daw::json::JsonLink<B>{}, object_01{}, object_02{} {

		set_jsonlinks( );
	}

	~B( ) final;

	void set_jsonlinks( ) {
		link_object( "object_01", object_01 );
		link_object( "object_02", object_02 );
	}
}; // B

B::~B( ) = default;

struct empty : public daw::json::JsonLink<empty> {
	empty( ) : daw::json::JsonLink<empty>{} {}

	empty( empty const & ) : daw::json::JsonLink<empty>{} {}

	empty( empty && ) : daw::json::JsonLink<empty>{} {}

	~empty( ) final;
};

empty::~empty( ) = default;

int main( int, char ** ) {
	B obj_b_01;
	obj_b_01.object_01.integer_01 = 12345;
	auto const obj_b_01_str = obj_b_01.to_string( );
	std::cout << "obj_b_01->\n" << obj_b_01_str << '\n';
	B obj_b_02;
	obj_b_02.from_string( obj_b_01_str );
	std::cout << "obj_b_02->\n" << obj_b_02 << '\n';
	std::stringstream ss;
	ss << obj_b_02;
	B obj_b_03;
	ss >> obj_b_03;
	std::cout << "obj_b_03->\n" << obj_b_03 << '\n';

	std::cout << "sizeof( empty )->" << sizeof( empty ) << '\n';
	std::cout << "sizeof( B )->" << sizeof( B ) << '\n';
	std::cout << obj_b_03.object_01.streamable_01.a << std::endl;
	return EXIT_SUCCESS;
}
