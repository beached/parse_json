cmake_minimum_required(VERSION 3.10)

project( parse_json_prj )
#set( CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested.")

find_package( Boost 1.60.0 COMPONENTS system filesystem regex iostreams unit_test_framework REQUIRED )
find_package( CURL REQUIRED )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

add_definitions( -DBOOST_TEST_DYN_LINK ) 

enable_testing()

include("${CMAKE_SOURCE_DIR}/dependent_projects/CMakeListsCompiler.txt")

include_directories("./include")
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(DEP_ROOT_DIR_BASE "${CMAKE_SOURCE_DIR}/dependent_projects/build")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(DEP_ROOT_DIR "${DEP_ROOT_DIR_BASE}/debug")
    set(GLEAN_CACHE "${CMAKE_SOURCE_DIR}/.glean/debug")
    set(GLEAN_CACHE2 "${CMAKE_SOURCE_DIR}/.glean/release")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(DEP_ROOT_DIR "${DEP_ROOT_DIR_BASE}/release")
    set(GLEAN_CACHE "${CMAKE_SOURCE_DIR}/.glean/release")
    set(GLEAN_CACHE2 "${CMAKE_SOURCE_DIR}/.glean/debug")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(DEP_ROOT_DIR "${DEP_ROOT_DIR_BASE}/release")
    set(GLEAN_CACHE "${CMAKE_SOURCE_DIR}/.glean/release")
    set(GLEAN_CACHE2 "${CMAKE_SOURCE_DIR}/.glean/debug")
endif ()

message("Building for: ${CMAKE_BUILD_TYPE}")
message("Checking for glean cache: '${GLEAN_CACHE}'")

option(GLEAN_INSTALL_ROOT "")
if (EXISTS "${GLEAN_INSTALL_ROOT}")
    message("using glean dependencies")
    message("installing to ${GLEAN_INSTALL_ROOT}")

    add_custom_target(dependency_stub)
    include_directories(SYSTEM "${GLEAN_INSTALL_ROOT}/include")
    link_directories("${GLEAN_INSTALL_ROOT}/lib")
elseif (EXISTS "${GLEAN_CACHE}")
    message("using glean dependencies")
    add_custom_target(dependency_stub)
    include_directories(SYSTEM "${GLEAN_CACHE}/include")
    link_directories("${GLEAN_CACHE}/lib")
elseif (EXISTS "${GLEAN_CACHE2}")
    message("looks like glean is in use but missing ${GLEAN_CACHE}")
elseif (EXISTS "${CMAKE_SOURCE_DIR}/glean.cmake")
    message("glean.cmake detected")
    include("${CMAKE_SOURCE_DIR}/glean.cmake")
    add_library(dependency_stub STATIC "${CMAKE_SOURCE_DIR}/dependent_projects/stub.cpp")
    add_dependencies(dependency_stub ${DEP_PROJECT_DEPS})
    include_directories(SYSTEM "${CMAKE_BINARY_DIR}/install/include")
    link_directories("${CMAKE_BINARY_DIR}/install/lib")
elseif (EXISTS "${DEP_ROOT_DIR}")
    message("${DEP_ROOT_DIR}")
    message("using dependency folder")
    add_custom_target(dependency_stub)
    include_directories(SYSTEM "${DEP_ROOT_DIR}/install/include")
    link_directories("${DEP_ROOT_DIR}/install/lib")
else ()
    message("downloading and building deps")
    include("${CMAKE_SOURCE_DIR}/dependent_projects/dependencies.txt")
    add_library(dependency_stub STATIC "${CMAKE_SOURCE_DIR}/dependent_projects/stub.cpp")
    add_dependencies(dependency_stub ${DEP_PROJECT_DEPS})
endif ()

set(HEADER_FOLDER "include")
set(TEST_FOLDER "tests")
set(SOURCE_FOLDER "src")

include_directories(${HEADER_FOLDER})
set( HEADER_FILES
	${HEADER_FOLDER}/daw_json.h
	${HEADER_FOLDER}/daw_json_details.h
	${HEADER_FOLDER}/daw_json_interface.h
	${HEADER_FOLDER}/daw_json_link.h
	${HEADER_FOLDER}/daw_json_parser.h
	${HEADER_FOLDER}/daw_json_value_t.h
	${HEADER_FOLDER}/daw_value_to_json.h
)

