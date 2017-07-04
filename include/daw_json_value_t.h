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
#include <boost/utility/string_view.hpp>
#include <boost/variant.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <daw/char_range/daw_char_range.h>
#include <daw/daw_common_mixins.h>
#include <typeindex>

namespace daw {
	namespace json {
		struct json_value_t;

		using json_string_value = daw::utf_string;

		using json_object_value_item = std::pair<json_string_value, json_value_t>;

		json_object_value_item make_object_value_item( json_string_value first, json_value_t second );

		struct json_object_value
		    : public daw::mixins::VectorLikeProxy<json_object_value, std::vector<json_object_value_item>> {
			std::vector<json_object_value_item> members_v;

			json_object_value( ) = default;
			~json_object_value( );

			json_object_value( json_object_value const & ) = default;

			json_object_value( json_object_value && ) = default;

			json_object_value &operator=( json_object_value ov ) {
				members_v = std::move( ov.members_v );
				return *this;
			}

			inline std::vector<json_object_value_item> &container( ) {
				return members_v;
			}

			inline std::vector<json_object_value_item> const &container( ) const {
				return members_v;
			}

			boost::optional<json_value_t> operator( )( boost::string_view key ) const;

			using key_type = std::string;
			using mapped_type = json_value_t;

			iterator find( boost::string_view key );

			const_iterator find( boost::string_view key ) const;

			bool has_member( boost::string_view key ) const;

			mapped_type &operator[]( boost::string_view key );

			mapped_type const &operator[]( boost::string_view key ) const;

			inline void shrink_to_fit( ) {
				members_v.shrink_to_fit( );
			}
		}; // struct json_object_value

		std::string to_string( json_object_value const &obj );

		using json_array_value = std::vector<json_value_t>;

		struct json_null_t final {};

		struct json_value_t {
			using null_t = json_null_t;
			using integer_t = intmax_t;
			using real_t = double;
			using string_t = json_string_value;
			using boolean_t = bool;
			using array_t = json_array_value;
			using object_t = json_object_value;

		  private:
			boost::variant<null_t, integer_t, real_t, string_t, boolean_t, array_t, object_t> m_value;

		  public:
			json_value_t( ) noexcept;

			explicit json_value_t( integer_t value ) noexcept;

			explicit json_value_t( real_t value ) noexcept;

			json_value_t( boost::string_view value );

			explicit json_value_t( string_t value ) noexcept;

			explicit json_value_t( boolean_t value ) noexcept;

			explicit json_value_t( null_t value ) noexcept;

			explicit json_value_t( array_t value ) noexcept;

			explicit json_value_t( object_t value ) noexcept;

			~json_value_t( );

			json_value_t( json_value_t const &other );

			json_value_t( json_value_t &&other ) noexcept;
			json_value_t &operator=( json_value_t &&other ) noexcept;

			json_value_t &operator=( json_value_t const &rhs );

			json_value_t &operator=( json_value_t::integer_t rhs ) noexcept;

			json_value_t &operator=( json_value_t::real_t rhs ) noexcept;

			json_value_t &operator=( boost::string_view rhs ) noexcept;

			json_value_t &operator=( json_value_t::string_t rhs ) noexcept;

			json_value_t &operator=( json_value_t::boolean_t rhs ) noexcept;

			json_value_t &operator=( json_value_t::null_t rhs ) noexcept;

			json_value_t &operator=( json_value_t::array_t rhs ) noexcept;

			json_value_t &operator=( json_value_t::object_t rhs ) noexcept;

			integer_t get_integer( ) const;

			real_t get_real( ) const;

			std::string get_string( ) const;

			string_t get_string_value( ) const;

			boolean_t const &get_boolean( ) const;

			boolean_t &get_boolean( );

			object_t const &get_object( ) const;

			object_t &get_object( );

			array_t const &get_array( ) const;

			array_t &get_array( );

			std::type_index type( ) const noexcept;

			void cleanup( );

			bool is_integer( ) const noexcept;
			bool is_real( ) const noexcept;
			bool is_numeric( ) const noexcept;
			bool is_string( ) const noexcept;
			bool is_boolean( ) const noexcept;
			bool is_null( ) const noexcept;
			bool is_array( ) const noexcept;
			bool is_object( ) const noexcept;

			int compare( json_value_t const &rhs ) const;

			template<typename Visitor>
			decltype( auto ) apply_visitor( Visitor &&visitor ) {
				return boost::apply_visitor( std::forward<Visitor>( visitor ), m_value );
			}

			template<typename Visitor>
			decltype( auto ) apply_visitor( Visitor &&visitor ) const {
				return boost::apply_visitor( std::forward<Visitor>( visitor ), m_value );
			}
			std::string to_string( ) const;
		}; // json_value_t

		std::string to_string( std::type_index const &ti );
		using value_opt_t = boost::optional<json_value_t>;

		std::string to_string( json_value_t const &value );

		std::string to_string( std::shared_ptr<json_value_t> const &value );

		std::string to_string( std::ostream &os, std::shared_ptr<json_value_t> const &value );

		std::ostream &operator<<( std::ostream &os, json_value_t const &value );

		std::ostream &operator<<( std::ostream &os, std::shared_ptr<json_value_t> const &value );

		bool operator==( json_value_t const &lhs, json_value_t const &rhs );
		bool operator!=( json_value_t const &lhs, json_value_t const &rhs );
		bool operator<( json_value_t const &lhs, json_value_t const &rhs );
		bool operator>( json_value_t const &lhs, json_value_t const &rhs );
		bool operator<=( json_value_t const &lhs, json_value_t const &rhs );
		bool operator>=( json_value_t const &lhs, json_value_t const &rhs );

		template<typename T>
		T get( json_value_t const & );

		template<>
		json_value_t::integer_t get<json_value_t::integer_t>( daw::json::json_value_t const &val );

		template<>
		json_value_t::real_t get<json_value_t::real_t>( daw::json::json_value_t const &val );

		template<>
		json_value_t::string_t get<json_value_t::string_t>( daw::json::json_value_t const &val );

		template<>
		std::string get<std::string>( daw::json::json_value_t const &val );

		template<>
		json_value_t::boolean_t get<json_value_t::boolean_t>( daw::json::json_value_t const &val );

		template<>
		json_value_t::object_t get<json_value_t::object_t>( daw::json::json_value_t const &val );

		template<>
		json_value_t::array_t get<json_value_t::array_t>( daw::json::json_value_t const &val );

		template<>
		json_value_t::null_t get<json_value_t::null_t>( daw::json::json_value_t const & );
	} // namespace json
} // namespace daw

namespace std {
	std::string to_string( daw::json::json_array_value const &arry );
}
