#pragma once

#include <omusubi/core/types.h>

namespace omusubi {

/**
 * @brief 1次元センサーインターフェース
 */
class Measurable1D {
public:
    Measurable1D() = default;
    virtual ~Measurable1D() = default;
    Measurable1D(const Measurable1D&) = delete;
    Measurable1D& operator=(const Measurable1D&) = delete;
    Measurable1D(Measurable1D&&) = delete;
    Measurable1D& operator=(Measurable1D&&) = delete;

    /** @brief センサー値を取得 */
    [[nodiscard]] virtual float get_value() const = 0;
};

/**
 * @brief 3次元センサーインターフェース
 */
class Measurable3D {
public:
    Measurable3D() = default;
    virtual ~Measurable3D() = default;
    Measurable3D(const Measurable3D&) = delete;
    Measurable3D& operator=(const Measurable3D&) = delete;
    Measurable3D(Measurable3D&&) = delete;
    Measurable3D& operator=(Measurable3D&&) = delete;

    /** @brief X/Y/Z軸の値をまとめて取得 */
    [[nodiscard]] virtual Vector3 get_values() const = 0;
};

} // namespace omusubi
