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

#include <boost/lexical_cast.hpp>
#include <boost/utility/string_view.hpp>
#include <cstdint>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>

#include <daw/char_range/daw_char_range.h>
#include <daw/daw_exception.h>
#include <daw/daw_operators.h>
#include <daw/daw_range.h>

#include "value_t.h"

namespace daw {
	namespace json {
		namespace impl {
			std::string to_string( value_t const & v ) {
				return v.to_string( );
			}

			std::string to_string( std::type_index const & ti ) {
				using namespace std::string_literals;
				if( ti == std::type_index( typeid( value_t::integer_t ) ) ) {
					return "integer"s;
				}
				if( ti == std::type_index( typeid( value_t::real_t ) ) ) {
					return "real"s;
				}
				if( ti == std::type_index( typeid( value_t::string_t ) ) ) {
					return "string"s;
				}
				if( ti == std::type_index( typeid( value_t::boolean_t ) ) ) {
					return "boolean"s;
				}
				if( ti == std::type_index( typeid( value_t::null_t ) ) ) {
					return "null"s;
				}
				if( ti == std::type_index( typeid( value_t::array_t ) ) ) {
					return "array"s;
				}
				if( ti == std::type_index( typeid( value_t::object_t ) ) ) {
					return "object"s;
				}
				daw::exception::daw_throw( "Invalid type passed to to string, not valid for value_t" );
			}

			value_t::value_t( ) : m_value{nullptr} {}

			value_t::value_t( value_t::integer_t value ) : m_value{std::move( value )} {}

			value_t::value_t( value_t::real_t value ) : m_value{std::move( value )} {}

			value_t::value_t( boost::string_view value ) : m_value{string_t{value}} {}

			value_t::value_t( value_t::string_t value ) : m_value{std::move( value )} {}

			value_t::value_t( value_t::boolean_t value ) : m_value{std::move( value )} {}

			value_t::value_t( value_t::null_t ) : m_value{nullptr} {}

			value_t::value_t( value_t::array_t value ) : m_value{value_t::array_t{std::move( value )}} {}

			value_t::value_t( value_t::object_t value ) : m_value{value_t::object_t{std::move( value )}} {}

			value_t::value_t( value_t const &other ) : m_value{other.m_value} {}

			value_t::value_t( value_t &&other ) noexcept : m_value{std::move( other.m_value )} {}

			value_t &value_t::operator=( value_t &&rhs ) noexcept {
				m_value = std::move( rhs.m_value );
				return *this;
			}

			value_t &value_t::operator=( value_t const &rhs ) {
				return *this = value_t{rhs};
			}

			value_t &value_t::operator=( value_t::integer_t rhs ) {
				m_value = std::move( rhs );
				return *this;
			}

			value_t &value_t::operator=( value_t::real_t rhs ) {
				m_value = std::move( rhs );
				return *this;
			}

			value_t &value_t::operator=( boost::string_view rhs ) {
				m_value = string_value{rhs};
				return *this;
			}

			value_t &value_t::operator=( value_t::string_t rhs ) {
				m_value = std::move( rhs );
				return *this;
			}

			value_t &value_t::operator=( value_t::boolean_t rhs ) {
				m_value = std::move( rhs );
				return *this;
			}

			value_t &value_t::operator=( value_t::null_t ) {
				m_value = nullptr;
				return *this;
			}

			value_t &value_t::operator=( value_t::array_t rhs ) {
				m_value = std::move( rhs );
				return *this;
			}

			value_t &value_t::operator=( value_t::object_t rhs ) {
				m_value = std::move( rhs );
				return *this;
			}

			value_t::~value_t( ) {}

