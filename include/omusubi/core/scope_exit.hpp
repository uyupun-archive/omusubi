#pragma once

/**
 * @file scope_exit.hpp
 * @brief スコープ終了時に自動実行されるRAIIヘルパー
 *
 * C++23の std::scope_exit をC++17で実装したもの。
 * スコープを抜ける際に指定した関数を自動的に実行する。
 *
 * @note std::experimental::scope_exit 互換
 */

#include <type_traits>

namespace omusubi {

/**
 * @brief スコープ終了時にコールバックを実行するRAIIクラス
 *
 * @tparam Func 実行するコールバックの型
 *
 * @par 使用例
 * @code
 * void example() {
 *     auto* resource = acquire_resource();
 *     scope_exit cleanup([&] { release_resource(resource); });
 *
 *     // 例外が発生しても、早期リターンしても、
 *     // スコープを抜ける際に必ず release_resource が呼ばれる
 *
 *     if (some_condition) {
 *         cleanup.release();  // 条件によってはクリーンアップをスキップ
 *         return;
 *     }
 *
 *     do_something(resource);
 * }
 * @endcode
 */
template <typename Func>
class scope_exit {
public:
    /**
     * @brief コンストラクタ
     * @param func スコープ終了時に実行するコールバック
     */
    explicit scope_exit(Func func) noexcept(std::is_nothrow_move_constructible_v<Func>) : func_(static_cast<Func&&>(func)), active_(true) {}

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元のscope_exit
     */
    scope_exit(scope_exit&& other) noexcept(std::is_nothrow_move_constructible_v<Func>) : func_(static_cast<Func&&>(other.func_)), active_(other.active_) { other.release(); }

    // コピー禁止
    scope_exit(const scope_exit&) = delete;
    scope_exit& operator=(const scope_exit&) = delete;
    scope_exit& operator=(scope_exit&&) = delete;

    /**
     * @brief デストラクタ
     *
     * active_ が true の場合、保持しているコールバックを実行する。
     */
    ~scope_exit() noexcept {
        if (active_) {
            func_();
        }
    }

    /**
     * @brief コールバックの実行をキャンセル
     *
     * この関数を呼び出すと、デストラクタでコールバックが実行されなくなる。
     */
    void release() noexcept { active_ = false; }

private:
    Func func_;   ///< 実行するコールバック
    bool active_; ///< コールバックを実行するかどうか
};

// C++17 クラステンプレート引数推論ガイド
template <typename Func>
scope_exit(Func) -> scope_exit<Func>;

/**
 * @brief scope_exit を生成するヘルパー関数
 *
 * @tparam Func コールバックの型（自動推論）
 * @param func スコープ終了時に実行するコールバック
 * @return scope_exit<Func> 生成されたscope_exitオブジェクト
 *
 * @par 使用例
 * @code
 * auto cleanup = make_scope_exit([&] { close_file(fd); });
 * @endcode
 *
 * @note C++17のクラステンプレート引数推論（CTAD）により、
 *       scope_exit guard([&] { ... }); と直接記述することも可能。
 */
template <typename Func>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) - プロジェクト方針によりstd::forward未使用
[[nodiscard]] auto make_scope_exit(Func&& func) noexcept(std::is_nothrow_constructible_v<std::decay_t<Func>, Func>) -> scope_exit<std::decay_t<Func>> {
    return scope_exit<std::decay_t<Func>>(static_cast<Func&&>(func));
}

} // namespace omusubi
