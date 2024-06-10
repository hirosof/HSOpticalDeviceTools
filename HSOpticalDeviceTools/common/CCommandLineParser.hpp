#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>

template <typename char_type> class CCommandLineParserType {
public:
	using String = std::basic_string<char_type>;

	struct  TElement {
		String name;
		String value;
	};

	using VectorElementArray = std::vector<TElement>;

private:
	VectorElementArray m_elements;

	static   size_t  Splitter( std::vector<String>* pSplitedCommandLine, const String commandLine ) {


		if ( pSplitedCommandLine == nullptr ) return 0;

		String current;

		char_type back_c = '\0';

		bool skip_flag = false;
		bool string_block_flag = false;

		for ( char_type c : commandLine ) {
			skip_flag = false;
			switch ( c ) {
				case ' ':
					if ( !string_block_flag ) {
						skip_flag = true;
						if ( !current.empty( ) ) {
							pSplitedCommandLine->push_back( current );
							current.clear( );
						}
					}
					break;
				case '\"':
					string_block_flag = !string_block_flag;
					break;
				default:
					break;
			}
			if ( skip_flag == false ) {
				current.push_back( c );
			}
			back_c = c;
		}

		if ( !current.empty( ) ) {
			pSplitedCommandLine->push_back( current );
		}
		return pSplitedCommandLine->size( );
	}

	static String RemoveQuote( const String  str ) {

		String s;

		char_type prev = '\0';

		for ( char_type c : str ) {

			if ( prev == '\\' ) {

				if ( c != '\"' ) {
					s.push_back( '\\' );
				}

				s.push_back( c );

				prev = ( c == '\\' ) ? prev = '\0' : prev = c;

			} else {
				if ( ( c != '\\' ) && ( c != '\"' ) ) {
					s.push_back( c );
				}

				prev = c;
			}
		}

		return s;
	}

	static TElement MakeElement( const String name, const String value ) {
		TElement e;
		e.name = name;
		e.value = value;
		return e;
	}

public:


	CCommandLineParserType( ) = default;
	~CCommandLineParserType( ) = default;


	virtual size_t parse( const String commandLine ) {


		std::vector<String> v;

		if ( Splitter( &v, commandLine ) == 0 ) return 0;

		this->m_elements.clear( );

		bool next_value_check_flag = false;
		String option_name;
		size_t option_switch_size = 1;
		size_t  before_quote_coron_pos = 0;
		TElement element;
		char_type empty = '\0';


		for ( String current : v ) {

			if ( (current.length() == 1 ) &&(current[0] == ':' )) continue;


			if ( current[0] == '/' || current[0] == '-' ) {

				if ( next_value_check_flag ) {
					m_elements.push_back( MakeElement( option_name, &empty ) );
				}

				before_quote_coron_pos = 0;

				for ( size_t i = 1; i < current.size( ); i++ ) {
					if ( current[i] == ':' ) {
						before_quote_coron_pos = i;
						break;
					}
					if ( current[i] == '\"' ) {
						break;
					}
				}

				option_switch_size = 1;
				if ( ( current[0] == '-' ) && ( current[1] == '-' ) ) {
					option_switch_size = 2;
				}

				if ( before_quote_coron_pos != 0 ) {
					element.name = current.substr( option_switch_size, before_quote_coron_pos - option_switch_size );
					element.value = RemoveQuote( current.substr( before_quote_coron_pos + 1 ) );
					m_elements.push_back( element );
					next_value_check_flag = false;
				} else {
					option_name = current.substr( option_switch_size );
					next_value_check_flag = true;
				}
			} else {
				if ( next_value_check_flag ) {
					m_elements.push_back( MakeElement( option_name, RemoveQuote( current ) ) );
					next_value_check_flag = false;
				} else {
					m_elements.push_back( MakeElement( &empty, RemoveQuote( current ) ) );
				}
			}

		}

		if ( next_value_check_flag ) {
			m_elements.push_back( MakeElement( option_name, &empty ) );
		}

		return m_elements.size( );
	}


	size_t count( void) const {
		return m_elements.size( );
	}


	const  TElement get( size_t index ) {
		return m_elements.at( index );
	}


	typename VectorElementArray::const_iterator  cbegin( void ) const {
		return m_elements.cbegin( );
	}

	typename VectorElementArray::const_iterator  cend( void ) const {
		return m_elements.cend( );
	}

};

