#pragma once

#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <typeinfo>
#include <cxxabi.h>
#include <charconv>
#include <format>

#include "types.hpp"



namespace stdx::details {

std::string demangle(const char* name) {
    int status = 0;
    char* demangled = abi::__cxa_demangle(name, 0, 0, &status);
    std::string result = (status == 0) ? demangled : name;
    std::free(demangled);
    return result;
}

template<typename T>
bool is_type_matches_placeholder(std::string_view placeholder) {
    if (placeholder.empty()) {
        return true;
    } else if (placeholder == "%d") {
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


// Функция для парсинга значения с учетом спецификатора формата
template <typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    if (!is_type_matches_placeholder<T>(fmt)) {
        return std::unexpected(scan_error(std::format("{} не соответствует типу {}.", fmt,  
                    demangle(typeid(T).name())), scan_error::ERROR::MISMATCH));
    }
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
        return std::string(input);
    } else { 
        std::remove_const_t<T> t;
        std::from_chars(input.data(), input.data() + input.size(), t);
        return t;
    }
}


// template<typename T>
// bool verify_matching(std::vector<std::string_view>& fmt, std::size_t& counter, std::string& error_str) {
//     if (!is_type_matches_placeholder<T>(fmt.at(++counter))) {
//         error_str = "Плейсхолдер с порядковым номером {} несоответствует указанному среди шаблонных аргументов типу";
//          // Несоответствие между типом аргумента "; // (%s) и плейсхолдером {%s}", typeid(T).name(), fmt[counter]);
//         return false;
//     }
//     return true;
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