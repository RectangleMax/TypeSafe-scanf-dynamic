#pragma once
#include "parse.hpp"
#include "types.hpp"
#include <expected>


namespace stdx {

// Рабочая версия функции scan
template <typename... Ts> 
requires ((!std::is_reference_v<Ts>) && ...)
std::expected<details::scan_result<Ts...>, details::scan_error>
scan(std::string_view input, std::string_view format)  {

    auto args_and_placeholders = details::parse_sources(input, format);
    if (!args_and_placeholders.has_value()) {
        return std::unexpected(args_and_placeholders.error());
    }

    std::size_t num_of_args = args_and_placeholders->first.size();
    if (num_of_args > sizeof...(Ts)) {
        return std::unexpected(details::scan_error("количество плейсхолдеров в переданной строке превышает количество шаблонных параметров функции scan"));
    } else if (num_of_args < sizeof...(Ts)) {
        return std::unexpected(details::scan_error("количество шаблонных параметров функции scan превышает количество плейсхолдеров в переданной строке"));
    }
    
    auto expected_tuple = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::make_tuple(details::parse_value_with_format<Ts>(args_and_placeholders->first[Is], args_and_placeholders->second[Is])...);
    }(std::index_sequence_for<Ts...>{});
    
    std::string message;
    std::size_t errors_counter = 0;
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (([&] {
            if (!std::get<Is>(expected_tuple).has_value()) {
                message += std::get<Is>(expected_tuple).error().message;
                ++errors_counter;
            }
        }()), ...);
    }(std::index_sequence_for<Ts...>{});
    
    if (errors_counter != 0) {
        return std::unexpected(details::scan_error(message));
    }
    
    return details::scan_result<Ts...>(
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::make_tuple(
                static_cast<Ts>(std::get<Is>(expected_tuple).value())...
            );
        }(std::index_sequence_for<Ts...>{})
    );
}

} // namespace stdx
