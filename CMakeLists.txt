cmake_minimum_required( VERSION 2.8.12 )

project( parse_json_prj )

include( ExternalProject )

find_package( Boost 1.58.0 COMPONENTS system filesystem regex unit_test_framework iostreams REQUIRED )

enable_testing( )
add_definitions( -DBOOST_TEST_DYN_LINK ) 

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL 'MSVC' )
	add_compile_options( -D_WIN32_WINNT=0x0601 ) 
else( )
	find_package( Threads REQUIRED )
	if( THREADS_HAVE_PTHREAD_ARG )
		set( COMPILER_SPECifIC_LIBS, "-lpthread" )
	endif( )
	if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" )
		add_compile_options(-std=c++14 -Weverything -g -Wno-c++98-compat -Wfatal-errors -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion)
	elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
		add_compile_options(-std=c++14 -Wall -ggdb -Wno-deprecated-declarations)
	endif()
endif( )

externalproject_add( 
	header_libraries_prj
	GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/deps/headlibs"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

externalproject_add( 
	char_range_prj
	GIT_REPOSITORY "https://github.com/beached/char_range.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/deps/chrrng"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

externalproject_add(
	date_prj
	GIT_REPOSITORY "https://github.com/beached/date.git"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/date"
	GIT_TAG "master"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

#externalproject_add(
#	JSONTestSuite
#	GIT_REPOSITORY "https://github.com/nst/JSONTestSuite.git"
#	GIT_TAG "master"
#	SOURCE_DIR "${CMAKE_BINARY_DIR}/tests"
#	CONFIGURE_COMMAND ""
#	INSTALL_COMMAND ""
#	TEST_COMMAND ""
#	BUILD_COMMAND ""
#)

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )
link_directories( "${CMAKE_BINARY_DIR}/install/lib" )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

set( HEADER_FOLDER "include" )
set( SOURCE_FOLDER "src" )
set( TEST_FOLDER "tests" )

include_directories( ${HEADER_FOLDER} )

set( HEADER_FILES
	${HEADER_FOLDER}/daw_json.h
	${HEADER_FOLDER}/daw_json_details.h
	${HEADER_FOLDER}/daw_json_interface.h
	${HEADER_FOLDER}/daw_json_link.h
	${HEADER_FOLDER}/daw_json_parser.h
	${HEADER_FOLDER}/value_t.h
)

set( SOURCE_FILES
	${SOURCE_FOLDER}/daw_json.cpp
	${SOURCE_FOLDER}/daw_json_link.cpp
	${SOURCE_FOLDER}/daw_json_parser.cpp
	${SOURCE_FOLDER}/value_t.cpp
)

add_library ( parse_json STATIC ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( parse_json header_libraries_prj char_range_prj date_prj )
target_link_libraries( parse_json char_range tz ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )

install( TARGETS parse_json DESTINATION lib )
install( DIRECTORY ${HEADER_FOLDER}/ DESTINATION include/daw/json )

add_executable( parse_json_test_bin ${HEADER_FILES} ${TEST_FOLDER}/parse_json_test.cpp )
add_dependencies( parse_json_test_bin header_libraries_prj char_range_prj )
target_link_libraries( parse_json_test_bin parse_json char_range ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )
add_test( parse_json_test parse_json_test_bin )

add_executable( json_file_parser_test_bin ${HEADER_FILES} ${TEST_FOLDER}/json_file_parser_test.cpp )
add_dependencies( json_file_parser_test_bin header_libraries_prj char_range_prj ) #JSONTestSuite )
target_link_libraries( json_file_parser_test_bin parse_json char_range ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )
add_test( json_file_parser_test json_file_parser_test_bin )

add_executable( json_link_test_bin ${HEADER_FILES} ${TEST_FOLDER}/json_link_test.cpp )
add_dependencies( json_link_test_bin header_libraries_prj char_range_prj ) #JSONTestSuite )
target_link_libraries( json_link_test_bin parse_json char_range ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )
add_test( json_link_test json_link_test_bin )

