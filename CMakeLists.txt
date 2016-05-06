cmake_minimum_required( VERSION 2.8.0 )

project( parse_json )

include( ExternalProject )

find_package( Boost 1.59.0 COMPONENTS system filesystem regex unit_test_framework REQUIRED )
set( Boost_Libs ${BOOST_Libraries} -lboost_system -lboost_regex -lboost_filesystem -lboost_program_options -lboost_unit_test_framework )

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC" )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601" )
	set( Boost_Libs )
elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
	find_package( Threads REQUIRED )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++ -g -O3 -Weverything -Wno-c++98-compat  -Wfatal-errors -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion -Wno-global-constructors" )
elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
	find_package( Threads REQUIRED )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -O3 -Wall -Wno-deprecated-declarations" )
endif( )

ExternalProject_Add( 
	header_libraries_prj
	GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/header_libraries"
	GIT_TAG "master"
	PATCH_COMMAND ""
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	TEST_COMMAND ""
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
 )

ExternalProject_Add( 
	char_range_prj
	GIT_REPOSITORY "https://github.com/beached/char_range.git"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/char_range"
	GIT_TAG "master"
	PATCH_COMMAND ""
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	TEST_COMMAND ""
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
 )


include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )
link_directories( "${CMAKE_BINARY_DIR}/install/lib" )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

set( HEADER_FOLDER "include" )
include_directories( ${HEADER_FOLDER} )

set( HEADER_FILES
	${HEADER_FOLDER}/daw_json.h
	${HEADER_FOLDER}/daw_json_details.h
	${HEADER_FOLDER}/daw_json_interface.h
	${HEADER_FOLDER}/daw_json_link.h
	${HEADER_FOLDER}/daw_json_parser.h
)

set( SOURCE_FILES
	daw_json.cpp
	daw_json_link.cpp
	daw_json_parser.cpp
)

add_library ( parse_json SHARED ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( parse_json header_libraries_prj char_range_prj )
target_link_libraries( parse_json char_range )

add_library ( parse_json_static STATIC ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( parse_json_static header_libraries_prj char_range_prj )
target_link_libraries( parse_json char_range_static )

install( TARGETS parse_json DESTINATION lib )
install( TARGETS parse_json_static DESTINATION lib )
install( DIRECTORY ${HEADER_FOLDER}/ DESTINATION include/daw/parse_json )


