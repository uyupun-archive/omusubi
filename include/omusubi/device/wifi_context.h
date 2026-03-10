#pragma once

#include <omusubi/interface/connectable.h>
#include <omusubi/interface/scannable.h>

namespace omusubi {

/**
 * @brief WiFi通信デバイス
 */
class WiFiContext : public Connectable, public Scannable {
public:
    WiFiContext() = default;
    ~WiFiContext() override = default;
    WiFiContext(const WiFiContext&) = delete;
    WiFiContext& operator=(const WiFiContext&) = delete;
    WiFiContext(WiFiContext&&) = delete;
    WiFiContext& operator=(WiFiContext&&) = delete;

    // Connectable interface
    [[nodiscard]] bool connect() override = 0;
    [[nodiscard]] bool disconnect() override = 0;
    [[nodiscard]] bool is_connected() const override = 0;

    // Scannable interface
    [[nodiscard]] bool start_scan() override = 0;
    [[nodiscard]] bool stop_scan() override = 0;
    [[nodiscard]] uint8_t get_found_count() const override = 0;
    [[nodiscard]] std::string_view get_found_name(uint8_t index) const override = 0;
    [[nodiscard]] int32_t get_found_signal_strength(uint8_t index) const override = 0;
};

} // namespace omusubi
