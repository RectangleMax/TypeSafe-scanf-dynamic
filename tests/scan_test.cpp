#include <gtest/gtest.h>
#include <print>

#include "../include/scan.hpp"


// 0
TEST(ScanTest, SimpleTest) {
    auto result = stdx::scan<std::string>("number", "{}");
    ASSERT_TRUE(result);
}

// 1
TEST(ScanTest, ToMuchTypes) {
    auto result = stdx::scan<const std::string, int>("number", "{}");
    ASSERT_TRUE(!result.has_value());
    EXPECT_STREQ("количество шаблонных параметров функции scan превышает количество плейсхолдеров в переданной строке",
                 result.error().message.c_str());
}

// 2
TEST(ScanTest, FormatStrMismatchInput) {
    auto result = stdx::scan<int, double, double>("1 - это натуральное число.", "{%u} - это натуральное число. А это лшиний текст.");
    std::cout << result.error().message.c_str() << std::endl;
    EXPECT_STREQ("Unformatted text in input and format string are different", result.error().message.c_str());
}

// 3
TEST(ScanTest, LackOfTypes) {
    auto result = stdx::scan<std::string, int>("Это строка (абв), целое (-1) и натуральное (1) числа.",
                                  "Это строка ({%s}), целое ({%d}) и натуральное ({%u}) числа.");
    EXPECT_STREQ("количество плейсхолдеров в переданной строке превышает количество шаблонных параметров функции scan",
                 result.error().message.c_str());
}

// 4
TEST(ScanTest, ItsOK) {
    auto result = stdx::scan<std::string_view, int, uint8_t>("Это строка (абв), целое (-1) и натуральное (1) числа.",
                                  "Это строка ({%s}), целое ({%d}) и натуральное ({%u}) числа.");
    EXPECT_TRUE(result.has_value());
}

// 5
TEST(ScanTest, BigUint) {
    auto result = stdx::scan<std::string_view, int, uint8_t>("Это строка (абв), целое (-1) и натуральное (100000000000) числа.",
                                  "Это строка ({%s}), целое ({%d}) и натуральное ({%u}) числа.");
    EXPECT_TRUE(!result.has_value());
    EXPECT_STREQ("Numerical result out of range", result.error().message.c_str());
}

// 6
TEST(ScanTest, BasicSuccessEmptyPlaceholders) {
    auto result = stdx::scan<int, double>("42 3.14", "{} {}");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().get_data_value<0>(), 42);
    EXPECT_DOUBLE_EQ(result.value().get_data_value<1>(), 3.14);
}

// 7
TEST(ScanTest, PointerTest) {
    auto result = stdx::scan<int*, double>("42 3.14", "{} {}");
    ASSERT_TRUE(!result.has_value());
    EXPECT_STREQ(result.error().message.c_str(), "использование указателя в качестве шаблонного аргумента scan запрещено\n");
}

// 8
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

// 9
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

// указатель, разные строки, std::from_chars_result - большое число, и