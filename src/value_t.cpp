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
#include <daw/daw_range.h>
#include <daw/daw_exception.h>
#include <daw/daw_operators.h>

#include "value_t.h"

namespace daw {
	namespace json {
		namespace impl {

			value_t::value_t( ):
				m_value{ },
				m_value_type{ value_types::null } { }

			value_t::value_t( int64_t const & value ):
				m_value{ value },
				m_value_type{ value_types::integral } { }

			value_t::value_t( double const & value ):
				m_value{ value },
				m_value_type{ value_types::real } { }

			value_t::value_t( std::string const & value ):
				m_value{ string_value{ range::create_char_range( value ) } },
				m_value_type{ value_types::string } { }

			value_t::value_t( boost::string_view value ):
				m_value{ string_value{ value } },
				m_value_type{ value_types::string } { }

			value_t::value_t( string_value value ):
				m_value{ value },
				m_value_type{ value_types::string } { }

			value_t::value_t( bool value ):
				m_value{ value },
				m_value_type{ value_types::boolean } { }

			value_t::value_t( std::nullptr_t ):
				m_value{ },
				m_value_type{ value_types::null } { }

			value_t::value_t( array_value value ):
				m_value{ array_value{ std::move( value ) } },
				m_value_type( value_types::array ) { }

			value_t::value_t( object_value value ):
				m_value{ object_value{ std::move( value ) } },
				m_value_type{ value_types::object } { }

			value_t::value_t( value_t const & other ):
				m_value{ other.m_value },
				m_value_type{ other.m_value_type } { }

			value_t & value_t::operator=( value_t const & rhs ) {
				if( this != &rhs ) {
					m_value = rhs.m_value;
					m_value_type = rhs.m_value_type;
				}
				return *this;
			}

			value_t::value_t( value_t && other ):
				m_value{ std::move( other.m_value ) },
				m_value_type{ other.m_value_type } { }

			value_t & value_t::operator=( value_t && rhs ) {
				if( this != &rhs ) {
					m_value = std::move( rhs.m_value );
					m_value_type = rhs.m_value_type;
				}
				return *this;
			}

			value_t::~value_t( ) { }

			bool const & value_t::get_boolean( ) const {
				daw::exception::daw_throw_on_false( m_value_type == value_types::boolean );
				using namespace boost; 
				using namespace daw; 
				return get<bool>( m_value );
			}

			bool & value_t::get_boolean( ) {
				daw::exception::daw_throw_on_false( m_value_type == value_types::boolean );
				using namespace boost; 
				using namespace daw; 
				return get<bool>( m_value );
			}

			value_t::integral_t value_t::get_integral( ) const {
				daw::exception::daw_throw_on_false( m_value_type == value_types::integral );
				using namespace boost; 
				using namespace daw; 
				return get<integral_t>( m_value );
			}

			namespace {
				constexpr bool is_numeric( value_t::value_types const t ) noexcept {
					return t == value_t::value_types::real || t == value_t::value_types::integral;
				}
			}	// namespace anonymous

			value_t::real_t value_t::get_real( ) const {
				daw::exception::daw_throw_on_false( is_numeric( m_value_type )  );
				using namespace boost;
				using namespace daw;
				if( m_value_type == value_types::integral ) {
					return static_cast<double>( get_integral( ) );
				}
				return get<double>( m_value );
			}

			std::string to_string( boost::string_view const & str ) {
				return str.to_string( );
			}

			std::string value_t::get_string( ) const {
				daw::exception::daw_throw_on_false( m_value_type == value_types::string );
				daw::exception::daw_throw_on_false( !m_value.empty( ) );
				//return to_string( *this );
				return get<string_value>( m_value ).to_string( );
			}

			string_value value_t::get_string_value( ) const {
				daw::exception::daw_throw_on_false( m_value_type == value_types::string );
				daw::exception::daw_throw_on_false( !m_value.empty( ) );
				using namespace boost; 
				using namespace daw; 
				return get<string_value>( m_value );
			}

			bool value_t::is_integral( ) const {
				return m_value_type == value_types::integral;
			}

			bool value_t::is_real( ) const {
				return m_value_type == value_types::real;
			}

			bool value_t::is_string( ) const {
				return m_value_type == value_types::string;
			}

			bool value_t::is_boolean( ) const {
				return m_value_type == value_types::boolean;
			}

			bool value_t::is_null( ) const {
				return m_value_type == value_types::null;
			}

			bool value_t::is_array( ) const {
				return m_value_type == value_types::array;
			}

			bool value_t::is_object( ) const {
				return m_value_type == value_types::object;
			}

