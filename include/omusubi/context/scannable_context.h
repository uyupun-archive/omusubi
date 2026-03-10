#pragma once

namespace omusubi {

// 前方宣言
class BluetoothContext;
class WiFiContext;
class BLEContext;

/**
 * @brief スキャン可能なデバイスのコンテキスト
 */
class ScannableContext {
public:
    ScannableContext() = default;
    virtual ~ScannableContext() = default;
    ScannableContext(const ScannableContext&) = delete;
    ScannableContext& operator=(const ScannableContext&) = delete;
    ScannableContext(ScannableContext&&) = delete;
    ScannableContext& operator=(ScannableContext&&) = delete;

    /** @brief Bluetoothコンテキストを取得 */
    [[nodiscard]] virtual BluetoothContext* get_bluetooth_context() const = 0;

    /** @brief WiFiコンテキストを取得 */
    [[nodiscard]] virtual WiFiContext* get_wifi_context() const = 0;

    /** @brief BLEコンテキストを取得 */
    [[nodiscard]] virtual BLEContext* get_ble_context() const = 0;
};

} // namespace omusubi
