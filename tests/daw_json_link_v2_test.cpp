// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
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

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "daw_json_link.h"

struct test_t final : public daw::json::json_link<test_t> {
	std::vector<int> a;
	int32_t b;
	int64_t c;
	std::string d;
	double e;
	int16_t f;
	bool g;

	static void map_to_json( ) {
		json_link_integer_array( "a", a );
		json_link_integer( "b", b );
		json_link_integer_fn( "c", []( test_t const &obj ) { return obj.c; },
		                      []( test_t &obj, auto const &value ) { obj.c = value; } );
		json_link_string( "d", d );
		json_link_real( "e", e );
		json_link_integer( "f", f );
		json_link_boolean( "g", g );
	}
}; // test_t

constexpr auto const expected_size = sizeof( int32_t ) + sizeof( int64_t ) + sizeof( std::string ) + sizeof( double ) +
                                     sizeof( int16_t ) + sizeof( bool ) + sizeof( std::vector<int> );

struct test2_t : public daw::json::json_link<test2_t> {
	test_t a;

	static void map_to_json( ) {
		json_link_object( "a", a );
	}
}; // test2_t

int main( int, char ** ) {
	try {
		test2_t t;
		std::cout << "size of test_t-> " << sizeof( test_t ) << " data member total sizes-> " << expected_size << '\n';
		std::cout << "size of base->" << sizeof( daw::json::json_link<test_t> ) << '\n';
		t.a.a.push_back( 1 );
		t.a.a.push_back( 2 );
		t.a.a.push_back( 3 );
		t.a.b = 4;
		t.a.c = 5;
		t.a.d = "six";
		t.a.e = 7.8;
		t.a.f = 9;
		t.a.g = true;
		auto const jstr = t.to_json_string( );
		std::cout << jstr << '\n';

		auto t2 = test2_t::from_json_string( jstr );
		t2.a.b = 33;
		std::cout << t2.to_json_string( ) << '\n';

		test_t t3;
		t3.a.push_back( 9 );
		t3.a.push_back( 8 );
		t3.a.push_back( 7 );
		t3.b = 6;
		t3.c = 5;
		t3.d = "t3 works";
		t3.e = 4.3;
		t3.f = 2;
		t3.g = false;
		auto const t3str = t3.to_json_string( );
		std::cout << t3str << '\n';

		t.a = test_t::from_json_string( t3str );

		std::cout << t.to_json_string( ) << '\n';

		auto const f = daw::json::array_from_file<test2_t>( "test.json", true );

		for( auto const &item : f ) {
			std::cout << item.to_json_string( ) << '\n';
		}
	} catch( std::exception const &ex ) {
		std::cerr << "Exception caught: " << ex.what( ) << '\n';
		std::cerr << "Quiting" << std::endl;
		exit( EXIT_FAILURE );
	}
	return EXIT_SUCCESS;
}
