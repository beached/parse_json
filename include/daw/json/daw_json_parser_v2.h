// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#pragma once

#include "daw_json_parser_exception.h"
#include "daw_json_parser_impl.h"

namespace daw::json {
	template<typename InputIteratorFirst, typename InputIteratorLast,
	         typename State>
	void json_parser( InputIteratorFirst first, InputIteratorLast last,
	                  State &state ) {
		first = impl::skip_whitespace( first, last );
		while( first != last ) {
			switch( *first ) {
			case '{':
				first = impl::parse_object( first, last, state );
				break;
			case '[':
				first = impl::parse_array( first, last, state );
				break;
			default:
				throw json_parser_exception(
				  "Expected an array '[' or object '{' at start of json file" );
			}
			first = impl::skip_whitespace( first, last );
		}
	}
} // namespace daw::json
