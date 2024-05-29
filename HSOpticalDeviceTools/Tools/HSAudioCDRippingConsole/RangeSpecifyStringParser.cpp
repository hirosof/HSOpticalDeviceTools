#include "RangeSpecifyStringParser.hpp"

RangeValueTypeUVector RangeSpecifyStringParseUnsigned( const std::string str ) {

    RangeValueTypeUVector ret;
    RangeValueTypeU range;

    bool isCurrentRangeSpecify = false;
    std::string base( str );
    std::string currentStr;

    base.push_back( '\0' );

    auto itemPush = [] ( RangeValueTypeUVector& vec, RangeValueTypeU value ) {
        if ( value.first > value.second ) {
            RangeValueTypeU swappedValue;
            swappedValue.first = value.second;
            swappedValue.second = value.first;
            vec.push_back( swappedValue );
        } else {
            vec.push_back( value );
        }
    };

    for ( const char c : base ) {
        switch ( c ) {
            // 次の文字は『,』と同じ意味にするためfallthroughとする
            // ・NULL文字 (\0)
            // ・ドット (.) ： 入力ミス対策
            // ・バーティカルバー(|)
            // ・プラス(+)
            case '.':
#if __cplusplus >= 201703L 
                [[fallthrough]];
#endif

            case '+':
#if __cplusplus >= 201703L 
                [[fallthrough]];
#endif

            case '|':
#if __cplusplus >= 201703L 
                [[fallthrough]];
#endif

            case '\0':
#if __cplusplus >= 201703L 
                [[fallthrough]];
#endif

            case ',':
                if ( isCurrentRangeSpecify ) {
                    if ( !currentStr.empty( ) ) {
                        range.second = std::stoul( currentStr );
                    }
                    itemPush( ret, range );
                } else {
                    if ( !currentStr.empty( ) ) {
                        range.second = std::stoul( currentStr );
                        range.first = range.second;
                        itemPush( ret, range );
                    }
                }
                isCurrentRangeSpecify = false;
                currentStr.clear( );
                break;
            case '-':

                if ( isCurrentRangeSpecify ) {
                    if ( !currentStr.empty( ) ) {
                        range.second = std::stoul( currentStr );
                        itemPush( ret, range );

                        range.first = range.second;
                        currentStr.clear( );
                    }
                } else {
                    range.first = std::stoul( currentStr );
                    range.second = range.first;
                    isCurrentRangeSpecify = true;
                    currentStr.clear( );
                }
                break;
            default:
                if ( ( c >= '0' ) && ( c <= '9' ) ) {
                    currentStr.push_back( c );
                }
                break;
        }

    }

    return ret;
}
