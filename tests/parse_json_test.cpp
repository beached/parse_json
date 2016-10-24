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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include "daw_json.h"
#include "daw_json_link.h"

using namespace daw::json;

struct Streamable {
	std::string a;

	Streamable( ):
			a( "This is a test!" ) { }

	bool operator==( Streamable const & rhs ) const {
		return a == rhs.a;
	}

	bool operator!=( Streamable const & rhs ) const {
		return a != rhs.a;
	}
};

std::istream & operator>>( std::istream & is, Streamable & value ) {
	auto const data_size = static_cast<size_t>(is.rdbuf( )->in_avail( ));
	value.a.reserve( data_size );
	is.read( &value.a[0], data_size );
	return is;
}

std::ostream & operator<<( std::ostream & os, Streamable const & value ) {
	os << value.a;
	return os;
}

struct A: public daw::json::JsonLink<A> {
	int a;
	int b;
	double c;
	std::vector<int> d;
	bool e;

	A( ):
			JsonLink<A>{ },
			a{ 1 },
			b{ 2 },
			c{ 1.23456789 },
			d{ 100, 5 },
			e{ true } {

		link_integral( "a", a );
		link_integral( "b", b );
		link_real( "c", c );
		link_array( "d", d );
		link_boolean( "e", e );
	}

	friend bool operator==( A const & lhs, A const & rhs ) {
		return std::tie( lhs.a, lhs.b, lhs.c ) == std::tie( rhs.a, rhs.b, rhs.c ) && std::equal( lhs.d.begin( ), lhs.d.end( ), rhs.d.begin( ), rhs.d.end( ) );
	}

	friend bool operator!=( A const & lhs, A const & rhs ) {
		return std::tie( lhs.a, lhs.b, lhs.c ) != std::tie( rhs.a, rhs.b, rhs.c ) || !std::equal( lhs.d.begin( ), lhs.d.end( ), rhs.d.begin( ), rhs.d.end( ) );
	}
};

struct B: public daw::json::JsonLink<B> {
	A a;
	std::string b;
	Streamable c;
	double d;

	B( ):
			JsonLink<B>{ },
			a{ },
			b{ "hello" },
			c{ },
			d{ 1.9233434e-12 } {

		link_object( "a", a );
		link_string( "b", b );
		link_streamable( "c", c );
		link_real( "d", d );
	}

	friend bool operator==( B const & lhs, B const & rhs ) {
		return std::tie( lhs.a, lhs.b, lhs.c ) == std::tie( rhs.a, rhs.b, rhs.c );
	}

	friend bool operator!=( B const & lhs, B const & rhs ) {
		return std::tie( lhs.a, lhs.b, lhs.c ) != std::tie( rhs.a, rhs.b, rhs.c );
	}
};

struct C : public daw::json::JsonLink<C> {
	boost::optional<int> a;
	C( ):
			daw::json::JsonLink<C>{ },
			a{ } {
		
		link_integral( "a", a );
	}
};

template<typename K, typename V>
bool operator==( std::unordered_map<K, V> const & a, std::unordered_map<K, V> const & b ) {
	return a.size( ) == a.size( ) && std::equal( a.begin( ), a.end( ), b.begin( ), b.end( ) );
}

template<typename Stream>
auto fsize( Stream & stream ) -> decltype( stream.tellg( ) ) {
	auto cur_pos = stream.tellg( );
	stream.seekg( 0, std::fstream::end );
	auto result = stream.tellg( );
	stream.seekg( cur_pos );
	return result;
}

//BOOST_AUTO_TEST_CASE( SimpleTest ) {
//	B b;
//	auto enc = b.encode( );
//	auto parsed = daw::json::parse_json( enc );
//	B c;
//	c.decode( parsed );
//	BOOST_CHECK_EQUAL( b, c );
//}
//
//BOOST_AUTO_TEST_CASE( MapValues ) {
//	std::unordered_map<std::string, B> test_umap;
//	test_umap["a"] = B( );
//	auto enc = daw::json::generate::value_to_json( "test_umap", test_umap );
//	auto parsed = daw::json::parse_json( enc );
//	std::unordered_map<std::string, B> test_umap2;
//	daw::json::parse::json_to_value( test_umap2, *parsed );
//	BOOST_REQUIRE( test_umap == test_umap2 );
//}
//
struct Test: public daw::json::JsonLink<Test> {
	int b;
	double c;
	std::string d;
	std::string e;

	Test( ):
			daw::json::JsonLink<Test>{ },    // Root objects must be nameless or it isn't valid json
			b{ 0 },
			c{ 0.0 },
			d{ },
			e{ } {

		link_integral( "b", b );
		link_real( "c", c );
		link_string( "d", d );
		link_string( "e", e );
	}

	void something( ) {
		unlink( "e" );
	}
};


int main( int, char ** ) {
	C ccls;
	std::cout << ccls.encode( ) << std::endl;
	ccls.a = 1;
	std::cout << ccls.encode( ) << std::endl;
	Test a;
	a.b = 1234;
	a.c = 10.001;
	a.d = "\"This is a string\nline two";
	a.e = "15\u00B0C";
	std::cout << "encode:" << std::endl;
	auto const s = a.encode( );
	std::cout << s << std::endl;

//	auto f = from_file<Test>( "file.json", true );
	Test b;
	std::cout << "decode:" << std::endl;
	b.decode( s );
	std::cout << "b.d=>'" << b.d << "'\n";
	std::cout << "encode:" << std::endl;
	std::cout << b.encode( ) << std::endl;
	b.something( );
	std::cout << b.encode( ) << std::endl;
	Test c;
	c.something( );
	c.decode( s );
	std::cout << c.encode( ) << std::endl;

	auto g = from_file<Test>( "file.json" );
	return EXIT_SUCCESS;
}

