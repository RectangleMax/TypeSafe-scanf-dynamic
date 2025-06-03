#pragma once

#include "parse.hpp"
#include "types.hpp"

#include <expected>
#include <iostream>

namespace stdx {

// замените болванку функции scan на рабочую версию
template <typename... Ts>
bool // std::expected<details::scan_result<Ts...>, details::scan_error>
scan(std::string_view input, std::string_view format)  {
    
    auto parsed_parts = details::parse_sources(input, format);
    // checked expected
    // compare with size_of

    std::size_t counter = 0;
        
    std::expected<bool, std::string> verification_result = (!details::verify_matching<Ts>(parsed_parts->second, counter) || ...);
    if (!verification_result) {
        std::cout << verification_result.error();
    }
    return *verification_result;
    // int aaa = 1;
    



    // scan(std::string_view input, std::string_view format) {
    // return std::unexpected(details::scan_error{"Dumb implementation"});
}

} // namespace stdx
