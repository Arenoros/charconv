// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/charconv/from_chars.hpp>
#include <boost/charconv/config.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <cerrno>
#include <cstddef>

namespace boost { namespace charconv { namespace detail {

static constexpr std::array<unsigned char, 256> uchar_values =
    {{255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 255, 255, 255, 255, 255, 255,
      255,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
       25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35, 255, 255, 255, 255, 255,
      255,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
       25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}};

// Convert characters for 0-9, A-Z, a-z to 0-35. Anything else is 255
inline unsigned char digit_from_char(char val) noexcept
{
    return uchar_values[static_cast<std::size_t>(val)];
}

template <typename Integer, typename std::enable_if<std::is_signed<Integer>::value, bool>::type = true>
inline Integer apply_sign(Integer val) noexcept
{
    return -val;
}

template <typename Integer, typename std::enable_if<std::is_unsigned<Integer>::value, bool>::type = true>
inline Integer apply_sign(Integer val) noexcept
{
    return val;
}

template <typename Integer, typename std::enable_if<std::is_integral<Integer>::value, bool>::type = true>
boost::charconv::from_chars_result from_chars_impl(const char* first, const char* last, Integer& value, int base) noexcept
{
    using Unsigned_Integer = typename std::make_unsigned<Integer>::type;
    Unsigned_Integer result = 0;
    Unsigned_Integer overflow_value = 0;
    Unsigned_Integer max_digit = 0;
    
    // Check pre-conditions
    BOOST_CHARCONV_ASSERT_MSG(base >= 2 && base <= 36, "Base must be between 2 and 36 (inclusive)");
    if (!(first <= last))
    {
        return {first, EINVAL};
    }

    // Strip sign if the type is signed
    // Negative sign will be appended at the end of parsing
    BOOST_ATTRIBUTE_UNUSED bool is_negative = false;
    auto next = first;
    BOOST_IF_CONSTEXPR (std::is_signed<Integer>::value)
    {
        if (next != last)
        {
            if (*next == '-')
            {
                is_negative = true;
                ++next;
            }
            else if (*next == '+')
            {
                ++next;
            }
        }

        overflow_value = (std::numeric_limits<Integer>::max)();
        max_digit = (std::numeric_limits<Integer>::max)();

        if (is_negative)
        {
            ++overflow_value;
            ++max_digit;
        }
    }
    else
    {
        overflow_value = (std::numeric_limits<Unsigned_Integer>::max)();
        max_digit = (std::numeric_limits<Unsigned_Integer>::max)();
    }

    overflow_value /= static_cast<Unsigned_Integer>(base);
    max_digit %= base;

    // If the only character was a sign abort now
    if (next == last)
    {
        return {first, EINVAL};
    }

    bool overflowed = false;
    while (next != last)
    {
        auto current_digit = digit_from_char(*next++);

        if (current_digit >= base)
        {
            break;
        }

        if (result < overflow_value || (result == overflow_value && current_digit <= max_digit))
        {
            result = static_cast<Unsigned_Integer>(result * base + current_digit);
        }
        else
        {
            // Required to keep updating the value of next, but the result is garbage
            overflowed = true;
        }
    }

    // Return the parsed value, adding the sign back if applicable
    // If we have overflowed then we do not return the result 
    if (overflowed)
    {
        return {next, ERANGE};
    }

    value = static_cast<Integer>(result);
    BOOST_IF_CONSTEXPR (std::is_signed<Integer>::value)
    {
        if (is_negative)
        {
            value = apply_sign(value);
        }
    }

    return {next, 0};
}

}}} // Namespace boost::charconv::detail

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, char& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned char& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, short& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned short& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, int& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned int& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned long& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, long long& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}

boost::charconv::from_chars_result boost::charconv::from_chars(const char* first, const char* last, unsigned long long& value, int base) noexcept
{
    return boost::charconv::detail::from_chars_impl(first, last, value, base);
}
