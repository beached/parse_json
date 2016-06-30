cmake_minimum_required( VERSION 2.8.12 )

project( parse_json_prj )

include( ExternalProject )

find_package( Boost 1.59.0 COMPONENTS system filesystem regex unit_test_framework REQUIRED )

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL 'MSVC' )
	add_compile_options( -D_WIN32_WINNT=0x0601 ) 
else( )
	find_package( Threads REQUIRED )
	if( THREADS_HAVE_PTHREAD_ARG )
		set( COMPILER_SPECifIC_LIBS, "-lpthread" )
	endif( )

	if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
		add_compile_options(-std=c++14 -stdlib=libc++ -Weverything -Wno-c++98-compat -Wfatal-errors -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion)		
	elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
		add_compile_options(-std=c++14 -Wall -Wno-deprecated-declarations)
	endif( )
endif( )

externalproject_add( 
	header_libraries_prj
	GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/deps/headlibs"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/inst"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/inst
	PATCH_COMMAND ""
	TEST_COMMAND ""
 )

externalproject_add( 
	char_range_prj
	GIT_REPOSITORY "https://github.com/beached/char_range.git"
	GIT_TAG "master"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/deps/chrrng"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/inst"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/inst
	PATCH_COMMAND ""
	TEST_COMMAND ""
 )

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/inst/include" )
link_directories( "${CMAKE_BINARY_DIR}/inst/lib" )

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

add_library ( parse_json STATIC ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( parse_json header_libraries_prj char_range_prj )
target_link_libraries( parse_json char_range ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )

install( TARGETS parse_json DESTINATION lib )
install( DIRECTORY ${HEADER_FOLDER}/ DESTINATION include/daw/parse_json )