using CCommandLineParserA = CCommandLineParserType<char>;
using CCommandLineParserW = CCommandLineParserType<wchar_t>;


template <typename char_type> class CCommandLineParserExType  :  public CCommandLineParserType<char_type>{
public:
	using String = typename CCommandLineParserType<char_type>::String;
	using TElement = typename CCommandLineParserType<char_type>::TElement;
	using VectorElementArray = typename CCommandLineParserType<char_type>::VectorElementArray;
	using VectorStringArray = std::vector<String>;
	using ElementMap = std::unordered_map<String, std::vector<String>>;


private:
	ElementMap m_mapElements;

public:

	CCommandLineParserExType( ) = default;
	~CCommandLineParserExType( ) = default;

	size_t parse( const String commandLine ) override{

		if ( CCommandLineParserType<char_type>::parse( commandLine ) == 0 ) {
			return 0;
		}

		m_mapElements.clear( );

		for ( auto element = this->cbegin( ); element != this->cend( ); element++ ) {
			m_mapElements[element->name].push_back( element->value );
		}

		return this->count( );
	}


	size_t countOfNamedOption(const String name )const {
		auto it = this->m_mapElements.find( name );
		if ( it == m_mapElements.end( ) ) return 0;
		return it->second.size( );
	}

	size_t countOfUnnamedOption( void )const {
		char_type empty = '\0';
		return countOfNamedOption( &empty );
	}


	bool hasNamedOption(const String name )const {
		auto it = this->m_mapElements.find( name );
		return ( it != m_mapElements.end( ) );
	}

	String getNamedOptionValue( const String name,const size_t  index = 0 ) const{
		auto it = this->m_mapElements.find( name );
		if ( it == m_mapElements.end( ) ) return String( );
		return ( it->second.size( ) > index ) ? it->second.at( index ) : String( );
	}

	template <typename T> T  getNamedOptionTypeValue( const String name, const size_t  index = 0 , const T defaultT = 0 ) const {
		String s = getNamedOptionValue( name, index );
		if ( s.empty( ) ) return defaultT;
		std::basic_istringstream<char_type>  stream( s );
		T v = defaultT;
		stream >> v;
		return  v;
	}


	VectorStringArray getNamedOptionValues( const String name ) const{
		auto it = this->m_mapElements.find( name );
		if ( it == m_mapElements.end( ) ) return VectorStringArray( );
		return it->second;
	}

	String getUnnamedOptionValue(const size_t  index = 0 )const {
		char_type empty = '\0';
		auto it = this->m_mapElements.find( &empty );
		if ( it == m_mapElements.end( ) ) return String( );
		return ( it->second.size( ) > index ) ? it->second.at( index ) : String( );
	}

	template <typename T> T  getUnnamedOptionTypeValue(  const size_t  index = 0, const T defaultT = 0 ) const {
		String s = getUnnamedOptionValue( index );
		if ( s.empty( ) ) return defaultT;
		std::basic_istringstream<char_type>  stream( s );
		T v = defaultT;
		stream >> v;
		return  v;
	}


	VectorStringArray getUnnamedOptionValues( void ) const{
		char_type empty = '\0';
		auto it = this->m_mapElements.find( &empty );
		if ( it == m_mapElements.end( ) ) return VectorStringArray( );
		return it->second;
	}
};

using CCommandLineParserExA = CCommandLineParserExType<char>;
using CCommandLineParserExW = CCommandLineParserExType<wchar_t>;
