#include <gtest/gtest.h>
#include <print>

#include "../include/scan.hpp"

std::string format_1 = "I want to sum {%d} and {%f} numbers.";
std::string input_1  = "I want to sum 42 and 3.14 numbers.";

// 1
TEST(ScanTest, SimpleTest) {
    auto result = stdx::scan<std::string>("number", "{}");
    ASSERT_TRUE(result);
}

// 2
TEST(ScanTest, ToMuchTypes) {
    auto result = stdx::scan<int, double, double>(input_1, format_1);
    auto it = result.error().errors_map.find(stdx::details::scan_error::ERROR::LACK_OF_ARGS);
    ASSERT_TRUE(it != result.error().errors_map.end());
}

// 3
TEST(ScanTest, LackOfTypes) {
    auto result = stdx::scan<int>("Это строка (абв), целое (-1) и натуральное (1) числа.",
                                  "Это строка ({%s}), целое ({%d}) и натуральное ({%u}) числа.");
    auto it = result.error().errors_map.find(stdx::details::scan_error::ERROR::LACK_OF_TYPES);
    ASSERT_TRUE(it != result.error().errors_map.end());
}

// 4
TEST(ScanTest, BasicSuccessEmptyPlaceholders) {
    auto result = stdx::scan<int, double>("42 3.14", "{} {}");
    ASSERT_TRUE(result.has_value());
    
    auto value = result.value();
    EXPECT_EQ(value.get_data_value<0>(), 42);
    EXPECT_DOUBLE_EQ(value.get_data_value<1>(), 3.14);
}

// 5 Тест на успешное сканирование с разными числовыми типами
TEST(ScanTest, DifferentNumericTypes) {
    auto result = stdx::scan<int8_t, uint16_t, int32_t, uint64_t, float, double>(
        "-10 20 -300 400000 3.14 2.718", 
        "{} {} {} {} {} {}"
    );
    ASSERT_TRUE(result.has_value());
    
    auto value = result.value();
    EXPECT_EQ(value.get_data_value<0>(), -10);
    EXPECT_EQ(value.get_data_value<1>(), 20);
    EXPECT_EQ(value.get_data_value<2>(), -300);
    EXPECT_EQ(value.get_data_value<3>(), 400000);
    EXPECT_FLOAT_EQ(value.get_data_value<4>(), 3.14f);
    EXPECT_DOUBLE_EQ(value.get_data_value<5>(), 2.718);
}

// 6 Тест на смешанный случай с текстом и спецификаторами
TEST(ScanTest, MixedCaseWithSpecifiers) {
    auto result = stdx::scan<int, double, std::string>(
        "Count: 42, Price: 3.14, Name: Apple", 
        "Count: {%d}, Price: {%f}, Name: {%s}"
    );
    ASSERT_TRUE(result.has_value());
    
    auto value = result.value();
    EXPECT_EQ(value.get_data_value<0>(), 42);
    EXPECT_DOUBLE_EQ(value.get_data_value<1>(), 3.14);
    EXPECT_EQ(value.get_data_value<2>(), "Apple");
}

// указатель, ссылка, разные строки, std::from_chars_result