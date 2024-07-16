#pragma once

#include "../../test_runner.h"
#include "string_parses.h"

void TestReadNumberOnLine() {
    std::istringstream is("123\n");
    ASSERT_EQUAL(ReadNumberOnLine<int>(is), 123);
}

void TestSplitTwoStrict() {
    std::string_view s = "hello world";
    auto [lhs, rhs] = SplitTwoStrict(s, " ");
    ASSERT_EQUAL(lhs, "hello");
    ASSERT_EQUAL(*rhs, "world");
}

void TestSplitTwo() {
    std::string_view s = "hello";
    auto [lhs, rhs] = SplitTwo(s, " ");
    ASSERT_EQUAL(lhs, "hello");
    ASSERT_EQUAL(rhs, "");
}

void TestReadToken() {
    std::string_view s = "hello world";
    ASSERT_EQUAL(ReadToken(s, " "), "hello");
    ASSERT_EQUAL(s, "world");
}

void TestConvertToDouble() {
    std::string_view s = "3.14";
    ASSERT_EQUAL(ConvertToDouble(s), 3.14);
    s = "55.611087, 37.20829";
    const double latitude = ConvertToDouble(ReadToken(s, ", "));
    const double longitude = ConvertToDouble(s);
    ASSERT_EQUAL(latitude, 55.611087);
    ASSERT_EQUAL(longitude, 37.20829);
}

void TestAllStringParses() {
    TestRunner tr;
    RUN_TEST(tr, TestReadNumberOnLine);
    RUN_TEST(tr, TestSplitTwoStrict);
    RUN_TEST(tr, TestSplitTwo);
    RUN_TEST(tr, TestReadToken);
    RUN_TEST(tr, TestConvertToDouble);
}