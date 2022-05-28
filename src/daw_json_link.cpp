// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include <daw/json/daw_json_link.h>

#include <daw/daw_string_view.h>

#include <string>

namespace daw::json::impl {
	std::string to_json_integer( json_value_t::integer_t i ) {
		using std::to_string;
		return to_string( i );
	}

	std::string to_json_real( json_value_t::real_t d ) {
		using std::to_string;
		return to_string( d );
	}

	std::string to_json_string( daw::string_view s ) {
		std::string result;
		result = "\"" + static_cast<std::string>( s ) + "\"";
		return result;
	}

	std::string to_json_boolean( bool b ) {
		return b ? "true" : "false";
	}

	std::string to_json_null( ) {
		using namespace std::string_literals;
		return "null"s;
	}
} // namespace daw::json::impl