			bool const &value_t::get_boolean( ) const {
				daw::exception::daw_throw_on_false( is_boolean( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                       "), expected boolean" );
				using namespace boost;
				using namespace daw;
				return get<bool>( m_value );
			}

			bool &value_t::get_boolean( ) {
				daw::exception::daw_throw_on_false( is_boolean( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                       "), expected boolean" );
				using namespace boost;
				using namespace daw;
				return get<bool>( m_value );
			}

			value_t::integer_t value_t::get_integer( ) const {
				daw::exception::daw_throw_on_false( is_integer( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                       "),expected integer" );
				using namespace boost;
				using namespace daw;
				return get<integer_t>( m_value );
			}

			value_t::real_t value_t::get_real( ) const {
				daw::exception::daw_throw_on_false( is_numeric( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                       "),expected numeric" );
				using namespace boost;
				using namespace daw;
				struct get_real_t final {
					value_t::real_t operator( )( value_t::null_t ) const {
						daw::exception::daw_throw( "Unexpected value type(null), expected numeric" );
					}
					value_t::real_t operator( )( value_t::string_t ) const {
						daw::exception::daw_throw( "Unexpected value type(string), expected numeric" );
					}
					value_t::real_t operator( )( value_t::boolean_t ) const {
						daw::exception::daw_throw( "Unexpected value type(boolean), expected numeric" );
					}
					value_t::real_t operator( )( value_t::array_t ) const {
						daw::exception::daw_throw( "Unexpected value type(array), expected numeric" );
					}
					value_t::real_t operator( )( value_t::object_t ) const {
						daw::exception::daw_throw( "Unexpected value type(object), expected numeric" );
					}
					value_t::real_t operator( )( value_t::integer_t const &value ) const {
						return static_cast<value_t::real_t>( value );
					}
					value_t::real_t operator( )( value_t::real_t const &value ) const {
						return value;
					}
				}; // get_real_t
				return apply_visitor( get_real_t{} );
			}

			std::string to_string( boost::string_view const &str ) {
				return str.to_string( );
			}

			std::string value_t::get_string( ) const {
				daw::exception::daw_throw_on_false( is_string( ), "Unexpected value type" + daw::json::impl::to_string( type( ) ) +
				                                                      "),expected string" );
				daw::exception::daw_throw_on_true( m_value.empty( ), "Unexpected empty string" );
				// return to_string( *this );
				return get<string_value>( m_value ).to_string( );
			}

			string_value value_t::get_string_value( ) const {
				daw::exception::daw_throw_on_false( is_string( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                      "),expected string" );
				daw::exception::daw_throw_on_true( m_value.empty( ), "Unexpected empty string" );
				using namespace boost;
				using namespace daw;
				return get<string_value>( m_value );
			}

			bool value_t::is_integer( ) const {
				return std::type_index( m_value.type( ) ) == std::type_index( typeid( integer_t ) );
			}

			bool value_t::is_real( ) const {
				return std::type_index( m_value.type( ) ) == std::type_index( typeid( real_t ) );
			}

			bool value_t::is_numeric( ) const {
				return is_real( ) || is_integer( );
			}

			bool value_t::is_string( ) const {
				return std::type_index( m_value.type( ) ) == std::type_index( typeid( string_t ) );
			}

			bool value_t::is_boolean( ) const {
				return std::type_index( m_value.type( ) ) == std::type_index( typeid( boolean_t ) );
			}

			bool value_t::is_null( ) const {
				return std::type_index( m_value.type( ) ) == std::type_index( typeid( null_t ) );
			}

			bool value_t::is_array( ) const {
				return std::type_index( m_value.type( ) ) == std::type_index( typeid( array_t ) );
			}

			bool value_t::is_object( ) const {
				return std::type_index( m_value.type( ) ) == std::type_index( typeid( object_t ) );
			}

			object_value const &value_t::get_object( ) const {
				daw::exception::daw_throw_on_false( is_object( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                      "),expected object" );
				using namespace boost;
				using namespace daw;
				return get<object_value>( m_value );
			}

			object_value &value_t::get_object( ) {
				daw::exception::daw_throw_on_false( is_object( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                      "),expected object" );
				using namespace boost;
				using namespace daw;
				return get<object_value>( m_value );
			}

			array_value const &value_t::get_array( ) const {
				daw::exception::daw_throw_on_false( is_array( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                     "),expected array" );
				using namespace boost;
				using namespace daw;
				return get<array_value>( m_value );
			}

			array_value &value_t::get_array( ) {
				daw::exception::daw_throw_on_false( is_array( ), "Unexpected value type(" + daw::json::impl::to_string( type( ) ) +
				                                                     "),expected array" );
				using namespace boost;
				using namespace daw;
				return get<array_value>( m_value );
			}

			std::type_index value_t::type( ) const noexcept {
				return std::type_index{m_value.type( )};
			}

			std::string to_string( object_value const &obj ) {
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

			std::string value_t::to_string( ) const {
				using std::to_string;
				struct to_string_t final {
					std::string operator( )( value_t::null_t ) const {
						return "null";
					}
					std::string operator( )( value_t::string_t const &v ) const {
						using namespace std::string_literals;
						return "\""s + v.to_string( ) + "\""s;
					}
					std::string operator( )( value_t::boolean_t v ) const {
						return v ? "True" : "False";
					}
					std::string operator( )( value_t::array_t const &v ) const {
						return to_string( v );
					}
					std::string operator( )( value_t::object_t const &v ) const {
						return to_string( v );
					}
					std::string operator( )( value_t::integer_t const &v ) const {
						return to_string( v );
					}
					std::string operator( )( value_t::real_t const &v ) const {
						return to_string( v );
					}
				}; // get_real_t
				return apply_visitor( to_string_t{} );
			}

			std::string to_string( std::shared_ptr<value_t> const &value ) {
				if( value ) {
					return to_string( *value );
				}
				return "{ null }";
			}

			std::ostream &operator<<( std::ostream &os, value_t const &value ) {
				os << to_string( value );
				return os;
			}

			std::ostream &operator<<( std::ostream &os, std::shared_ptr<value_t> const &value ) {
				os << to_string( value );
				return os;
			}

			object_value::~object_value( ) {}

			object_value_item make_object_value_item( string_value first, value_t second ) {
				return std::make_pair<string_value, value_t>( std::move( first ), std::move( second ) );
			}

			object_value::iterator object_value::find( boost::string_view key ) {
				auto const k = key.to_string( );
				return std::find_if( members_v.begin( ), members_v.end( ), [&]( object_value_item const &item ) {
					// hack be here
					auto const a = item.first.to_string( );
					return a == k;
				} );
			}

			object_value::const_iterator object_value::find( boost::string_view key ) const {
				auto const k = key.to_string( );
				return std::find_if( members_v.begin( ), members_v.end( ), [&]( object_value_item const &item ) {
					// hack be here
					auto const a = item.first.to_string( );
					return a == k;
				} );
			}

			bool object_value::has_member( boost::string_view key ) const {
				return find( key ) != end( );
			}

			boost::optional<value_t> object_value::operator( )( boost::string_view key ) const {
				auto it = find( key );
				if( it != end( ) ) {
					return boost::optional<value_t>{it->second};
				}
				return {};
			}

			object_value::mapped_type &object_value::operator[]( boost::string_view key ) {
				auto pos = find( key );
				if( end( ) == pos ) {
					pos = insert( pos, std::make_pair<string_value, value_t>( range::create_char_range( key ),
					                                                          value_t{nullptr} ) );
				}
				return pos->second;
			}

			object_value::mapped_type const &object_value::operator[]( boost::string_view key ) const {
				auto pos = find( key );
				if( end( ) == pos ) {
					throw std::out_of_range( "Attempt to access an undefined value in a const object" );
				}
				return pos->second;
			}
			int value_t::compare( value_t const &rhs ) const {
				// return m_value.compare( rhs.m_value );
				return to_string( ).compare( rhs.to_string( ) );
			}

			create_comparison_operators( value_t );

		} // namespace impl
	}     // namespace json
} // namespace daw

namespace std {
	std::string to_string( daw::json::impl::array_value const &arry ) {
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
