// The MIT License (MIT)
//
// Copyright (c) 2014-2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define BOOST_TEST_MODULE json_file_parser_test
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/test/unit_test.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "daw_json.h"
#include <daw/daw_memory_mapped_file.h>

using namespace daw::json;

template<typename String>
bool path_begins_with( boost::filesystem::path const &p, String const &str ) {
	auto const &fname = p.filename( ).string( );
	return 0 == fname.find( str );
}

BOOST_AUTO_TEST_CASE( test_files ) {
	boost::filesystem::path test_file_path{"tests/test_parsing/"};
	BOOST_REQUIRE( exists( test_file_path ) );

	for( auto const &de : boost::make_iterator_range(
	       boost::filesystem::directory_iterator{test_file_path} ) ) {
		auto const &path = de.path( );
		std::cout << "Testing: " << path.stem( ) << '\n';
		bool success = false;
		try {
			auto const p = path.native( );
			auto json_file =
			  daw::filesystem::memory_mapped_file_t<char>( {p.data( ), p.size( )} );
			if( json_file ) {
				volatile auto const json_obj =
				  daw::json::parse_json( json_file.cbegin( ), json_file.cend( ) );
				success = true;
			}
		} catch( ... ) { success = false; }
		if( path_begins_with( path, "y_" ) ) {
			BOOST_REQUIRE_MESSAGE( success, "Failed parsing required file " +
			                                  path.filename( ).string( ) );
		} else if( path_begins_with( path, "n_" ) ) {
			BOOST_REQUIRE_MESSAGE( !success, "Unsuccessfuly failed to parse file " +
			                                   path.filename( ).string( ) );
		} else {
			BOOST_CHECK_MESSAGE( success, "Failed parsing optional file " +
			                                path.filename( ).string( ) );
		}
	}
}
