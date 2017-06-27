/*
    Small C++ header providing facilities to ask a user for input from the command line
    Copyright (C) 2017 Fergus Waugh

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
    OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ASK_FOR_H_OB4J7TGX
#define ASK_FOR_H_OB4J7TGX

#include <iostream>
#include <sstream>
#include <array>
#include <vector>
#include <string>
#include <tuple>

template <typename T, std::size_t N, std::size_t... Is>
inline std::istream& input_stream(std::istream& is, std::array<T, N>& array,
                                  std::index_sequence<Is...>)
{
    (void)std::initializer_list<int>{(is >> std::get<Is>(array), 0)...};
    return is;
}

template <typename T, std::size_t N>
inline std::istream& operator>>(std::istream& is, std::array<T, N>& array)
{
    return input_stream(is, array, std::make_index_sequence<N>());
}

template <typename T>
inline std::istream& operator>>(std::istream& is, std::vector<T>& vector)
{
    while (true) {
        T t;
        is >> t;
        if (is.fail()) break;
        vector.push_back(t);
    }

    // As long as we haven't encountered a serious error, just ignore a failure (this is simply the
    // end of the list of things to put in the container)
    if (is.fail()) {
        is.clear(is.rdstate() ^ std::ios_base::failbit);
    }

    return is;
}

struct Eof_exception : std::exception {
    const char* what() const noexcept { return "End of file"; }
};

// Function reads a line and attempts to fill objects. If there is no line (eof) a special
// exception is thrown.
//
// If there is an error parsing into the type, the fail bit is set. If the whole line was used to
// fill the object, the eof bit is set (this is therefore expected on a successful run).
template <typename... T>
inline std::istream& get_line_fill(T&... t)
{
    auto& is = std::cin;
    std::string s;

    std::getline(is, s);

    if (is.eof()) throw Eof_exception{};

    // Check if anything is wrong, including eof, fail, or bad
    // If so don't bother going any further
    if (!is.good()) return is;

    // If there is no input and the object is a single string, just return an empty string (and set
    // stream to eof to indicate success)
    const bool single_string_empty = [&s](auto&& a, auto&&... b) {
        if (sizeof...(b) == 0 && s.empty() &&
            std::is_same<std::decay_t<decltype(a)>, std::string>::value)
        {
            return true;
        }
        return false;
    }(t...);
    if (single_string_empty) {
        is.setstate(std::ios_base::eofbit);
        return is;
    }

    std::istringstream ss{s};
    (void)std::initializer_list<int>{(ss >> t, 0)...};

    // Sometimes eof is not set at the end of reading a stream, so check here and set if necessary.
    // I believe this is dependent on the type, for example when reading ints it is set, but for
    // chars it isn't. If I don't do this, then for some types the ask_for function will think
    // there's excess input, since eof is used to check that the stream is empty.
    if (ss.rdbuf()->in_avail() == 0) {
        ss.clear(ss.rdstate() | std::ios_base::eofbit);
    }

    is.setstate(ss.rdstate());

    return is;
}

// Test for condition errors ----------------------------------------------------------------------

template <typename T, typename F>
inline bool condition_errors(T& t, F f)
{
    return !f(t);
}

/*
 * The following is commented out because I don't know if it makes sense to have it by default.
 * Essentially you might want a condition on a vector to be checked on all its elements, rather
 * than the vector as a whole. However, I'm not sure this behaviour makes particular sense, hence
 * the commented code.
 *
template <typename T, typename F>
inline bool condition_errors(std::vector<T>& t, F f)
{
    std::size_t error_count = 0;
    for (auto& x : t) {
        if (condition_errors(x, f)) ++error_count;
    }
    return error_count > 0;
}

template <typename T, std::size_t N, typename F>
inline bool condition_errors(std::array<T, N>& t, F f)
{
    std::size_t error_count = 0;
    for (auto& x : t) {
        if (condition_errors(x, f)) ++error_count;
    }
    return error_count > 0;
}
*/

// Main implementation functions ------------------------------------------------------------------

template <typename... T, typename F_of_T>
inline bool ask_for_impl(const std::string& message, F_of_T&& condition,
                         const std::string& condition_error, const std::string& parse_error,
                         T&... t)
{
    std::cout << message;

    get_line_fill(t...);

    if (std::cin.bad()) {
        std::cerr << "Cannot read from stream\n";
    } else if (std::cin.fail()) {
        std::cout << parse_error << '\n';
    } else if (!std::cin.eof()) {
        std::cout << "Error: excess input\n";
    } else {
        bool errors = true;
        (void)std::initializer_list<int>{(errors = condition_errors(t, condition), 0)...};

        if (errors) {
            std::cout << condition_error << '\n';
        } else {
            std::cin.clear();
            return true;
        }
    }

    std::cin.clear();
    return false;
}

template <typename... T, typename F_of_T, std::size_t... I>
inline bool ask_for_impl(const std::string& message, F_of_T&& condition,
                         const std::string& condition_error, const std::string& parse_error,
                         std::tuple<T...>& tuple, std::index_sequence<I...>)
{
    return ask_for_impl(message, std::forward<F_of_T>(condition), condition_error, parse_error,
                        std::get<I>(tuple)...);
}

// Ask for multiple -------------------------------------------------------------------------------

template <typename T1, typename T2, typename... T, typename F_of_T>
inline std::tuple<T1, T2, T...>
ask_for(const std::string& message, F_of_T condition,
        const std::string& condition_error = "Error: unmet condition",
        const std::string& parse_error = "Error: parse error")
{
    while (true) {
        std::tuple<T1, T2, T...> tuple;
        if (ask_for_impl(message, condition, condition_error, parse_error, tuple,
                         std::make_index_sequence<std::tuple_size<decltype(tuple)>::value>())) {
            return tuple;
        }
    }
}

template <typename T1, typename T2, typename... T>
inline std::tuple<T1, T2, T...> ask_for(const std::string& message = "Enter input: ",
                                        const std::string& parse_error = "Error: parse error")
{
    return ask_for<T1, T2, T...>(message, [](auto) { return true; }, "", parse_error);
}

// Ask for single ---------------------------------------------------------------------------------

template <typename T, typename F_of_T>
inline T ask_for(const std::string& message, F_of_T condition,
                 const std::string& condition_error = "Error: unmet condition",
                 const std::string& parse_error = "Error: parse error")
{
    while (true) {
        T t;
        if (ask_for_impl(message, condition, condition_error, parse_error, t)) {
            return t;
        }
    }
}

template <typename T>
inline T ask_for(const std::string& message = "Enter input: ",
                 const std::string& parse_error = "Error: parse error")
{
    return ask_for<T>(message, [](auto) { return true; });
}

#endif /* end of include guard: ASK_FOR_H_OB4J7TGX */

