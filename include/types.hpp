#pragma once
#include <string>
#include <expected>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования
struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования
template <typename... Ts>
struct scan_result {
    scan_result() = default;
    scan_result(Ts... args) : data(args ...) {}
    scan_result(std::tuple<Ts...> tutu) : data(tutu) {}

    

    template<std::size_t index>
    auto value() { return std::get<index>(data); }

private:
    std::tuple<Ts...> data;
};

} // namespace stdx::details
