// Copyright 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/detail/splitmix64.hpp>
#include <iostream>
#include <limits>
#include <cstdint>
#include <cfloat>
#include <cmath>

int const N = 1024;

static boost::detail::splitmix64 rng;

// integral types, random values

template<class T> void test_roundtrip( T value, int base )
{
    char buffer[ 256 ] = {};

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ) - 1, value, base );

    BOOST_TEST_EQ( r.ec, 0 );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2, base );

    BOOST_TEST_EQ( r2.ec, 0 ) && BOOST_TEST_EQ( v2, value );
}

template<class T> void test_roundtrip_int8( int base )
{
    for( int i = -256; i <= 255; ++i )
    {
        test_roundtrip( static_cast<T>( i ), base );
    }
}

template<class T> void test_roundtrip_uint8( int base )
{
    for( int i = 0; i <= 256; ++i )
    {
        test_roundtrip( static_cast<T>( i ), base );
    }
}

template<class T> void test_roundtrip_int16( int base )
{
    test_roundtrip_int8<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::int16_t w = static_cast<std::uint16_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_uint16( int base )
{
    test_roundtrip_uint8<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::uint16_t w = static_cast<std::uint16_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_int32( int base )
{
    test_roundtrip_int16<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::int32_t w = static_cast<std::uint32_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_uint32( int base )
{
    test_roundtrip_uint16<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::uint32_t w = static_cast<std::uint32_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_int64( int base )
{
    test_roundtrip_int32<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::int64_t w = static_cast<std::uint64_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_uint64( int base )
{
    test_roundtrip_uint32<T>( base );

    for( int i = 0; i < N; ++i )
    {
        std::uint64_t w = static_cast<std::uint64_t>( rng() );
        test_roundtrip( static_cast<T>( w ), base );
    }
}

#ifdef BOOST_CHARCONV_HAS_INT128

// https://stackoverflow.com/questions/25114597/how-to-print-int128-in-g
std::ostream&
operator<<( std::ostream& dest, boost::charconv::int128_t value )
{
    std::ostream::sentry s( dest );
    if ( s ) {
        boost::charconv::uint128_t tmp = value < 0 ? -value : value;
        char buffer[ 128 ];
        char* d = std::end( buffer );
        do
        {
            -- d;
            *d = "0123456789"[ tmp % 10 ];
            tmp /= 10;
        } while ( tmp != 0 );
        if ( value < 0 ) {
            -- d;
            *d = '-';
        }
        int len = std::end( buffer ) - d;
        if ( dest.rdbuf()->sputn( d, len ) != len ) {
            dest.setstate( std::ios_base::badbit );
        }
    }
    return dest;
}

std::ostream&
operator<<( std::ostream& dest, boost::charconv::uint128_t value )
{
    std::ostream::sentry s( dest );
    if ( s ) {
        boost::charconv::uint128_t tmp = value;
        char buffer[ 128 ];
        char* d = std::end( buffer );
        do
        {
            -- d;
            *d = "0123456789"[ tmp % 10 ];
            tmp /= 10;
        } while ( tmp != 0 );
        int len = std::end( buffer ) - d;
        if ( dest.rdbuf()->sputn( d, len ) != len ) {
            dest.setstate( std::ios_base::badbit );
        }
    }
    return dest;
}

inline boost::charconv::uint128_t concatenate(std::uint64_t word1, std::uint64_t word2)
{
    return static_cast<boost::charconv::uint128_t>(word1) << 64 | word2;
}

template<class T> void test_roundtrip128( T value, int base )
{
    char buffer[ 256 ] = {};

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ) - 1, value, base );

    BOOST_TEST_EQ( r.ec, 0 );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2, base );

    if(BOOST_TEST_EQ( r2.ec, 0 ) && BOOST_TEST( v2 == value ))
    {
    }
    else
    {
        std::cerr << "... test failure for value=" << value << "; buffer='" << std::string( buffer, r.ptr ) << "'" << std::endl;
    }
}

template<class T> void test_roundtrip_int128( int base )
{
    for( int i = 0; i < N; ++i )
    {
        boost::charconv::int128_t w = static_cast<boost::charconv::uint128_t>( concatenate(rng(), rng()) );
        test_roundtrip128( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_uint128( int base )
{
    for( int i = 0; i < N; ++i )
    {
        boost::charconv::uint128_t w = static_cast<boost::charconv::uint128_t>( concatenate(rng(), rng()) );
        test_roundtrip128( static_cast<T>( w ), base );
    }
}

template<class T> void test_roundtrip_bv128( int base )
{
    test_roundtrip128( std::numeric_limits<T>::min(), base );
    test_roundtrip128( std::numeric_limits<T>::max(), base );
}
#endif

// integral types, boundary values

template<class T> void test_roundtrip_bv( int base )
{
    test_roundtrip( std::numeric_limits<T>::min(), base );
    test_roundtrip( std::numeric_limits<T>::max(), base );
}

// floating point types, random values

template<class T> void test_roundtrip( T value )
{
    char buffer[ 256 ] = {};

    auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ) - 1, value );

    BOOST_TEST_EQ( r.ec, 0 );

    T v2 = 0;
    auto r2 = boost::charconv::from_chars( buffer, r.ptr, v2 );

    if( BOOST_TEST_EQ( r2.ec, 0 ) && BOOST_TEST_EQ( v2, value ) )
    {
    }
    else
    {
        std::cerr << "... test failure for value=" << value << "; buffer='" << std::string( buffer, r.ptr ) << "'" << std::endl;
    }
}

// floating point types, boundary values

template<class T> void test_roundtrip_bv()
{
    test_roundtrip( std::numeric_limits<T>::min() );
    test_roundtrip( -std::numeric_limits<T>::min() );
    test_roundtrip( std::numeric_limits<T>::max() );
    test_roundtrip( +std::numeric_limits<T>::max() );
}

//

int main()
{
    // integral types, random values

    for( int base = 2; base <= 36; ++base )
    {
        test_roundtrip_int8<std::int8_t>( base );
        test_roundtrip_uint8<std::uint8_t>( base );

        test_roundtrip_int16<std::int16_t>( base );
        test_roundtrip_uint16<std::uint16_t>( base );

        test_roundtrip_int32<std::int32_t>( base );
        test_roundtrip_uint32<std::uint32_t>( base );

        test_roundtrip_int64<std::int64_t>( base );
        test_roundtrip_uint64<std::uint64_t>( base );

        #ifdef BOOST_CHARCONV_HAS_INT128
        test_roundtrip_int128<boost::charconv::int128_t>( base );
        //test_roundtrip_uint128<boost::charconv::uint128_t>( base );
        #endif
    }

    // integral types, boundary values

    for( int base = 2; base <= 36; ++base )
    {
        test_roundtrip_bv<char>( base );
        test_roundtrip_bv<signed char>( base );
        test_roundtrip_bv<unsigned char>( base );

        test_roundtrip_bv<short>( base );
        test_roundtrip_bv<unsigned short>( base );

        test_roundtrip_bv<int>( base );
        test_roundtrip_bv<unsigned int>( base );

        test_roundtrip_bv<long>( base );
        test_roundtrip_bv<unsigned long>( base );

        test_roundtrip_bv<long long>( base );
        test_roundtrip_bv<unsigned long long>( base );

        #ifdef BOOST_CHARCONV_HAS_INT128
        test_roundtrip_bv128<boost::charconv::int128_t>( base );
        //test_roundtrip_bv128<boost::charconv::uint128_t>( base );
        #endif
    }

    // float

    double const q = std::pow( 1.0, -64 );

    {
        for( int i = 0; i < N; ++i )
        {
            float w0 = static_cast<float>( rng() ); // 0 .. 2^64
            test_roundtrip( w0 );

            float w1 = static_cast<float>( rng() * q ); // 0.0 .. 1.0
            test_roundtrip( w1 );

            float w2 = FLT_MAX / static_cast<float>( rng() ); // large values
            test_roundtrip( w2 );

            float w3 = FLT_MIN * static_cast<float>( rng() ); // small values
            test_roundtrip( w3 );
        }

        test_roundtrip_bv<float>();
    }

    // double

    {
        for( int i = 0; i < N; ++i )
        {
            double w0 = rng() * 1.0; // 0 .. 2^64
            test_roundtrip( w0 );

            double w1 = rng() * q; // 0.0 .. 1.0
            test_roundtrip( w1 );

            double w2 = DBL_MAX / rng(); // large values
            test_roundtrip( w2 );

            double w3 = DBL_MIN * rng(); // small values
            test_roundtrip( w3 );
        }

        test_roundtrip_bv<double>();
    }

    // long double

    {
        long double const ql = std::pow( 1.0L, -64 );

        for( int i = 0; i < N; ++i )
        {
            long double w0 = rng() * 1.0L; // 0 .. 2^64
            test_roundtrip( w0 );

            long double w1 = rng() * ql; // 0.0 .. 1.0
            test_roundtrip( w1 );

            long double w2 = LDBL_MAX / rng(); // large values
            test_roundtrip( w2 );

            long double w3 = LDBL_MIN * rng(); // small values
            test_roundtrip( w3 );
        }

        test_roundtrip_bv<long double>();
    }

    return boost::report_errors();
}
