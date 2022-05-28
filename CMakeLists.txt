# Copyright (c) Darrell Wright
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/beached/parse_json
#

cmake_minimum_required( VERSION 3.18 ) 

project( "parse_json"
	VERSION "1.1.1" 
	DESCRIPTION "Parse JSON & json_link"
	HOMEPAGE_URL "https://github.com/beached/parse_json"
	LANGUAGES C CXX)

set( CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested.")
set( CMAKE_CXX_STANDARD_REQUIRED ON)
set( CMAKE_CXX_EXTENSIONS OFF)
set(Boost_NO_WARN_NEW_VERSIONS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package( Boost 1.62 COMPONENTS system filesystem regex iostreams unit_test_framework REQUIRED )
find_package( CURL REQUIRED )
find_package( OpenSSL REQUIRED )
find_package( daw-header-libraries REQUIRED )
find_package( daw-utf-range REQUIRED )
find_package( date REQUIRED )

include(GNUInstallDirs)
set(parse_json_INSTALL_CMAKEDIR
        "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake"
        CACHE STRING
        "The install directory to use for CMake package config files")

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

add_library ( ${PROJECT_NAME} )
add_library( daw::${PROJECT_NAME} ALIAS ${PROJECT_NAME} )

target_link_libraries( ${PROJECT_NAME} 
	daw::daw-header-libraries 
	daw::daw-utf-range 
	date::date 
	Boost::system 
	Boost::filesystem 
	Boost::regex 
	Boost::iostreams 
	Boost::unit_test_framework 
	CURL::libcurl 
	OpenSSL::SSL
	OpenSSL::Crypto
	)

add_library ( ${PROJECT_NAME}_v2 )
add_library( daw::${PROJECT_NAME}_v2 ALIAS ${PROJECT_NAME}_v2 )

target_link_libraries( ${PROJECT_NAME}_v2 
	daw::daw-header-libraries 
	daw::daw-utf-range 
	date::date 
	Boost::system 
	Boost::filesystem 
	Boost::regex 
	Boost::iostreams 
	Boost::unit_test_framework 
	CURL::libcurl 
	OpenSSL::SSL
	OpenSSL::Crypto
	daw::${PROJECT_NAME}
	)

target_compile_features( ${PROJECT_NAME} INTERFACE cxx_std_17)
target_include_directories( ${PROJECT_NAME}
	PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_features( ${PROJECT_NAME}_v2 INTERFACE cxx_std_17)
target_include_directories( ${PROJECT_NAME}_v2
	PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_sources( ${PROJECT_NAME} 
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_details.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_interface.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_link.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_parser.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_value_t.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_value_to_json.h
	PRIVATE
		src/daw_json.cpp
		src/daw_json_link.cpp
		src/daw_json_parser.cpp
		src/daw_json_value_t.cpp
		src/daw_value_to_json.cpp
	)

target_sources( ${PROJECT_NAME}_v2
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_parser_v2.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_parser_exception.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_link_definitions.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_parser_v2_state.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_parser_impl.h
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/daw/json/daw_json_link_binding.h
	PRIVATE
		src/daw_json_parser_v2_state.cpp
		src/daw_json_parser_exception.cpp
	)

install( TARGETS ${PROJECT_NAME}
	EXPORT ${PROJECT_NAME}_Targets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install( TARGETS ${PROJECT_NAME}_v2
	EXPORT ${PROJECT_NAME}_Targets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


include( CMakePackageConfigHelpers)

configure_package_config_file( 
	"${PROJECT_SOURCE_DIR}/cmake/daw_${PROJECT_NAME}Config.cmake.in"
	"${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
	INSTALL_DESTINATION ${parse_json_INSTALL_CMAKEDIR} )

write_basic_package_version_file( "${PROJECT_NAME}ConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion)

install( EXPORT ${PROJECT_NAME}_Targets
	FILE ${PROJECT_NAME}Targets.cmake
	NAMESPACE daw::
	DESTINATION ${parse_json_INSTALL_CMAKEDIR})

install( FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
	"${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
	DESTINATION ${parse_json_INSTALL_CMAKEDIR}$ )

install( DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

option(DAW_ENABLE_TESTING "Build unit tests" OFF)
if (DAW_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()


