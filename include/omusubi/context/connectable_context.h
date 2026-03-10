#pragma once

#include <cstdint>

namespace omusubi {

// 前方宣言
class SerialContext;
class BluetoothContext;
class WiFiContext;
class BLEContext;

/**
 * @brief 接続可能なデバイスのコンテキスト
 */
class ConnectableContext {
public:
    ConnectableContext() = default;
    virtual ~ConnectableContext() = default;
    ConnectableContext(const ConnectableContext&) = delete;
    ConnectableContext& operator=(const ConnectableContext&) = delete;
    ConnectableContext(ConnectableContext&&) = delete;
    ConnectableContext& operator=(ConnectableContext&&) = delete;

    /** @brief シリアルポートコンテキストを取得（実行時） */
    [[nodiscard]] virtual SerialContext* get_serial_context(uint8_t port) const = 0;

    /** @brief シリアルポートコンテキストを取得（コンパイル時） */
    template <uint8_t Port>
    [[nodiscard]] SerialContext* get_serial_context() const {
        static_assert(Port <= 2, "Serial port must be 0, 1, or 2");
        return get_serial_context(Port);
    }

    /** @brief シリアルポート数を取得 */
    [[nodiscard]] virtual uint8_t get_serial_count() const = 0;

    /** @brief Bluetoothコンテキストを取得 */
    [[nodiscard]] virtual BluetoothContext* get_bluetooth_context() const = 0;

    /** @brief WiFiコンテキストを取得 */
    [[nodiscard]] virtual WiFiContext* get_wifi_context() const = 0;

    /** @brief BLEコンテキストを取得 */
    [[nodiscard]] virtual BLEContext* get_ble_context() const = 0;
};
} // namespace omusubi
