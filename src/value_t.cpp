// The MIT License (MIT)
//
// Copyright (c) 2014-2016 Darrell Wright
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
				assert( m_value_type == value_types::boolean );
				using namespace boost; 
				using namespace daw; 
				return get<bool>( m_value );
			}

			bool & value_t::get_boolean( ) {
				assert( m_value_type == value_types::boolean );
				using namespace boost; 
				using namespace daw; 
				return get<bool>( m_value );
			}

			int64_t const & value_t::get_integral( ) const {
				assert( m_value_type == value_types::integral );
				using namespace boost; 
				using namespace daw; 
				return get<int64_t>( m_value );
			}

			int64_t & value_t::get_integral( ) {
				assert( m_value_type == value_types::integral );
				using namespace boost; 
				using namespace daw; 
				return get<int64_t>( m_value );
			}

			double const & value_t::get_real( ) const {
				assert( m_value_type == value_types::real );
				using namespace boost; 
				using namespace daw; 
				return get<double>( m_value );
			}

			double & value_t::get_real( ) {
				assert( m_value_type == value_types::real );
				using namespace boost; 
				using namespace daw; 
				return get<double>( m_value );
			}

			std::string value_t::get_string( ) const {
				assert( m_value_type == value_types::string );
				assert( !m_value.empty( ) );
				return to_string( *this );
			}

			string_value value_t::get_string_value( ) const {
				assert( m_value_type == value_types::string );
				assert( !m_value.empty( ) );
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
				assert( m_value_type == value_types::object );
				using namespace boost; 
				using namespace daw; 
				return get<object_value>( m_value );
			}

			object_value & value_t::get_object( ) {
				assert( m_value_type == value_types::object );
				using namespace boost; 
				using namespace daw; 
				return get<object_value>( m_value );
			}

			array_value const & value_t::get_array( ) const {
				assert( m_value_type == value_types::array );
				using namespace boost; 
				using namespace daw; 
				return get<array_value>( m_value );
			}

			array_value & value_t::get_array( ) {
				assert( m_value_type == value_types::array );
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
				return std::find_if( members_v.begin( ), members_v.end( ), [key]( object_value_item const & item ) {
						return item.first == key;
						} );
			}

			object_value::const_iterator object_value::find( boost::string_view key ) const {
				return std::find_if( members_v.begin( ), members_v.end( ), [key]( object_value_item const & item ) {
						return item.first == key;
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
