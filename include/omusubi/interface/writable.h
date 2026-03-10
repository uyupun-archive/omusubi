#pragma once

#include <omusubi/core/span.hpp>

namespace omusubi {

/**
 * @brief バイト列書き込みインターフェース
 */
class ByteWritable {
public:
    ByteWritable() = default;
    virtual ~ByteWritable() = default;
    ByteWritable(const ByteWritable&) = delete;
    ByteWritable& operator=(const ByteWritable&) = delete;
    ByteWritable(ByteWritable&&) = delete;
    ByteWritable& operator=(ByteWritable&&) = delete;

    /** @brief バイト列を書き込む */
    virtual size_t write(span<const uint8_t> data) = 0;
};

/**
 * @brief テキスト書き込みインターフェース
 *
 * Java設計思想: java.io.Writerに相当
 * - ByteWritable（OutputStream相当）とは独立したインターフェース
 * - ISP（インターフェース分離原則）に従い、責務を分離
 */
class TextWritable {
public:
    TextWritable() = default;
    virtual ~TextWritable() = default;
    TextWritable(const TextWritable&) = delete;
    TextWritable& operator=(const TextWritable&) = delete;
    TextWritable(TextWritable&&) = delete;
    TextWritable& operator=(TextWritable&&) = delete;

    /** @brief テキストを書き込む */
    virtual size_t write_text(span<const char> text) = 0;
};

} // namespace omusubi
