// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#pragma once

#include <daw/daw_exception.h>
#include <daw/daw_string_view.h>
#include <daw/not_null.h>

#include <cstdlib>
#include <functional>
#include <string>

namespace daw::json {
	namespace types {
		enum class expected_json_types : uint8_t {
			integer,
			real,
			string,
			boolean,
			integral_array,
			real_array,
			string_array,
			boolean_array,
			object,
			object_array
		};
	}

	template<typename Derived>
	struct json_binding_t {
		using to_json_function_t = std::function<std::string( Derived const & )>;
		using from_json_function_t =
		  std::function<void( Derived &, daw::string_view )>;

		types::expected_json_types expected_json_type;
		bool is_optional;
		from_json_function_t from_json_function;
		to_json_function_t to_json_function;

		json_binding_t( types::expected_json_types ExpectedJsonType,
		                bool IsOptional, from_json_function_t FromJson,
		                to_json_function_t ToJson )
		  : expected_json_type{ ExpectedJsonType }
		  , is_optional{ IsOptional }
		  , from_json_function{ FromJson }
		  , to_json_function{ ToJson } {}

		json_binding_t( ) = delete;
		~json_binding_t( ) = default;
		json_binding_t( json_binding_t const & ) = default;
		json_binding_t( json_binding_t && ) noexcept = default;
		json_binding_t &operator=( json_binding_t && ) noexcept = default;

		json_binding_t &operator=( json_binding_t const &rhs ) {
			return *this = json_binding_t{ rhs };
		}

	}; // json_binding_t
} // namespace daw::json
