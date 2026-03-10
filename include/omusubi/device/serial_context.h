#pragma once

#include <omusubi/interface/connectable.h>
#include <omusubi/interface/readable.h>
#include <omusubi/interface/writable.h>

namespace omusubi {

/**
 * @brief シリアル通信デバイス
 */
class SerialContext : public ByteReadable, public TextReadable, public ByteWritable, public TextWritable, public Connectable {
public:
    SerialContext() = default;
    ~SerialContext() override = default;
    SerialContext(const SerialContext&) = delete;
    SerialContext& operator=(const SerialContext&) = delete;
    SerialContext(SerialContext&&) = delete;
    SerialContext& operator=(SerialContext&&) = delete;

    // ByteReadable interface
    size_t read(span<uint8_t> buffer) override = 0;
    [[nodiscard]] size_t available() const override = 0;

    // TextReadable interface
    size_t read_line(span<char> buffer) override = 0;

    // ByteWritable interface
    size_t write(span<const uint8_t> data) override = 0;

    // TextWritable interface
    size_t write_text(span<const char> text) override = 0;

    // Connectable interface
    [[nodiscard]] bool connect() override = 0;
    [[nodiscard]] bool disconnect() override = 0;
    [[nodiscard]] bool is_connected() const override = 0;
};

} // namespace omusubi
