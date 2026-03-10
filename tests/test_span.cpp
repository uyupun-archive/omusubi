// span<T> の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/span.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("span<T> - 基本機能") {
    SUBCASE("デフォルトコンストラクタ") {
        span<int> empty_span;
        CHECK(empty_span.empty());
        CHECK_EQ(empty_span.size(), 0U);
    }

    SUBCASE("配列からの構築") {
        int arr[] = {1, 2, 3, 4, 5};
        span<int> s(arr);
        CHECK_EQ(s.size(), 5U);
        CHECK_EQ(s[0], 1);
        CHECK_EQ(s[4], 5);
    }

    SUBCASE("ポインタと長さからの構築") {
        int arr[] = {1, 2, 3, 4, 5};
        span<int> s2(arr, 3);
        CHECK_EQ(s2.size(), 3U);
        CHECK_EQ(s2[2], 3);
    }
}

TEST_CASE("span<T> - イテレータ") {
    int arr[] = {10, 20, 30};
    span<int> s(arr);

    SUBCASE("範囲for文") {
        int sum = 0;
        for (int val : s) {
            sum += val;
        }
        CHECK_EQ(sum, 60);
    }

    SUBCASE("begin/end") {
        CHECK_EQ(*s.begin(), 10);
        CHECK_EQ(*(s.end() - 1), 30);
    }
}

TEST_CASE("span<T> - 部分span操作") {
    int arr[] = {1, 2, 3, 4, 5};
    span<int> s(arr);

    SUBCASE("subspan") {
        auto sub = s.subspan(1, 3);
        CHECK_EQ(sub.size(), 3U);
        CHECK_EQ(sub[0], 2);
        CHECK_EQ(sub[2], 4);
    }

    SUBCASE("first") {
        auto first = s.first(2);
        CHECK_EQ(first.size(), 2U);
        CHECK_EQ(first[0], 1);
    }

    SUBCASE("last") {
        auto last = s.last(2);
        CHECK_EQ(last.size(), 2U);
        CHECK_EQ(last[1], 5);
    }
}

TEST_CASE("span<T> - 型変換") {
    SUBCASE("as_bytes") {
        char str[] = "test";
        span<char> char_span(str, 4);

        auto bytes = as_bytes(char_span);
        CHECK_EQ(bytes.size(), 4U);
        CHECK_EQ(bytes[0], static_cast<uint8_t>('t'));
    }

    SUBCASE("as_chars") {
        uint8_t byte_arr[] = {65, 66, 67}; // 'A', 'B', 'C'
        span<uint8_t> byte_span(byte_arr);
        auto chars = as_chars(byte_span);
        CHECK_EQ(chars.size(), 3U);
        CHECK_EQ(chars[0], 'A');
    }
}

TEST_CASE("span<T> - const span") {
    const int arr[] = {1, 2, 3};
    span<const int> cs(arr);

    CHECK_EQ(cs.size(), 3U);
    CHECK_EQ(cs[1], 2);
    CHECK_FALSE(cs.empty());
}
