// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include "daw/json/daw_json.h"

#include <daw/daw_memory_mapped_file.h>

#define BOOST_TEST_MODULE json_file_parser_test
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/test/unit_test.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace daw::json;

template<typename String>
bool path_begins_with( boost::filesystem::path const &p, String const &str ) {
	auto const &fname = p.filename( ).string( );
	return 0 == fname.find( str );
}

BOOST_AUTO_TEST_CASE( test_files ) {
	boost::filesystem::path test_file_path{ "tests/test_parsing/" };
	BOOST_REQUIRE( exists( test_file_path ) );

	for( auto const &de : boost::make_iterator_range(
	       boost::filesystem::directory_iterator{ test_file_path } ) ) {
		auto const &path = de.path( );
		std::cout << "Testing: " << path.stem( ) << '\n';
		bool success = false;
		try {
			auto const p = path.native( );
			auto json_file = daw::filesystem::memory_mapped_file_t<char>(
			  std::string( p.data( ), p.size( ) ).c_str( ) );
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
