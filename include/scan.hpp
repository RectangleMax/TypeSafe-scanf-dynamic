#pragma once

#include "parse.hpp"
#include "types.hpp"

#include <expected>
#include <iostream>


namespace stdx {

template<typename Head, typename... Tail>
std::expected<std::tuple<Head, Tail...>, details::scan_error> 
operator+(std::expected<std::tuple<Tail...>, details::scan_error> tail_tuple, std::expected<Head, details::scan_error> head_value) {
    // return std::tuple_cat(std::make_tuple(head_value.value()), tail_tuple.value()); // продолжаем собирать результаты парсинга в кортеж
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
            return std::unexpected(tail_tuple.error());  // если несоответствие встречается не впервые, то объединяем этот факт с информацией о предыдущих ошибках
        }
    }   
}
    

// замените болванку функции scan на рабочую версию
template <typename... Ts> 
std::expected<details::scan_result<Ts...>, details::scan_error>
scan(std::string_view input, std::string_view format)  {
    
    auto args_and_placeholders = details::parse_sources(input, format);
    std::size_t i = args_and_placeholders->first.size();
    
    std::expected<std::tuple<>, details::scan_error> x0;  // начинаем с пустого кортежа
    auto result_tuple = (x0 + ... + (--i, details::parse_value_with_format<Ts>(
                                        args_and_placeholders->first[i],
                                        args_and_placeholders->second[i])));

    // if ((!details::verify_matching<Ts>(parsed_parts->second, counter, error_str) || ...)) {
    //     // std::cout << error_str;
    //     // ruturn std::unexpected
    //     return false;
    // }
    
    return details::scan_result(result_tuple.value());
    // int aaa = 1;
    
}

} // namespace stdx
