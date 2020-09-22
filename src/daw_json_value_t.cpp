// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/parse_json
//

#include <daw/json/daw_json_value_t.h>

#include <daw/daw_exception.h>
#include <daw/daw_move.h>
#include <daw/daw_operators.h>
#include <daw/daw_range.h>
#include <daw/daw_string_view.h>
#include <daw/utf_range/daw_utf_range.h>

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace daw::json {
	json_object_value_item make_object_value_item( json_string_value first,
	                                               json_value_t second ) {
		return std::make_pair<json_string_value, json_value_t>(
		  daw::move( first ), daw::move( second ) );
	}
	std::string to_string( json_value_t const &v ) {
		return v.to_string( );
	}

	std::string to_string( size_t ti ) noexcept {
		using namespace std::string_literals;
		switch( ti ) {
		case json_value_t::index_of<json_value_t::integer_t>( ):
			return "integer";
		case json_value_t::index_of<json_value_t::real_t>( ):
			return "real";
		case json_value_t::index_of<json_value_t::string_t>( ):
			return "string";
		case json_value_t::index_of<json_value_t::boolean_t>( ):
			return "boolean";
		case json_value_t::index_of<json_value_t::null_t>( ):
			return "null";
		case json_value_t::index_of<json_value_t::array_t>( ):
			return "array";
		case json_value_t::index_of<json_value_t::object_t>( ):
			return "object";
		default:
			// Should never happen
			std::terminate( );
		}
	}

	json_value_t::json_value_t( ) noexcept
	  : m_value{ null_t{} } {}

	json_value_t::json_value_t( json_value_t::integer_t value ) noexcept
	  : m_value{ daw::move( value ) } {}

	json_value_t::json_value_t( json_value_t::real_t value ) noexcept
	  : m_value{ daw::move( value ) } {}

	json_value_t::json_value_t( daw::string_view value )
	  : m_value{ string_t{ value } } {}

	json_value_t::json_value_t( json_value_t::string_t value ) noexcept
	  : m_value{ daw::move( value ) } {}

	json_value_t::json_value_t( json_value_t::boolean_t value ) noexcept
	  : m_value{ daw::move( value ) } {}

	json_value_t::json_value_t( json_value_t::null_t ) noexcept
	  : m_value{ null_t{} } {}

	json_value_t::json_value_t( json_value_t::array_t value ) noexcept
	  : m_value{ json_value_t::array_t{ daw::move( value ) } } {}

	json_value_t::json_value_t( json_value_t::object_t value ) noexcept
	  : m_value{ json_value_t::object_t{ daw::move( value ) } } {}

	json_value_t::json_value_t( json_value_t const &other )
	  : m_value{ other.m_value } {}

	json_value_t::json_value_t( json_value_t &&other ) noexcept
	  : m_value{ daw::move( other.m_value ) } {}

	json_value_t &json_value_t::operator=( json_value_t &&rhs ) noexcept {
		m_value = daw::move( rhs.m_value );
		return *this;
	}

	json_value_t &json_value_t::operator=( json_value_t const &rhs ) {
		return *this = json_value_t{ rhs };
	}

	json_value_t &
	json_value_t::operator=( json_value_t::integer_t rhs ) noexcept {
		m_value = daw::move( rhs );
		return *this;
	}

	json_value_t &json_value_t::operator=( json_value_t::real_t rhs ) noexcept {
		m_value = daw::move( rhs );
		return *this;
	}

	json_value_t &json_value_t::operator=( daw::string_view rhs ) noexcept {
		m_value = json_string_value{ rhs };
		return *this;
	}

	json_value_t &json_value_t::operator=( json_value_t::string_t rhs ) noexcept {
		m_value = daw::move( rhs );
		return *this;
	}

	json_value_t &
	json_value_t::operator=( json_value_t::boolean_t rhs ) noexcept {
		m_value = daw::move( rhs );
		return *this;
	}

	json_value_t &json_value_t::operator=( json_value_t::null_t ) noexcept {
		m_value = null_t{ };
		return *this;
	}

	json_value_t &json_value_t::operator=( json_value_t::array_t rhs ) noexcept {
		m_value = daw::move( rhs );
		return *this;
	}

	json_value_t &json_value_t::operator=( json_value_t::object_t rhs ) noexcept {
		m_value = daw::move( rhs );
		return *this;
	}

	json_value_t::~json_value_t( ) {}

	bool const &json_value_t::get_boolean( ) const {
		daw::exception::daw_throw_on_false( is_boolean( ),
		                                    "Unexpected value type(" +
		                                      daw::json::to_string( type( ) ) +
		                                      "), expected boolean" );
		return std::get<boolean_t>( m_value );
	}

	bool &json_value_t::get_boolean( ) {
		daw::exception::daw_throw_on_false( is_boolean( ),
		                                    "Unexpected value type(" +
		                                      daw::json::to_string( type( ) ) +
		                                      "), expected boolean" );
		return std::get<boolean_t>( m_value );
	}

	json_value_t::integer_t json_value_t::get_integer( ) const {
		daw::exception::daw_throw_on_false(
		  is_integer( ), "Unexpected value type(" +
		                   daw::json::to_string( type( ) ) + "),expected integer" );
		return std::get<integer_t>( m_value );
	}

	json_value_t::real_t json_value_t::get_real( ) const {
		daw::exception::daw_throw_on_false(
		  is_numeric( ), "Unexpected value type(" +
		                   daw::json::to_string( type( ) ) + "),expected numeric" );
		using namespace boost;
		using namespace daw;
		struct get_real_t {
			json_value_t::real_t operator( )( json_value_t::null_t ) const {
				daw::exception::daw_throw(
				  "Unexpected value type(null), expected numeric" );
			}
			json_value_t::real_t operator( )( json_value_t::string_t ) const {
				daw::exception::daw_throw(
				  "Unexpected value type(string), expected numeric" );
			}
			json_value_t::real_t operator( )( json_value_t::boolean_t ) const {
				daw::exception::daw_throw(
				  "Unexpected value type(boolean), expected numeric" );
			}
			json_value_t::real_t operator( )( json_value_t::array_t ) const {
				daw::exception::daw_throw(
				  "Unexpected value type(array), expected numeric" );
			}
			json_value_t::real_t operator( )( json_value_t::object_t ) const {
				daw::exception::daw_throw(
				  "Unexpected value type(object), expected numeric" );
			}
			json_value_t::real_t
			operator( )( json_value_t::integer_t const &value ) const {
				return static_cast<json_value_t::real_t>( value );
			}
			json_value_t::real_t
			operator( )( json_value_t::real_t const &value ) const {
				return value;
			}
		}; // get_real_t
		return apply_visitor( get_real_t{ } );
	}

	std::string to_string( daw::string_view const &str ) {
		return str.to_string( );
	}

	std::string json_value_t::get_string( ) const {
		daw::exception::daw_throw_on_false(
		  is_string( ), "Unexpected value type" + daw::json::to_string( type( ) ) +
		                  "),expected string" );
		daw::exception::daw_throw_on_true( std::get<string_t>( m_value ).empty( ),
		                                   "Unexpected empty string" );
		return std::get<string_t>( m_value ).to_string( );
	}

	json_string_value json_value_t::get_string_value( ) const {
		daw::exception::daw_throw_on_false(
		  is_string( ), "Unexpected value type(" + daw::json::to_string( type( ) ) +
		                  "),expected string" );
		daw::exception::daw_throw_on_true( std::get<string_t>( m_value ).empty( ),
		                                   "Unexpected empty string" );
		using namespace daw;
		return std::get<string_t>( m_value );
	}

	bool json_value_t::is_integer( ) const noexcept {
		return m_value.index( ) == index_of<integer_t>( );
	}

	bool json_value_t::is_real( ) const noexcept {
		return m_value.index( ) == index_of<real_t>( );
	}

	bool json_value_t::is_numeric( ) const noexcept {
		return is_real( ) || is_integer( );
	}

	bool json_value_t::is_string( ) const noexcept {
		return m_value.index( ) == index_of<string_t>( );
	}

	bool json_value_t::is_boolean( ) const noexcept {
		return m_value.index( ) == index_of<boolean_t>( );
	}

	bool json_value_t::is_null( ) const noexcept {
		return m_value.index( ) == index_of<null_t>( );
	}

	bool json_value_t::is_array( ) const noexcept {
		return m_value.index( ) == index_of<array_t>( );
	}

	bool json_value_t::is_object( ) const noexcept {
		return m_value.index( ) == index_of<object_t>( );
	}

	json_object_value const &json_value_t::get_object( ) const {
		daw::exception::daw_throw_on_false(
		  is_object( ), "Unexpected value type(" + daw::json::to_string( type( ) ) +
		                  "),expected object" );
		return std::get<json_object_value>( m_value );
	}

	json_object_value &json_value_t::get_object( ) {
		daw::exception::daw_throw_on_false(
		  is_object( ), "Unexpected value type(" + daw::json::to_string( type( ) ) +
		                  "),expected object" );
		return std::get<json_object_value>( m_value );
	}

	json_array_value const &json_value_t::get_array( ) const {
		daw::exception::daw_throw_on_false(
		  is_array( ), "Unexpected value type(" + daw::json::to_string( type( ) ) +
		                 "),expected array" );
		return std::get<json_array_value>( m_value );
	}

	json_array_value &json_value_t::get_array( ) {
		daw::exception::daw_throw_on_false(
		  is_array( ), "Unexpected value type(" + daw::json::to_string( type( ) ) +
		                 "),expected array" );
		return std::get<json_array_value>( m_value );
	}

	size_t json_value_t::type( ) const noexcept {
		return m_value.index( );
	}

	std::string to_string( json_object_value const &obj ) {
		std::stringstream ss;
		ss << "{ ";
		const auto &items = obj.members_v;
		if( !items.empty( ) ) {
			ss << '"' << items[0].first << "\" : " << items[0].second;
			for( size_t n = 1; n < items.size( ); ++n ) {
				ss << ", \"" << items[n].first << "\" : " << items[n].second;
			}
		}
		ss << " }";
		return ss.str( );
	}

	std::string json_value_t::to_string( ) const {
		using std::to_string;
		struct to_string_t {
			std::string operator( )( json_value_t::null_t ) const {
				return "null";
			}
			std::string operator( )( json_value_t::string_t const &v ) const {
				using namespace std::string_literals;
				return "\""s + v.to_string( ) + "\""s;
			}
			std::string operator( )( json_value_t::boolean_t v ) const {
				return v ? "True" : "False";
			}
			std::string operator( )( json_value_t::array_t const &v ) const {
				return to_string( v );
			}
			std::string operator( )( json_value_t::object_t const &v ) const {
				return to_string( v );
			}
			std::string operator( )( json_value_t::integer_t const &v ) const {
				return to_string( v );
			}
			std::string operator( )( json_value_t::real_t const &v ) const {
				return to_string( v );
			}
		}; // get_real_t
		return apply_visitor( to_string_t{ } );
	}

	std::string to_string( std::shared_ptr<json_value_t> const &value ) {
		if( value ) {
			return to_string( *value );
		}
		return "{ null }";
	}

	std::ostream &operator<<( std::ostream &os, json_value_t const &value ) {
		os << to_string( value );
		return os;
	}

	std::ostream &operator<<( std::ostream &os,
	                          std::shared_ptr<json_value_t> const &value ) {
		os << to_string( value );
		return os;
	}

	json_object_value::~json_object_value( ) {}

	json_object_value::iterator json_object_value::find( daw::string_view key ) {
		auto const k = key.to_string( );
		return std::find_if( members_v.begin( ), members_v.end( ),
		                     [&]( json_object_value_item const &item ) {
			                     // hack be here
			                     auto const a = item.first.to_string( );
			                     return a == k;
		                     } );
	}

	json_object_value::const_iterator
	json_object_value::find( daw::string_view key ) const {
		auto const k = key.to_string( );
		return std::find_if( members_v.begin( ), members_v.end( ),
		                     [&]( json_object_value_item const &item ) {
			                     // hack be here
			                     auto const a = item.first.to_string( );
			                     return a == k;
		                     } );
	}

	bool json_object_value::has_member( daw::string_view key ) const {
		return find( key ) != end( );
	}

	std::optional<json_value_t>
	json_object_value::operator( )( daw::string_view key ) const {
		auto it = find( key );
		if( it != end( ) ) {
			return std::optional<json_value_t>{ it->second };
		}
		return { };
	}

	json_object_value::mapped_type &
	json_object_value::operator[]( daw::string_view key ) {
		auto pos = find( key );
		if( end( ) == pos ) {
			pos = insert( pos, make_object_value_item( json_string_value( key ),
			                                           json_value_t{ } ) );
		}
		return pos->second;
	}

	json_object_value::mapped_type const &
	json_object_value::operator[]( daw::string_view key ) const {
		auto pos = find( key );
		if( end( ) == pos ) {
			throw std::out_of_range(
			  "Attempt to access an undefined value in a const object" );
		}
		return pos->second;
	}

	int json_value_t::compare( json_value_t const &rhs ) const {
		return to_string( ).compare( rhs.to_string( ) );
	}

	template<>
	json_value_t::integer_t
	get<json_value_t::integer_t>( daw::json::json_value_t const &val ) {
		return val.get_integer( );
	}

	template<>
	json_value_t::real_t
	get<json_value_t::real_t>( daw::json::json_value_t const &val ) {
		return val.get_real( );
	}

	template<>
	json_value_t::string_t
	get<json_value_t::string_t>( daw::json::json_value_t const &val ) {
		return val.get_string_value( );
	}

	template<>
	std::string get<std::string>( daw::json::json_value_t const &val ) {
		return val.get_string( );
	}

	template<>
	json_value_t::boolean_t
	get<json_value_t::boolean_t>( daw::json::json_value_t const &val ) {
		return val.get_boolean( );
	}

	template<>
	json_value_t::object_t
	get<json_value_t::object_t>( daw::json::json_value_t const &val ) {
		return val.get_object( );
	}

	template<>
	json_value_t::array_t
	get<json_value_t::array_t>( daw::json::json_value_t const &val ) {
		return val.get_array( );
	}

	template<>
	json_value_t::null_t
	get<json_value_t::null_t>( daw::json::json_value_t const & ) {
		return json_value_t::null_t{ };
	}

	create_comparison_operators( json_value_t )
} // namespace daw::json

namespace std {
	std::string to_string( daw::json::json_array_value const &arry ) {
		std::stringstream ss;
		ss << "[ ";
		if( !arry.empty( ) ) {
			ss << arry[0];
			for( size_t n = 1; n < arry.size( ); ++n ) {
				ss << ", " << arry[n];
			}
		}
		ss << " ]";
		return ss.str( );
	}
} // namespace std
