#pragma once
#include <string>
#include <expected>
#include <deque>
#include <map>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования
struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования
template <typename... Ts>
struct scan_result {
    scan_result() = default;
    scan_result(std::tuple<Ts...> tuple) : data(tuple) {}
 
    template<std::size_t index>
    auto get_data_value() { return std::get<index>(data); }

    template<std::size_t index>
    using get_data_type = typename std::tuple_element<index, std::tuple<Ts...>>::type;

// private:
    std::tuple<Ts...> data;
};

} // namespace stdx::details
