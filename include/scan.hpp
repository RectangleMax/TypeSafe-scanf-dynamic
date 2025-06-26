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

// Перегрузка "+" для оборачивания кортежа, полученного при разворачивании variadic template
template<typename Head, typename... Tail>
std::tuple<Head, Tail...> operator+(std::tuple<Tail...> tuple, Head head) {
    return std::tuple_cat(std::make_tuple(head), tuple);
}

// Функция для оборачивания кортежа
template<typename... Ts, std::size_t... Inds>
auto reverse_tuple(std::tuple<Ts...> tuple, std::index_sequence<Inds...>) { 
    std::tuple<> x0;  
    return (x0 + ... + std::get<Inds>(tuple));
}

// Рабочая версия функции scan
template <typename... Ts> 
std::expected<details::scan_result<Ts...>, details::scan_error>
scan(std::string_view input, std::string_view format)  {
    
    auto args_and_placeholders = details::parse_sources(input, format);
    std::size_t i = args_and_placeholders->first.size();
    
    if (i < sizeof...(Ts)) {
        return std::unexpected(details::scan_error("Количество плейсхолдеров в переданной строке превышает количество шаблонных параметров функции scan",
                                 details::scan_error::ERROR::LACK_OF_ARGS));
    } else if (i > sizeof...(Ts)) {
        return std::unexpected(details::scan_error("Количество шаблонных параметров функции scan превышает количество плейсхолдеров в переданной строке",
                                 details::scan_error::ERROR::LACK_OF_TYPES));
    }
    

    std::expected<std::tuple<>, details::scan_error> x0;                        // начинаем с пустого кортежа
    auto result_tuple = (x0 + ... + (--i, details::parse_value_with_format<Ts>( // и разворачиваем аргуметны в обратном порядке
                                        args_and_placeholders->first[i],
                                        args_and_placeholders->second[i])));

    
    if (result_tuple.has_value()) {
        return reverse_tuple(result_tuple.value(), std::make_index_sequence<sizeof...(Ts)>{});
    } else {
        return std::unexpected(result_tuple.error());
    }
}

} // namespace stdx
