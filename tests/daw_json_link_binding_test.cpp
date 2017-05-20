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

struct test: public daw::json::json_link<test> {
	int32_t a;
	int64_t b;
	std::string c;
	double d;
	int16_t e;

	static void map_to_json( ) {
		json_link_integer( "a", a );
		json_link_integer_fn( "b", []( test const & obj ) { return obj.b; }, []( test & obj, auto const & value ) { obj.b = value; } );
		json_link_string( "c", c );
		json_link_real( "d", d );
		json_link_integer( "e", e );
	}
};	// test

struct test2: public daw::json::json_link<test2> {
	test a;

	static void map_to_json( ) {
		json_link_object( "a", a );
	}
};	// test2

int main( int, char** ) {
	test2 t;
	t.a.a = 5;
	std::cout << t.to_json_string( ) << '\n';
	return EXIT_SUCCESS;
}