set( SOURCE_FILES
	${SOURCE_FOLDER}/daw_json.cpp
	${SOURCE_FOLDER}/daw_json_link.cpp
	${SOURCE_FOLDER}/daw_json_parser.cpp
	${SOURCE_FOLDER}/daw_json_value_t.cpp
	${SOURCE_FOLDER}/daw_value_to_json.cpp
)

set( HEADER_FILES_v2
	${HEADER_FOLDER}/daw_json_link_binding.h
	${HEADER_FOLDER}/daw_json_parser_v2.h
	${HEADER_FOLDER}/daw_json_parser_exception.h
	${HEADER_FOLDER}/daw_json_parser_impl.h
)

set( SOURCE_FILES_v2
	${SOURCE_FOLDER}/daw_json_parser_v2.cpp
	${SOURCE_FOLDER}/daw_json_parser_v2_state.cpp
	${SOURCE_FOLDER}/daw_json_parser_exception.cpp
)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_custom_target(full)

add_library ( parse_json ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( parse_json dependency_stub )
target_link_libraries( parse_json utf_range utf_string ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES} )
add_dependencies(check parse_json)
add_dependencies(full parse_json)

add_library ( parse_json_v2 ${HEADER_FILES_v2} ${SOURCE_FILES_v2} )
add_dependencies( parse_json_v2 dependency_stub )
target_link_libraries( parse_json_v2 utf_range utf_string ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES} )
add_dependencies(check parse_json_v2)
add_dependencies(full parse_json_v2)

install( TARGETS parse_json DESTINATION lib )
install( TARGETS parse_json_v2 DESTINATION lib )
install( DIRECTORY ${HEADER_FOLDER}/ DESTINATION include/daw/json )

add_executable( parse_json_test_bin EXCLUDE_FROM_ALL ${HEADER_FILES} ${TEST_FOLDER}/parse_json_test.cpp )
add_dependencies( parse_json_test_bin dependency_stub )
target_link_libraries( parse_json_test_bin utf_string utf_range parse_json ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )
add_test( NAME parse_json_test COMMAND parse_json_test_bin )
add_dependencies(check parse_json_test_bin)
add_dependencies(full parse_json_test_bin)

add_executable( parse_json_test_v2_bin EXCLUDE_FROM_ALL ${HEADER_FOLDER}/daw_json_parser_v2.h ${HEADER_FILES_v2} ${TEST_FOLDER}/parse_json_test_v2.cpp )
add_dependencies( parse_json_test_v2_bin dependency_stub )
target_link_libraries( parse_json_test_v2_bin parse_json_v2 ${Boost_LIBRARIES} )
add_test( NAME parse_json_test_v2 COMMAND parse_json_test_v2_bin )
add_dependencies(check parse_json_test_v2_bin)
add_dependencies(full parse_json_test_v2_bin)

add_executable( json_file_parser_test_bin EXCLUDE_FROM_ALL ${HEADER_FILES} ${TEST_FOLDER}/json_file_parser_test.cpp )
add_dependencies( json_file_parser_test_bin dependency_stub )
target_link_libraries( json_file_parser_test_bin utf_string utf_range parse_json ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )
add_test( NAME json_file_parser_test COMMAND json_file_parser_test_bin )
add_dependencies(check json_file_parser_test_bin)
add_dependencies(full json_file_parser_test_bin)

add_executable( daw_json_link_v2_test_bin EXCLUDE_FROM_ALL ${HEADER_FILES_v2} ${TEST_FOLDER}/daw_json_link_v2_test.cpp )
add_dependencies( daw_json_link_v2_test_bin dependency_stub )
target_link_libraries( daw_json_link_v2_test_bin utf_range utf_string parse_json ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )
add_test( NAME daw_json_link_v2_test COMMAND daw_json_link_v2_test_bin )
add_dependencies(check daw_json_link_v2_test_bin)
add_dependencies(full daw_json_link_v2_test_bin)

add_executable( json_link_test_bin EXCLUDE_FROM_ALL ${HEADER_FILES} ${TEST_FOLDER}/json_link_test.cpp )
add_dependencies( json_link_test_bin dependency_stub )
target_link_libraries( json_link_test_bin utf_range utf_string parse_json ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} )
add_test( NAME json_link_test COMMAND json_link_test_bin )
add_dependencies(check json_link_test_bin)
add_dependencies(full json_link_test_bin)

