// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv.hpp>
#include <boost/core/lightweight_test.hpp>
#include <system_error>
#include <limits>
#include <random>
#include <array>
#include <cstdint>

constexpr std::size_t N = 1024;
static std::mt19937_64 rng(42);

template <typename T>
void test_non_finite()
{
    constexpr std::array<T, 6> values = {{std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(),
                                         std::numeric_limits<T>::quiet_NaN(), -std::numeric_limits<T>::quiet_NaN(),
                                         std::numeric_limits<T>::signaling_NaN(), -std::numeric_limits<T>::signaling_NaN()}};

    for (const auto val : values)
    {
        char buffer[2];
        auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), val);
        BOOST_TEST(r.ec == std::errc::result_out_of_range);
    }

    char inf_buffer[3];
    constexpr T inf_val = std::numeric_limits<T>::infinity();
    auto r_inf = boost::charconv::to_chars(inf_buffer, inf_buffer + 3, inf_val);
    BOOST_TEST(r_inf);
    BOOST_TEST(!std::memcmp(inf_buffer, "inf", 3));

    char nan_buffer[3];
    constexpr T nan_val = std::numeric_limits<T>::quiet_NaN();
    auto r_nan = boost::charconv::to_chars(nan_buffer, nan_buffer + 3, nan_val);
    BOOST_TEST(r_nan);
    BOOST_TEST(!std::memcmp(nan_buffer, "nan", 3));

    char neg_nan_buffer[9];
    auto r_neg_nan = boost::charconv::to_chars(neg_nan_buffer, neg_nan_buffer + 9, -nan_val);
    BOOST_TEST(r_neg_nan);
    BOOST_TEST(!std::memcmp(neg_nan_buffer, "-nan(ind)", 9));

    char snan_buffer[9];
    constexpr T snan_val = std::numeric_limits<T>::signaling_NaN();
    auto r_snan = boost::charconv::to_chars(snan_buffer, snan_buffer + 9, snan_val);
    BOOST_TEST(r_snan);
    BOOST_TEST(!std::memcmp(snan_buffer, "nan(snan)", 9));
};

#ifdef BOOST_CHARCONV_HAS_FLOAT128
template <>
void test_non_finite<__float128>()
{
    constexpr std::array<__float128, 4> values = {{INFINITY, -INFINITY, NAN, -NAN}};

    for (const auto val : values)
    {
        char buffer[2];
        auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), val);
        BOOST_TEST(r.ec == std::errc::result_out_of_range);
    }

    char inf_buffer[3];
    auto r_inf = boost::charconv::to_chars(inf_buffer, inf_buffer + 3, values[0]);
    BOOST_TEST(r_inf);
    BOOST_TEST(!std::memcmp(inf_buffer, "inf", 3));

    char nan_buffer[3];
    auto r_nan = boost::charconv::to_chars(nan_buffer, nan_buffer + 3, values[2]);
    BOOST_TEST(r_nan);
    BOOST_TEST(!std::memcmp(nan_buffer, "nan", 3));

    char neg_nan_buffer[9];
    auto r_neg_nan = boost::charconv::to_chars(neg_nan_buffer, neg_nan_buffer + 9, values[3]);
    BOOST_TEST(r_neg_nan);
    BOOST_TEST(!std::memcmp(neg_nan_buffer, "-nan(ind)", 9));
}
#endif

template <typename T>
void test_min_buffer_size()
{
    #ifdef BOOST_MSVC
    std::uniform_real_distribution<T> dist((std::numeric_limits<T>::min)(), (std::numeric_limits<T>::max)());
    #else
    std::uniform_real_distribution<T> dist((std::numeric_limits<T>::lowest)(), (std::numeric_limits<T>::max)());
    #endif

    // No guarantees are made for fixed, especially in this domain
    auto formats = {boost::charconv::chars_format::hex,
                    boost::charconv::chars_format::scientific,
                    boost::charconv::chars_format::general};

    for (const auto format : formats)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            char buffer[boost::charconv::limits<T>::max_chars10];
            const T value = dist(rng);

            if (!std::isnormal(value))
            {
                continue;
            }

            auto r = boost::charconv::to_chars(buffer, buffer + sizeof(buffer), value, format);
            if (!BOOST_TEST(r))
            {
                std::cerr << "Overflow for: " << value << std::endl; // LCOV_EXCL_LINE
            }
        }
    }
}

int main()
{
    test_non_finite<float>();
    test_non_finite<double>();
    test_non_finite<long double>();

    #ifdef BOOST_CHARCONV_HAS_FLOAT128
    test_non_finite<__float128>();
    #endif

    test_min_buffer_size<float>();
    test_min_buffer_size<double>();
    test_min_buffer_size<long double>();

    return boost::report_errors();
}
