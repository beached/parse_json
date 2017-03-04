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

struct test: public json_link_v2<test> {
	int32_t a;
	int64_t b;
	std::string c;
	double d;
	int16_t e;

	static auto bind_json( test & object ) {
		using std::to_string;
		json_expect_integral( "a", 
			[]( auto & obj, auto val ) { 
				obj.a = val; 
			},
			[]( auto const & obj ) { 
				return to_string( obj.a ); 
			} );

		json_expect_integral_string( "b", 
			[]( auto & obj, auto val ) { 
				auto const tmp = stoll( val.c_str( ) );
				obj.b = tmp; 
			},
			[]( auto const & obj ) {
				return to_string( obj.b );
			} );

		json_expect_string( "c", 
			[]( auto & obj, auto val ) {
				obj.c = val.to_string( );
			},
			[]( auto const & obj ) {
				return obj.c;
			} );

		json_expect_real( "d", 
			[]( auto & obj, auto val ) {
				obj.d = val;
			},
			[]( auto const & obj ) {
				return to_string( obj.d );
			} );

		JSON_EXPECT( real, "e", e );
	}
};	// test

struct test2: public json_link_v2<test2> {
	bool a;
	test b;

	static auto bind_json( test2 & object ) {
		using std::to_string;
		json_expect_boolean( "a", 
			[]( auto & obj, bool val ) {
				obj.a = val;
			},
			[]( auto const & obj ) {
				return to_string( obj.a );
			} );

		json_expect_object( "b", 
			[]( auto & obj, auto val ) {
				b = val;
			},
			[]( auto const & obj ) {
				return obj.to_json_string( );
			} );
	}
};	// test2


struct tmp {
	int n;
};

int main( int, char** ) {
	auto b = daw::json::json_bind_integral<tmp, &tmp::n>( );


	return EXIT_SUCCESS;
}


