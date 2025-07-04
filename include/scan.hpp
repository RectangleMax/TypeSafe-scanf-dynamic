#pragma once
#include "parse.hpp"
#include "types.hpp"
#include <expected>


namespace stdx {

// Перегрузка "+" для разворачивания шаблонных параметров, завёрнутых в variadic template
template<typename Head, typename... Tail>
std::expected<std::tuple<Head, Tail...>, details::scan_error> 
operator+(std::expected<std::tuple<Tail...>, details::scan_error> tail_tuple, std::expected<Head, details::scan_error> head_value) {
    if (tail_tuple.has_value()) {
        if (head_value.has_value()) { 
            return std::tuple_cat(std::make_tuple(head_value.value()), tail_tuple.value()); // продолжаем собирать результаты парсинга в кортеж
        } else {
            return std::unexpected(head_value.error());  // если несоответствие плейсхолдера и типа встречается впервые 
        }
    } else { 
        if (head_value.has_value()) {
            return std::unexpected(tail_tuple.error());  // если сейчас ОК, однако несоответствие было раньше, то пробрасываем информацию об ошибках дальше
        } else {
            tail_tuple.error().unite_mismatching_errors(head_value.error());
            return std::unexpected(tail_tuple.error());  // если выявленное несоответствие не впервой, то объединяем этот факт с информацией о предыдущих ошибках
        }
    }   
}

// // Перегрузка "+" для оборачивания кортежа, полученного при разворачивании variadic template
// template<typename Head, typename... Tail>
// std::tuple<Head, Tail...> operator+(std::tuple<Tail...> tuple, Head head) {
//     return std::tuple_cat(std::make_tuple(head), tuple);
// }

// // Функция для оборачивания кортежа
// template<typename... Ts, std::size_t... Inds>
// auto reverse_tuple(std::tuple<Ts...> tuple, std::index_sequence<Inds...>) { 
//     std::tuple<> x0;  
//     return (x0 + ... + std::get<Inds>(tuple));
// }

// Рабочая версия функции scan
template <typename... Ts> 
std::expected<details::scan_result<Ts...>, details::scan_error>
scan(std::string_view input, std::string_view format)  {
    
    if constexpr (((std::is_reference_v<Ts> || std::is_pointer_v<Ts>) || ...)) {
        return std::unexpected(details::scan_error{"использование ссылочных типов и указателей в качестве шаблонных аргументов scan запрещено\n"});
    }

    auto args_and_placeholders = details::parse_sources(input, format);
    if (!args_and_placeholders.has_value()) {
        return std::unexpected(args_and_placeholders.error());
    }

    std::size_t num_of_args = args_and_placeholders->first.size();
    if (num_of_args < sizeof...(Ts)) {
        return std::unexpected(details::scan_error("Количество плейсхолдеров в переданной строке превышает количество шаблонных параметров функции scan"));
    } else if (num_of_args > sizeof...(Ts)) {
        return std::unexpected(details::scan_error("Количество шаблонных параметров функции scan превышает количество плейсхолдеров в переданной строке"));
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
            return std::forward_as_tuple(
                static_cast<Ts>(std::get<Is>(expected_tuple).value())...
            );
        }(std::index_sequence_for<Ts...>{})
    );

}

} // namespace stdx
