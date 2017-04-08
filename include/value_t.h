// The MIT License (MIT)
//
// Copyright (c) 2014-2017 Darrell Wright
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

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <daw/daw_common_mixins.h>
#include <daw/char_range/daw_char_range.h>
#include <daw/daw_variant.h>

namespace daw {
	namespace json {
		namespace impl {
			struct value_t;

			struct null_value final { };

			using string_value = ::daw::utf_string;

			using object_value_item = std::pair<string_value, value_t>;

			object_value_item make_object_value_item( string_value first, value_t second );

			struct object_value: public daw::mixins::VectorLikeProxy<object_value, std::vector<object_value_item>> {
				std::vector<object_value_item> members_v;

				object_value( ) = default;
				~object_value( );

				object_value( object_value const & ) = default;

				object_value( object_value && ) = default;

				object_value & operator=( object_value ov ) {
					members_v = std::move( ov.members_v );
					return *this;
				}

				inline std::vector<object_value_item> & container( ) {
					return members_v;
				}

				inline std::vector<object_value_item> const & container( ) const {
					return members_v;
				}

				boost::optional<value_t> operator( )( boost::string_view key ) const;

				using key_type = std::string;
				using mapped_type = value_t;

				iterator find( boost::string_view key );

				const_iterator find( boost::string_view key ) const;

				bool has_member( boost::string_view key ) const;

				mapped_type & operator[]( boost::string_view key );

				mapped_type const & operator[]( boost::string_view key ) const;

				inline void shrink_to_fit( ) {
					members_v.shrink_to_fit( );
				}
			};    // struct object_value

			std::string to_string( object_value const & obj );

			using array_value = std::vector<value_t>;

			struct value_t {
				using integer_t = intmax_t;
				using real_t = double;
				using string_t = string_value;
				using boolean_t = bool;
				using array_t = array_value;
				using object_t = object_value;

				enum class value_types {
					integer,
					real,
					string,
					boolean,
					null,
					array,
					object
				};
			private:
				boost::variant<integer_t, real_t, string_t, boolean_t, array_t, object_t> m_value;
				value_types m_value_type;
			public:
				value_t( );

				explicit value_t( integer_t const & value );

				explicit value_t( real_t const & value );

				value_t( boost::string_view value );

				explicit value_t( string_t value );

				explicit value_t( boolean_t value );

				explicit value_t( std::nullptr_t value );

				explicit value_t( array_t value );

				explicit value_t( object_t value );

				~value_t( );

				value_t( value_t const & other );

				value_t( value_t && other ) noexcept;
				value_t & operator=( value_t && other ) noexcept;

				value_t & operator=( value_t const & rhs );

				value_t & operator=( value_t::integer_t rhs );

				value_t & operator=( value_t::real_t rhs );

				value_t & operator=( boost::string_view rhs );

				value_t & operator=( value_t::string_t rhs );

				value_t & operator=( value_t::boolean_t rhs );

				value_t & operator=( std::nullptr_t rhs );

				value_t & operator=( value_t::array_t rhs );

				value_t & operator=( value_t::object_t rhs );

				integer_t get_integer( ) const;

				real_t get_real( ) const;

				std::string get_string( ) const;

				string_t get_string_value( ) const;

				boolean_t const & get_boolean( ) const;

				boolean_t & get_boolean( );

				object_t const & get_object( ) const;

				object_t & get_object( );

				array_t const & get_array( ) const;

				array_t & get_array( );

				value_types type( ) const;

				void cleanup( );

				bool is_integer( ) const;

				bool is_real( ) const;

				bool is_numeric( ) const;

				bool is_string( ) const;

				bool is_boolean( ) const;

				bool is_null( ) const;

				bool is_array( ) const;

				bool is_object( ) const;
				
				int compare( value_t const & rhs ) const;
			};    // value_t

			
			std::string to_string( value_t::value_types type ) noexcept;
			using value_opt_t = boost::optional<value_t>;

			std::string to_string( value_t const & value );

			std::string to_string( std::shared_ptr<value_t> const & value );

			std::string to_string( std::ostream & os, std::shared_ptr<value_t> const & value );

			std::ostream & operator<<( std::ostream & os, value_t const & value );

			std::ostream & operator<<( std::ostream & os, std::shared_ptr<value_t> const & value );

			bool operator==( value_t const & lhs, value_t const & rhs );
			bool operator!=( value_t const & lhs, value_t const & rhs );
			bool operator<( value_t const & lhs, value_t const & rhs );
			bool operator>( value_t const & lhs, value_t const & rhs );
			bool operator<=( value_t const & lhs, value_t const & rhs );
			bool operator>=( value_t const & lhs, value_t const & rhs );
		}    // namespace impl
	}    // namespace json
}    // namespace daw

namespace std {
	std::string to_string( daw::json::impl::array_value const & arry );
}