			object_value const & value_t::get_object( ) const {
				daw::exception::daw_throw_on_false( m_value_type == value_types::object );
				using namespace boost; 
				using namespace daw; 
				return get<object_value>( m_value );
			}

			object_value & value_t::get_object( ) {
				daw::exception::daw_throw_on_false( m_value_type == value_types::object );
				using namespace boost; 
				using namespace daw; 
				return get<object_value>( m_value );
			}

			array_value const & value_t::get_array( ) const {
				daw::exception::daw_throw_on_false( m_value_type == value_types::array );
				using namespace boost; 
				using namespace daw; 
				return get<array_value>( m_value );
			}

			array_value & value_t::get_array( ) {
				daw::exception::daw_throw_on_false( m_value_type == value_types::array );
				using namespace boost; 
				using namespace daw; 
				return get<array_value>( m_value );
			}

			value_t::value_types value_t::type( ) const {
				return m_value_type;
			}

			std::string to_string( object_value const & obj ) {
				std::stringstream ss;
				ss << "{ ";
				const auto & items = obj.members_v;
				if( !items.empty( ) ) {
					ss << '"' << items[0].first << "\" : " << items[0].second;
					for( size_t n = 1; n < items.size( ); ++n ) {
						ss << ", \"" << items[n].first << "\" : " << items[n].second;
					}
				}
				ss << " }";
				return ss.str( );
			}

			std::string to_string( value_t const & value ) {
				std::stringstream ss;
				switch( value.type( ) ) {
					case value_t::value_types::array:
						ss << to_string( value.get_array( ) );
						break;
					case value_t::value_types::boolean: 
						ss << (value.get_boolean( ) ? "True" : "False");
						break;
					case value_t::value_types::integral: 
						ss << value.get_integral( );
						break;
					case value_t::value_types::null: 
						ss << "null";
						break;
					case value_t::value_types::object:
						ss << to_string( value.get_object( ) );
						break;
					case value_t::value_types::real: 
						ss << value.get_real( );
						break;
					case value_t::value_types::string: 
						ss << '"' << value.get_string( ) << '"';
						break;
					default: throw std::runtime_error( "Unexpected value type" );
				}
				return ss.str( );
			}

			std::string to_string( std::shared_ptr<value_t> const & value ) {
				if( value ) {
					return to_string( *value );
				}
				return "{ null }";
			}

			std::ostream & operator<<( std::ostream & os, value_t const & value ) {
				os << to_string( value );
				return os;
			}

			std::ostream & operator<<( std::ostream & os, std::shared_ptr<value_t> const & value ) {
				os << to_string( value );
				return os;
			}

			object_value::~object_value( ) { }

			object_value_item make_object_value_item( string_value first, value_t second ) {
				return std::make_pair<string_value, value_t>( std::move( first ), std::move( second ) );
			}

			object_value::iterator object_value::find( boost::string_view key ) {
				auto const k = key.to_string( );
				return std::find_if( members_v.begin( ), members_v.end( ), [&]( object_value_item const & item ) {
						// hack be here
						auto const a = item.first.to_string( );
						return a == k;
					} );
			}

			object_value::const_iterator object_value::find( boost::string_view key ) const {
				auto const k = key.to_string( );
				return std::find_if( members_v.begin( ), members_v.end( ), [&]( object_value_item const & item ) {
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
					return boost::optional<value_t>{ it->second };
				}
				return { };
			}

			object_value::mapped_type & object_value::operator[]( boost::string_view key ) {
				auto pos = find( key );
				if( end( ) == pos ) {
					pos = insert( pos, std::make_pair<string_value, value_t>( range::create_char_range( key ), value_t( nullptr ) ) );
				}
				return pos->second;
			}

			object_value::mapped_type const & object_value::operator[]( boost::string_view key ) const {
				auto pos = find( key );
				if( end( ) == pos ) {
					throw std::out_of_range( "Attempt to access an undefined value in a const object" );
				}
				return pos->second;
			}
			int value_t::compare( value_t const & rhs ) const {
				//return m_value.compare( rhs.m_value );
				return to_string( *this ).compare( to_string( rhs ) );
			}

			create_comparison_operators( value_t );

			std::string to_string( value_t::value_types type ) noexcept {
				switch( type ) {
					case value_t::value_types::integral: return "integral";
					case value_t::value_types::real: return "real";
					case value_t::value_types::string: return "string";
					case value_t::value_types::boolean: return "boolean";
					case value_t::value_types::null: return "null";
					case value_t::value_types::array: return "array";
					case value_t::value_types::object: return "object";
				}
				std::abort( );
			}


		}    // namespace impl
	}    // namespace json
}    // namespace daw

namespace std {
	std::string to_string( daw::json::impl::array_value const & arry ) {
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
}
