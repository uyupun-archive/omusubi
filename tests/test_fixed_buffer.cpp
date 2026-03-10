// FixedBuffer<N> の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/fixed_buffer.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("FixedBuffer<N> - 基本機能") {
    FixedBuffer<32> buf;
    CHECK_EQ(buf.size(), 0U);
    CHECK(buf.is_empty());
    CHECK_EQ(buf.capacity(), 32U);
}

TEST_CASE("FixedBuffer<N> - 追加操作") {
    FixedBuffer<16> buf;

    SUBCASE("1バイト追加") {
        CHECK(buf.append(0x42));
        CHECK_EQ(buf.size(), 1U);
        CHECK_EQ(buf[0], 0x42);
    }

    SUBCASE("複数バイト追加") {
        CHECK(buf.append(0x43));
        CHECK(buf.append(0x44));
        CHECK_EQ(buf.size(), 2U);
        CHECK_EQ(buf[1], 0x44);
    }
}

TEST_CASE("FixedBuffer<N> - 容量制限") {
    FixedBuffer<4> buf;

    SUBCASE("容量いっぱいまで追加") {
        CHECK(buf.append(0x01));
        CHECK(buf.append(0x02));
        CHECK(buf.append(0x03));
        CHECK(buf.append(0x04));
        CHECK_EQ(buf.size(), 4U);
    }

    SUBCASE("容量超過") {
        buf.append(0x01);
        buf.append(0x02);
        buf.append(0x03);
        buf.append(0x04);
        CHECK_FALSE(buf.append(0x05));
        CHECK_EQ(buf.size(), 4U);
    }
}

TEST_CASE("FixedBuffer<N> - クリア操作") {
    FixedBuffer<16> buf;
    buf.append(0x01);
    buf.append(0x02);
    buf.append(0x03);
    CHECK_EQ(buf.size(), 3U);

    buf.clear();
    CHECK_EQ(buf.size(), 0U);
    CHECK(buf.is_empty());

    SUBCASE("クリア後の再利用") {
        CHECK(buf.append(0xFF));
        CHECK_EQ(buf.size(), 1U);
    }
}

TEST_CASE("FixedBuffer<N> - アクセス操作") {
    FixedBuffer<8> buf;
    buf.append(0x10);
    buf.append(0x20);
    buf.append(0x30);

    SUBCASE("[]演算子") {
        CHECK_EQ(buf[0], 0x10);
        CHECK_EQ(buf[1], 0x20);
        CHECK_EQ(buf[2], 0x30);
    }

    SUBCASE("範囲外アクセス") {
        CHECK_EQ(buf[100], 0);
    }

    SUBCASE("data()") {
        const uint8_t* data = buf.data();
        CHECK_EQ(data[0], 0x10);
    }
}

TEST_CASE("FixedBuffer<N> - イテレータ") {
    FixedBuffer<8> buf;
    buf.append(0x01);
    buf.append(0x02);
    buf.append(0x03);

    SUBCASE("範囲for文") {
        int sum = 0;
        for (uint8_t val : buf) {
            sum += val;
        }
        CHECK_EQ(sum, 6);
    }

    SUBCASE("begin/end") {
        CHECK_EQ(*buf.begin(), 0x01);
        CHECK_EQ(buf.end() - buf.begin(), 3);
    }
}

TEST_CASE("FixedBuffer<N> - span変換") {
    FixedBuffer<16> buf;
    buf.append(0xAA);
    buf.append(0xBB);
    buf.append(0xCC);

    SUBCASE("as_span") {
        auto span_const = buf.as_span();
        CHECK_EQ(span_const.size(), 3U);
        CHECK_EQ(span_const[0], 0xAA);
    }

    SUBCASE("from_span") {
        FixedBuffer<16> buf2;
        uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
        span<const uint8_t> s(data);
        buf2.from_span(s);

        CHECK_EQ(buf2.size(), 4U);
        CHECK_EQ(buf2[0], 0x11);
        CHECK_EQ(buf2[3], 0x44);
    }
}

TEST_CASE("FixedBuffer<N> - from_spanの切り詰め") {
    FixedBuffer<4> buf;
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    span<const uint8_t> s(data);

    buf.from_span(s);
    CHECK_EQ(buf.size(), 4U);
    CHECK_EQ(buf[3], 0x04);
}
