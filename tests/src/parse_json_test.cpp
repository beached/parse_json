// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include "daw/json/daw_json_parser.h"

#include <daw/daw_memory_mapped_file.h>
#include <daw/daw_move.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace daw::json;

struct Streamable {
	std::string a;

	Streamable( )
	  : a( "This is a test!" ) {}

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

/*
struct A : public daw::json::JsonLink<A> {
    int a;
    int b;
    double c;
    std::vector<int> d;
    bool e;

    A( ) : JsonLink<A>{}, a{1}, b{2}, c{1.23456789}, d{100, 5}, e{true} {

        set_jsonlinks( );
    }

    ~A( );

    void set_jsonlinks( ) {
        link_integer( "a", a );
        link_integer( "b", b );
        link_real( "c", c );
        link_array( "d", d );
        link_boolean( "e", e );
    }

    friend bool operator==( A const &lhs, A const &rhs ) {
        return std::tie( lhs.a, lhs.b, lhs.c ) == std::tie( rhs.a, rhs.b, rhs.c
) && std::equal( lhs.d.begin( ), lhs.d.end( ), rhs.d.begin( ), rhs.d.end( ) );
    }

    friend bool operator!=( A const &lhs, A const &rhs ) {
        return std::tie( lhs.a, lhs.b, lhs.c ) != std::tie( rhs.a, rhs.b, rhs.c
) || !std::equal( lhs.d.begin( ), lhs.d.end( ), rhs.d.begin( ), rhs.d.end( ) );
    }
};

A::~A( ) {}

struct B : public daw::json::JsonLink<B> {
    A a;
    std::string b;
    Streamable c;
    double d;

    B( ) : JsonLink<B>{}, a{}, b{"hello"}, c{}, d{1.9233434e-12} {

        set_jsonlinks( );
    }

    ~B( );

    void set_jsonlinks( ) {
        link_object( "a", a );
        link_string( "b", b );
        link_streamable( "c", c );
        link_real( "d", d );
    }

    friend bool operator==( B const &lhs, B const &rhs ) {
        return std::tie( lhs.a, lhs.b, lhs.c ) == std::tie( rhs.a, rhs.b, rhs.c
);
    }

    friend bool operator!=( B const &lhs, B const &rhs ) {
        return std::tie( lhs.a, lhs.b, lhs.c ) != std::tie( rhs.a, rhs.b, rhs.c
);
    }
};

B::~B( ) {}

struct C : public daw::json::JsonLink<C> {
    std::optional<int> a;
    C( ) : daw::json::JsonLink<C>{}, a{} {

        set_jsonlinks( );
    }

    ~C( );

    void set_jsonlinks( ) {
        link_integer( "a", a );
    }
};

C::~C( ) {}

template<typename K, typename V>
bool operator==( std::unordered_map<K, V> const &a, std::unordered_map<K, V>
const &b ) { return a.size( ) == a.size( ) && std::equal( a.begin( ), a.end( ),
b.begin( ), b.end( ) );
}

template<typename Stream>
auto fsize( Stream &stream ) -> decltype( stream.tellg( ) ) {
    auto cur_pos = stream.tellg( );
    stream.seekg( 0, std::fstream::end );
    auto result = stream.tellg( );
    stream.seekg( cur_pos );
    return result;
}

 BOOST_AUTO_TEST_CASE( SimpleTest ) {
    B b;
    auto enc = b.to_string( );
    auto parsed = daw::json::parse_json( enc );
    B c;
    c.from_json_obj( parsed );
    BOOST_CHECK_EQUAL( b, c );
}

 BOOST_AUTO_TEST_CASE( MapValues ) {
    std::unordered_map<std::string, B> test_umap;
    test_umap["a"] = B( );
    auto enc = daw::json::generate::value_to_json( "test_umap", test_umap );
    auto parsed = daw::json::parse_json( enc );
    std::unordered_map<std::string, B> test_umap2;
    daw::json::parse::json_to_value( test_umap2, *parsed );
    BOOST_REQUIRE( test_umap == test_umap2 );
}

struct Test : public daw::json::JsonLink<Test> {
    int b;
    double c;
    std::string d;
    std::string e;

  private:
    void link_values( ) {
        link_integer( "b", b );
        link_real( "c", c );
        link_string( "d", d );
        link_string( "e", e );
    }

  public:
    Test( )
        : daw::json::JsonLink<Test>{}
        , // Root objects must be nameless or it isn't valid json
        b{0}
        , c{0.0}
        , d{}
        , e{} {

        link_values( );
    }

    ~Test( );

    Test( Test const &other )
        : daw::json::JsonLink<Test>{}
        , // Root objects must be nameless or it isn't valid json
        b{other.b}
        , c{other.c}
        , d{other.d}
        , e{other.e} {

        link_values( );
    }

    Test( Test &&other )
        : daw::json::JsonLink<Test>{}
        , // Root objects must be nameless or it isn't valid json
        b{daw::move( other.b )}
        , c{daw::move( other.c )}
        , d{daw::move( other.d )}
        , e{daw::move( other.e )} {

        link_values( );
    }

    Test &operator=( Test const &rhs ) {
        if( this != &rhs ) {
            Test tmp{rhs};
            using std::swap;
            swap( *this, tmp );
        }
        return *this;
    }

    Test &operator=( Test &&rhs ) {
        if( this != &rhs ) {
            Test tmp{daw::move( rhs )};
            using std::swap;
            swap( *this, tmp );
        }
        return *this;
    }

    void something( ) {
        unlink( "e" );
    }
};

Test::~Test( ) {}
*/

int main( int argc, char **argv ) {
	if( argc < 2 ) {
		std::cerr << "Must supply a filename to open\n";
		exit( 1 );
	}
	auto const json_data =
	  daw::filesystem::memory_mapped_file_t<char>( std::string_view( argv[1] ) );
	daw::string_view json_sv( json_data.data( ), json_data.size( ) );
	auto obj = daw::json::parse_json( json_sv );
	/*
	    C ccls;
	    std::cout << ccls.to_string( ) << std::endl;
	    ccls.a = 1;
	    std::cout << ccls.to_string( ) << std::endl;
	    Test a;
	    a.b = 1234;
	    a.c = 10.001;
	    a.d = "\"This is a string\nline two";
	    a.e = "15\u00B0C";
	    std::cout << "to_string:" << std::endl;
	    auto const json_str = a.to_string( );
	    std::cout << json_str << std::endl;

	    //	auto f = from_file<Test>( "file.json", true );
	    Test b;
	    std::cout << "from_string:" << std::endl;
	    b.from_string( json_str );
	    std::cout << "b.d=>'" << b.d << "'\n";
	    std::cout << "to_string:" << std::endl;
	    std::cout << b.to_string( ) << std::endl;
	    b.something( );
	    std::cout << b.to_string( ) << std::endl;
	    Test c;
	    c.something( );
	    c.from_string( json_str );
	    std::cout << c.to_string( ) << std::endl;
	    boost::filesystem::path f{"file.json"};
	    if( !exists( f ) ) {
	        std::cout << "Could not test from_file '" << f << "' does not
	   exist\n"; } else { auto g = from_file<Test>( "file.json" );
	    }
	    */
	return EXIT_SUCCESS;
}
