// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#pragma once

#include <daw/daw_move.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace daw::json {
	struct json_parser_exception final : public std::runtime_error {
		inline json_parser_exception( std::string message )
		  : std::runtime_error{ daw::move( message ) } {}

		~json_parser_exception( );

		json_parser_exception( json_parser_exception const & ) = default;

		json_parser_exception( json_parser_exception && ) noexcept = default;
		json_parser_exception &
		operator=( json_parser_exception && ) noexcept = default;

		inline json_parser_exception &operator=( json_parser_exception const &rhs ) {
			return *this = json_parser_exception{ rhs };
		}
	}; // struct json_parser_exception
} // namespace daw::json
