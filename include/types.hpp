#pragma once
#include <string>
#include <expected>
#include <deque>
#include <map>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования
struct scan_error {
    // std::string message;
    
    enum class ERROR {
        DIFF_UNFORMAT_TEXT,
        LACK_OF_ARGS,
        LACK_OF_TYPES,
        FORBIDDEN_TYPE,
        MISMATCH
        // ошибка ковертации
    };

    scan_error() = default;
    scan_error(const std::string& message, ERROR err = ERROR::DIFF_UNFORMAT_TEXT) {
        errors_map[err].push_back(message);
    }

    std::map< ERROR, std::deque<std::string> > errors_map;


    void unite_mismatching_errors(const scan_error& other) {
        auto it2 = other.errors_map.find(ERROR::MISMATCH);
        if (it2 != other.errors_map.end()) {
            errors_map[ERROR::MISMATCH].insert(errors_map[ERROR::MISMATCH].begin(), it2->second.begin(), it2->second.end());
        }
    }
};

// Шаблонный класс для хранения результатов успешного сканирования
template <typename... Ts>
struct scan_result {
    scan_result() = default;
    // scan_result(Ts... args) : data(args ...) {}

    // template<typename... Us>
    // scan_result(Us... args): data(make_reverse_tuple(args...)) {}

    scan_result(std::tuple<Ts...> tuple) : data(tuple) {}
 
    template<std::size_t index>
    auto value() { return std::get<index>(data); }

private:
    std::tuple<Ts...> data;

    template<typename Head, typename... Tail>
    std::tuple<Tail..., Head> make_reverse_tuple(Head head, Tail... tail) {
        return std::tuple_cat(std::make_tuple(head), make_reverse_tuple(tail...));
    }

    std::tuple<> make_reverse_tuple() { return std::tuple<>(); }
};


} // namespace stdx::details
