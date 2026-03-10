// scope_exit の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/scope_exit.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("scope_exit - 基本機能") {
    SUBCASE("スコープ終了時にコールバックが実行される") {
        int counter = 0;
        {
            scope_exit guard([&counter] { ++counter; });
            CHECK_EQ(counter, 0); // まだ実行されていない
        }
        CHECK_EQ(counter, 1); // スコープを抜けて実行された
    }

    SUBCASE("複数のscope_exitは逆順に実行される") {
        int value = 0;
        {
            scope_exit guard1([&value] { value *= 2; });  // 後に実行
            scope_exit guard2([&value] { value += 10; }); // 先に実行
        }
        // guard2: value = 0 + 10 = 10
        // guard1: value = 10 * 2 = 20
        CHECK_EQ(value, 20);
    }
}

TEST_CASE("scope_exit - release()") {
    SUBCASE("release()を呼ぶとコールバックが実行されない") {
        int counter = 0;
        {
            scope_exit guard([&counter] { ++counter; });
            guard.release();
        }
        CHECK_EQ(counter, 0); // release()したので実行されない
    }

    SUBCASE("release()は何度呼んでも安全") {
        int counter = 0;
        {
            scope_exit guard([&counter] { ++counter; });
            guard.release();
            guard.release(); // 複数回呼んでも問題なし
            guard.release();
        }
        CHECK_EQ(counter, 0);
    }
}

TEST_CASE("scope_exit - ムーブ") {
    SUBCASE("ムーブ後は元のオブジェクトはinactive") {
        int counter = 0;

        // ラムダを変数に格納してから使用
        auto increment = [&counter] { ++counter; };
        using GuardType = scope_exit<decltype(increment)>;

        {
            GuardType guard1(increment);
            GuardType guard2(static_cast<GuardType&&>(guard1));
            // guard1はムーブされたのでinactive
        }
        // guard2のみが実行される（guard1はムーブ済みで実行されない）
        CHECK_EQ(counter, 1);
    }
}

TEST_CASE("scope_exit - make_scope_exit") {
    SUBCASE("ヘルパー関数でscope_exitを作成") {
        int counter = 0;
        {
            auto guard = make_scope_exit([&counter] { ++counter; });
            CHECK_EQ(counter, 0);
        }
        CHECK_EQ(counter, 1);
    }

    SUBCASE("[[nodiscard]]のため、戻り値を変数に格納する必要がある") {
        // make_scope_exit の戻り値を無視すると警告が出る
        // （このテストでは正しく使用している例を示す）
        int counter = 0;
        {
            [[maybe_unused]] auto guard = make_scope_exit([&counter] { ++counter; });
        }
        CHECK_EQ(counter, 1);
    }
}

TEST_CASE("scope_exit - CTAD (クラステンプレート引数推論)") {
    SUBCASE("テンプレート引数を省略して構築") {
        int counter = 0;
        {
            // C++17 CTAD: scope_exit<decltype(lambda)> を明示せずに構築
            scope_exit guard([&counter] { ++counter; });
            CHECK_EQ(counter, 0);
        }
        CHECK_EQ(counter, 1);
    }
}

TEST_CASE("scope_exit - 実用的な使用例") {
    SUBCASE("リソースのクリーンアップ") {
        bool resource_acquired = false;
        bool resource_released = false;

        {
            // リソース取得
            resource_acquired = true;

            // スコープ終了時に自動解放
            scope_exit cleanup([&] { resource_released = true; });

            CHECK(resource_acquired);
            CHECK_FALSE(resource_released);
        }

        CHECK(resource_acquired);
        CHECK(resource_released);
    }

    SUBCASE("条件付きクリーンアップ") {
        bool should_cleanup = true;
        bool cleaned_up = false;

        {
            scope_exit guard([&] { cleaned_up = true; });

            if (!should_cleanup) {
                guard.release();
            }
        }

        CHECK(cleaned_up); // should_cleanup == true なので実行される
    }

    SUBCASE("条件付きクリーンアップ - スキップ") {
        bool should_cleanup = false;
        bool cleaned_up = false;

        {
            scope_exit guard([&] { cleaned_up = true; });

            if (!should_cleanup) {
                guard.release();
            }
        }

        CHECK_FALSE(cleaned_up); // should_cleanup == false なので実行されない
    }
}
