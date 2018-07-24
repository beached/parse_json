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

#include <boost/iostreams/device/mapped_file.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>

#include "daw_json_parser_v2.h"
#include "daw_json_parser_v2_state.h"

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
