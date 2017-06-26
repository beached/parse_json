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

#include "daw_json_link_v2.h"

struct test_t final : public daw::json::json_link<test_t> {
	int32_t a;
	int64_t b;
	std::string c;
	double d;
	int16_t e;
	bool f;
	std::vector<int> g;

	static void map_to_json( ) {
		json_link_integer( "a", a );
		json_link_integer_fn( "b", []( test_t const &obj ) { return obj.b; },
		                      []( test_t &obj, auto const &value ) { obj.b = value; } );
		json_link_string( "c", c );
		json_link_real( "d", d );
		json_link_integer( "e", e );
		json_link_boolean( "f", f );
		json_link_integer_array( "g", g );
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
	test2_t t;
	std::cout << "size of test_t-> " << sizeof( test_t ) << " data member total sizes-> " << expected_size << '\n';
	std::cout << "size of base->" << sizeof( daw::json::json_link<test_t> ) << '\n';
	t.a.a = 1;
	t.a.b = 2;
	t.a.c = "three";
	t.a.d = 4.5;
	t.a.e = 6;
	t.a.f = true;
	t.a.g.push_back( 7 );
	t.a.g.push_back( 8 );
	t.a.g.push_back( 9 );
	std::cout << t.to_json_string( ) << '\n';
	return EXIT_SUCCESS;
}
