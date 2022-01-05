/*
* Copyright (C) 2014 Łukasz Czerwiński
*
* GitHub: https://github.com/wo3kie/groupBy
*
* Distributed under the BSD Software License (see file license)
*/

#ifndef SPYDER_GROUP_BY_H
#define SPYDER_GROUP_BY_H

#include <map>
#include <vector>

namespace spyder
{
    /*
     * return_type
     */

    template< typename T >
    struct return_type
    {
        typedef typename return_type< typename std::decay< decltype( &T::operator() )>::type >::type type;
    };

    template< typename R, typename C, typename... A >
    struct return_type< R ( C::* )( A... ) > 
    {
        typedef typename std::decay< R >::type type;
    };

    template< typename R, typename C, typename... A >
    struct return_type< R ( C::* )( A... ) const > 
    {
        typedef typename std::decay< R >::type type;
    };

    template< typename R, typename... A >
    struct return_type< R ( * )( A... ) > 
    {
        typedef typename std::decay< R >::type type;
    };
}

namespace spyder
{
    /*
     * GroupBy
     */

    template< typename Arg, typename... Ts >
    struct GroupBy;

    template< typename Arg, typename T, typename... Ts >
    struct GroupBy< Arg, T, Ts... >
    {
        typedef std::map<
            typename spyder::return_type< typename std::decay< T >::type >::type,
            typename GroupBy< Arg, Ts... >::return_type
        > return_type;
    };

    template< typename Arg, typename T >
    struct GroupBy< Arg, T >
    {
        typedef std::map<
            typename return_type< typename std::decay< T >::type >::type,
            std::vector< Arg >
        > return_type;
    };
}

namespace spyder
{
    /*
     * groupByImpl
     */

    template< typename Map, typename Iterator, typename F >
    void groupByImpl( Map & map, Iterator && current, F && f )
    {
        map[ f( *current ) ].push_back( *current );
    }

    template< typename Map, typename Iterator, typename F, typename... Fs >
    void groupByImpl( Map & map, Iterator && current, F && f, Fs &&... fs )
    {
        groupByImpl( map[ f( *current ) ], current, std::forward< Fs >( fs )... );
    }
}

/*
 * groupBy
 */

template< typename Iterator, typename F, typename... Fs >
typename spyder::GroupBy< typename std::iterator_traits< Iterator >::value_type, F, Fs... >::return_type
groupBy( Iterator begin, Iterator const end, F && f, Fs &&... fs )
{
    typename spyder::GroupBy< typename std::iterator_traits< Iterator >::value_type, F, Fs... >::return_type result;
    for( /* empty */ ; begin != end ; ++ begin )
    {
        spyder::groupByImpl( result, begin, std::forward< F >( f ), std::forward< Fs >( fs )... );
    }

    return result;
}

#endif
