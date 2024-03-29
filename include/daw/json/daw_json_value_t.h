// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#pragma once

#include <daw/daw_common_mixins.h>
#include <daw/daw_move.h>
#include <daw/daw_string_view.h>
#include <daw/daw_visit.h>
#include <daw/utf_range/daw_utf_range.h>
#include <daw/utf_range/daw_utf_string.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <typeindex>
#include <utility>
#include <variant>
#include <vector>

namespace daw::json {
	struct json_value_t;

	using json_string_value = daw::utf_string;

	using json_object_value_item = std::pair<json_string_value, json_value_t>;

	json_object_value_item make_object_value_item( json_string_value first,
	                                               json_value_t second );

	struct json_object_value
	  : public daw::mixins::VectorLikeProxy<json_object_value,
	                                        std::vector<json_object_value_item>> {
		std::vector<json_object_value_item> members_v;

		json_object_value( ) = default;
		~json_object_value( );

		json_object_value( json_object_value const & ) = default;

		json_object_value( json_object_value && ) = default;

		json_object_value &operator=( json_object_value ov ) {
			members_v = DAW_MOVE( ov.members_v );
			return *this;
		}

		inline std::vector<json_object_value_item> &container( ) {
			return members_v;
		}

		inline std::vector<json_object_value_item> const &container( ) const {
			return members_v;
		}

		std::optional<json_value_t> operator( )( daw::string_view key ) const;

		using key_type = std::string;
		using mapped_type = json_value_t;

		iterator find( daw::string_view key );

		const_iterator find( daw::string_view key ) const;

		bool has_member( daw::string_view key ) const;

		mapped_type &operator[]( daw::string_view key );

		mapped_type const &operator[]( daw::string_view key ) const;

		inline void shrink_to_fit( ) {
			members_v.shrink_to_fit( );
		}
	}; // struct json_object_value

	std::string to_string( json_object_value const &obj );

	using json_array_value = std::vector<json_value_t>;

	struct json_null_t final {
		constexpr json_null_t( ) noexcept {}
	};

	struct json_value_t {
		using null_t = json_null_t;
		using integer_t = intmax_t;
		using real_t = double;
		using string_t = json_string_value;
		using boolean_t = bool;
		using array_t = json_array_value;
		using object_t = json_object_value;

	private:
		std::variant<null_t, integer_t, real_t, string_t, boolean_t, array_t,
		             object_t>
		  m_value;

	public:
		template<typename T>
		static constexpr size_t index_of( ) {
			auto result =
			  daw::traits::pack_index_of<T, null_t, integer_t, real_t, string_t,
			                             boolean_t, array_t, object_t>( );

			if( result < 0 ) {
				throw std::out_of_range{ "T" };
			}
			return result;
		}

		json_value_t( ) noexcept;

		explicit json_value_t( integer_t value ) noexcept;

		explicit json_value_t( real_t value ) noexcept;

		json_value_t( daw::string_view value );

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

		json_value_t &operator=( daw::string_view rhs ) noexcept;

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

		size_t type( ) const noexcept;

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
			return daw::visit_nt( m_value, DAW_FWD( visitor ) );
		}
		template<typename Visitor>
		decltype( auto ) apply_visitor( Visitor &&visitor ) const {
			return daw::visit_nt( m_value, DAW_FWD( visitor ) );
		}

		std::string to_string( ) const;
	}; // json_value_t

	std::string to_string( size_t ti ) noexcept;
	using value_opt_t = std::optional<json_value_t>;

	std::string to_string( json_value_t const &value );

	std::string to_string( std::shared_ptr<json_value_t> const &value );

	std::string to_string( std::ostream &os,
	                       std::shared_ptr<json_value_t> const &value );

	std::ostream &operator<<( std::ostream &os, json_value_t const &value );

	std::ostream &operator<<( std::ostream &os,
	                          std::shared_ptr<json_value_t> const &value );

	bool operator==( json_value_t const &lhs, json_value_t const &rhs );
	bool operator!=( json_value_t const &lhs, json_value_t const &rhs );
	bool operator<( json_value_t const &lhs, json_value_t const &rhs );
	bool operator>( json_value_t const &lhs, json_value_t const &rhs );
	bool operator<=( json_value_t const &lhs, json_value_t const &rhs );
	bool operator>=( json_value_t const &lhs, json_value_t const &rhs );

	template<typename T>
	T get( json_value_t const & );

	template<>
	json_value_t::integer_t
	get<json_value_t::integer_t>( daw::json::json_value_t const &val );

	template<>
	json_value_t::real_t
	get<json_value_t::real_t>( daw::json::json_value_t const &val );

	template<>
	json_value_t::string_t
	get<json_value_t::string_t>( daw::json::json_value_t const &val );

	template<>
	std::string get<std::string>( daw::json::json_value_t const &val );

	template<>
	json_value_t::boolean_t
	get<json_value_t::boolean_t>( daw::json::json_value_t const &val );

	template<>
	json_value_t::object_t
	get<json_value_t::object_t>( daw::json::json_value_t const &val );

	template<>
	json_value_t::array_t
	get<json_value_t::array_t>( daw::json::json_value_t const &val );

	template<>
	json_value_t::null_t
	get<json_value_t::null_t>( daw::json::json_value_t const & );
} // namespace daw::json

namespace std {
	std::string to_string( daw::json::json_array_value const &arry );
}
