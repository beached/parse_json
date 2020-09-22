// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include "daw/json/daw_json_parser_v2.h"
#include "daw/json/daw_json_parser_v2_state.h"

#include <boost/iostreams/device/mapped_file.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>

int main( int argc, char **argv ) {
	boost::iostreams::mapped_file mmap( argv[1],
	                                    boost::iostreams::mapped_file::readonly );

	try {
		daw::json::state::state_control_t state_proxy;
		auto const ts_start = std::chrono::system_clock::now( );
		daw::json::json_parser( mmap.const_data( ),
		                        mmap.const_data( ) + mmap.size( ), state_proxy );
		auto const ts_end = std::chrono::system_clock::now( );

		auto const duration =
		  std::chrono::duration<double>( ts_end - ts_start ).count( );
		double const sz = mmap.size( );
		std::cout << duration << "s duration. "
		          << state_proxy.current_state( ).to_string( ) << std::endl;
		double const sp = sz / duration;
		std::cout << "speed " << sp << "bytes/sec " << ( ( sp / 1024.0 ) / 1024.0 )
		          << "MB/s\n";
	} catch( std::runtime_error const &ex ) {
		std::cerr << "Exception '" << ex.what( ) << "'\n";
		exit( EXIT_FAILURE );
	}
	return EXIT_SUCCESS;
}
