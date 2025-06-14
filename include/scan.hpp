#pragma once

#include "parse.hpp"
#include "types.hpp"

#include <expected>
#include <iostream>


namespace stdx {

template<typename Head, typename... Tail>
std::expected<std::tuple<Head, Tail...>, details::scan_error> 
operator+(std::expected<std::tuple<Tail...>, details::scan_error> tuple_, std::expected<Head, details::scan_error> value_) {
    // tuple_ содержит либор кортеж, либо код ошибок
    return std::tuple_cat(std::make_tuple(value_.value()), tuple_.value()); // продолжаем собирать результаты парсинга в кортеж
    // if (tuple_.has_value()) {
    //     if (value_.has_value()) {
    //         return std::tuple_cat(std::make_tuple(value_), tuple_); // продолжаем собирать результаты парсинга в кортеж
    //     } else {
    //         return value_.error();  // если несоответствие плейсхолдера и типа встречается впервые 
    //     }
    // } else { 
    //     if (value_.has_value()) {
    //         return tuple_.error();  // пробрасываем информацию об ошибках дальше
    //     } else {
    //         value_.error().aggregate(tuple_.error);
    //         return value_.error();
    //     }
    // }   
}
    

// замените болванку функции scan на рабочую версию
template <typename... Ts> // std::expected<details::scan_result<Ts...>, details::scan_error>
bool scan(std::string_view input, std::string_view format)  {
    
    auto parsed_parts = details::parse_sources(input, format);

// для большего удобства создаём "вектор пар" из "пары векторов"
    // std::vector<std::pair<std::string, std::string>> value_format_vec;
    // value_format_vec.reserve(parsed_parts->first.size());
    // for (std::size_t i = 0; i < parsed_parts->first.size(); ++i) {
    //     value_format_vec.push_back(std::pair{parsed_parts->first[i], parsed_parts->second[i]});
    // }

    // checked expected
    // compare with size_of

    details::parse_values_counter = 0;
    std::string a;
    
    std::expected<std::tuple<>, details::scan_error> x0;  // начинаем с пустого кортежа
    auto result_tuple = (x0 + ... + details::parse_value_with_format<Ts>(
                                            parsed_parts->first[ details::parse_values_counter],
                                            parsed_parts->second[details::parse_values_counter]));

    // if ((!details::verify_matching<Ts>(parsed_parts->second, counter, error_str) || ...)) {
    //     // std::cout << error_str;
    //     // ruturn std::unexpected
    //     return false;
    // }
    return true;
    // int aaa = 1;
    
}

} // namespace stdx
