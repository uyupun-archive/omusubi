#pragma once

#include <omusubi/core/types.h>

namespace omusubi {

/**
 * @brief ボタン入力インターフェース
 */
class Pressable {
public:
    Pressable() = default;
    virtual ~Pressable() = default;
    Pressable(const Pressable&) = delete;
    Pressable& operator=(const Pressable&) = delete;
    Pressable(Pressable&&) = delete;
    Pressable& operator=(Pressable&&) = delete;

    /** @brief ボタン状態を取得 */
    [[nodiscard]] virtual ButtonState get_state() const = 0;
};

} // namespace omusubi
