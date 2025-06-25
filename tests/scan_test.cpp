#include <gtest/gtest.h>
#include <print>

#include "../include/scan.hpp"

std::string format_1 = "I want to sum {%d} and {%f} numbers.";
std::string input_1  = "I want to sum 42 and 3.14 numbers.";

TEST(ScanTest, SimpleTest) {
    auto result = stdx::scan<std::string>("number", "{}");
    ASSERT_TRUE(result);
}

TEST(ScanTest, ToMuchTypes) {
    auto result = stdx::scan<int, double, double>(input_1, format_1);
    auto it = result.error().errors_map.find(stdx::details::scan_error::ERROR::LACK_OF_ARGS);
    ASSERT_TRUE(it != result.error().errors_map.end());
}

TEST(ScanTest, LackOfTypes) {
    auto result = stdx::scan<int>("Это строка (абв), целое (-1) и натуральное (1) числа.",
                                  "Это строка ({%s}), целое ({%d}) и натуральное ({%u}) числа.");
    auto it = result.error().errors_map.find(stdx::details::scan_error::ERROR::LACK_OF_TYPES);
    ASSERT_TRUE(it != result.error().errors_map.end());
}