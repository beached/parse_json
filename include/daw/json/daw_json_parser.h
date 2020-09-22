// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#pragma once

#include "daw_json_value_t.h"

#include <daw/daw_string_view.h>

#include <string>

namespace daw::json {
	struct JsonParserException final {
		std::string message;

		JsonParserException( std::string msg ) noexcept;
		~JsonParserException( );

		JsonParserException( ) noexcept = default;
		JsonParserException( JsonParserException const & ) = default;
		JsonParserException( JsonParserException && ) noexcept = default;
		JsonParserException &operator=( JsonParserException && ) noexcept = default;
		JsonParserException &operator=( JsonParserException const &rhs ) = default;
	}; // struct JsonParserException

	using json_obj = json_value_t;

	json_obj parse_json( char const *Begin, char const *End );

	json_obj parse_json( daw::string_view json_text );
} // namespace daw::json
