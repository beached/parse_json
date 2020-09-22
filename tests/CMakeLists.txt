# Copyright (c) Darrell Wright
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/beached/parse_json
#

add_executable( parse_json_test src/parse_json_test.cpp )
target_link_libraries( parse_json_test daw::parse_json )
add_test( NAME parse_json_test_test COMMAND parse_json_test )

add_executable( json_file_parser_test src/json_file_parser_test.cpp )
target_link_libraries( json_file_parser_test daw::parse_json )
add_test( NAME json_file_parser_test COMMAND json_file_parser_test )

add_executable( json_link_test src/json_link_test.cpp )
target_link_libraries( json_link_test daw::parse_json )
add_test( NAME json_link_test_test COMMAND json_link_test )

add_executable( parse_json_test_v2 src/parse_json_test_v2.cpp )
target_link_libraries( parse_json_test_v2 daw::parse_json_v2 )
add_test( NAME parse_json_test_v2_test COMMAND parse_json_test_v2 )

add_executable( daw_json_link_v2_test ${HEADER_FILES_v2} src/daw_json_link_v2_test.cpp )
target_link_libraries( daw_json_link_v2_test daw::parse_json_v2 )
add_test( NAME daw_json_link_v2_test_test COMMAND daw_json_link_v2_test )

