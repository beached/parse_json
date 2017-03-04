// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <boost/utility/string_view.hpp>
#include <cstdlib>
#include <functional>
#include <string>

#include <daw/daw_exception.h>
#include <daw/not_null.h>

namespace daw {
	namespace json {
		struct json_binding_t {
			enum class expected_json_types: uint8_t { array, boolean, number, object, string };
			using serialize_function_t = std::function<std::string( void const * object )>; 
			using deserialize_function_t = std::function<void( void * object, boost::string_view )>; 

			expected_json_types expected_json_type;
			bool is_optional;	
			serialize_function_t serialize_function;
			deserialize_function_t deserialize_function;

			json_binding_t( ):
					is_optional{ false },
					serialize_function{ nullptr },
					deserialize_function{ nullptr } { }

			~json_binding_t( ) = default;
			json_binding_t( json_binding_t const & ) = default;
			json_binding_t( json_binding_t && ) = default;
			json_binding_t & operator=( json_binding_t const & ) = default;
			json_binding_t & operator=( json_binding_t && ) = default;
		};	// json_binding_t
		
	
		template<typename Class, size_t MemberOffset>
		json_binding_t json_bind_integral( ) {
			json_binding_t result;
		
			result.serialize_function = []( daw::not_null<void const *> obj_ptr, std::ostream & os ) {
				auto obj = static_cast<Class const *>(obj_ptr);
				auto const & member = ClassMember  (obj.&Class::ClassMember);
				using std::to_string;
				return to_string( member );
			};

			result.deserialize_function = []( daw::not_null<void *> object, boost::string_view str ) {
				auto obj = static_cast<Class *>(obj_ptr);
				auto & member = *(obj.*ClassMember);
		
				auto end = std::next( str.data( ), str.size( ) );
				member = strtol( str.data( ), &end, 0 ); 
			};
			return result;
		}

		template<typename Class, typename ClassMember>
		json_binding_t json_bind_integral_optional( ) {
			json_binding_t result;
			result.is_optional = true;
		
			result.serialize_function = []( daw::not_null<void const *> obj_ptr, std::ostream & os ) {
				auto obj = static_cast<Class const *>(obj_ptr);
				auto const member = obj.*ClassMember;
				if( member ) {
					using std::to_string;
					return to_string( member );
				}
				return "null";
			};

			result.deserialize_function = []( daw::not_null<void *> object, boost::string_view str ) {
				auto obj = static_cast<Class *>(obj_ptr);
				auto member = obj.*ClassMember;

				if( 'n' == str[0] ) {
					member = nullptr;
					return;
				}
				auto end = std::next( str.data( ), str.size( ) );
				*member = strtol( str.data( ), &end, 0 ); 
			};
			return result;
		}

	}	// namespace json
}    // namespace daw

#define JSON_EXPECT( type_name, json_name, member_name ) \
	daw::json::json_bind_##type_name##<
