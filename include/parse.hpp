#pragma once

#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <typeinfo>
#include <cxxabi.h>

#include "types.hpp"

#include <iostream>

namespace stdx::details {

template<typename T>
bool is_type_matches_placeholder(std::string_view placeholder) {
    if (placeholder == "%d") {
        return std::is_integral_v<T>;
    } else if (placeholder == "%s") {
        return std::is_same_v<T, std::string> ||
               std::is_same_v<T, std::string_view>;
    } else if (placeholder == "%u") {
        return std::is_integral_v<T> && !std::is_same_v<T, bool>
            && std::is_unsigned_v<T>; 
    } else if (placeholder == "%f") {
        return std::is_floating_point_v<T>; 
    }
    return false;
};


// template<typename Head, typename... Tail>
// std::expected<std::tuple<Head, Tail...>, details::scan_error> 
// operator+(std::expected<std::tuple<Tail...>, details::scan_error> tuple_, std::expected<Head, details::scan_error> value_) {
//     // tuple_ содержит либор кортеж, либо код ошибок
//     if (tuple_.has_value()) {
//         if (value_.has_value()) {
//             return std::tuple_cat(std::make_tuple(value_), tuple_); // продолжаем собирать результаты парсинга в кортеж
//         } else {
//             return value_.error();  // если несоответствие плейсхолдера и типа встречается впервые 
//         }
//     } else { 
//         if (value_.has_value()) {
//             return tuple_.error();  // пробрасываем информацию об ошибках дальше
//         } else {
//             value_.error().aggregate(tuple_.error);
//             return value_.error();
//         }
//     }
    
// }

std::size_t parse_values_counter;

// Функция для парсинга значения с учетом спецификатора формата
template <typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    ++parse_values_counter;
    T t;
    return t;
    // if (!is_type_matches_placeholder<T>(input)) {
    //     return scan_error();
    // }
}

// template<>
// std::expected<std::tuple<Head, Ts>, std::string> operator+() {

// }



template<typename T>
bool verify_matching(std::vector<std::string_view>& fmt, std::size_t& counter, std::string& error_str) {
    if (!is_type_matches_placeholder<T>(fmt.at(++counter))) {
        error_str = "Плейсхолдер с порядковым номером {} несоответствует указанному среди шаблонных аргументов типу";
         // Несоответствие между типом аргумента "; // (%s) и плейсхолдером {%s}", typeid(T).name(), fmt[counter]);
        return false;
    }
    return true;
}

// template<typename T>
// T add_verified_value(std::vector<std::string_view>& input, std::size_t& counter) {
//     return input[counter++];
// }

// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
template <typename... Ts>
std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;  // Части формата между {}
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней }
    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{input_parts, format_parts};
}

} // namespace stdx::details