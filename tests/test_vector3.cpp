// Vector3 の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/types.h>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("Vector3 - 基本機能") {
    SUBCASE("デフォルトコンストラクタ") {
        Vector3 v1;
        CHECK(doctest::Approx(v1.x).epsilon(0.001) == 0.0f);
        CHECK(doctest::Approx(v1.y).epsilon(0.001) == 0.0f);
        CHECK(doctest::Approx(v1.z).epsilon(0.001) == 0.0f);
    }

    SUBCASE("値指定コンストラクタ") {
        Vector3 v2(1.0f, 2.0f, 3.0f);
        CHECK(doctest::Approx(v2.x).epsilon(0.001) == 1.0f);
        CHECK(doctest::Approx(v2.y).epsilon(0.001) == 2.0f);
        CHECK(doctest::Approx(v2.z).epsilon(0.001) == 3.0f);
    }
}

TEST_CASE("Vector3 - constexpr対応") {
    constexpr Vector3 v1;
    constexpr Vector3 v2(1.0f, 2.0f, 3.0f);

    CHECK(doctest::Approx(v1.x).epsilon(0.001) == 0.0f);
    CHECK(doctest::Approx(v2.x).epsilon(0.001) == 1.0f);
    CHECK(doctest::Approx(v2.y).epsilon(0.001) == 2.0f);
    CHECK(doctest::Approx(v2.z).epsilon(0.001) == 3.0f);
}

TEST_CASE("Vector3 - 代入操作") {
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2;

    SUBCASE("コピー代入") {
        v2 = v1;
        CHECK(doctest::Approx(v2.x).epsilon(0.001) == 1.0f);
        CHECK(doctest::Approx(v2.y).epsilon(0.001) == 2.0f);
        CHECK(doctest::Approx(v2.z).epsilon(0.001) == 3.0f);
    }

    SUBCASE("個別の値変更") {
        v2.x = 10.0f;
        v2.y = 20.0f;
        v2.z = 30.0f;
        CHECK(doctest::Approx(v2.x).epsilon(0.001) == 10.0f);
        CHECK(doctest::Approx(v2.y).epsilon(0.001) == 20.0f);
        CHECK(doctest::Approx(v2.z).epsilon(0.001) == 30.0f);
    }
}

TEST_CASE("Vector3 - センサーデータのユースケース") {
    SUBCASE("加速度センサーのデータ") {
        Vector3 accel(0.0f, 0.0f, 9.81f);
        CHECK(doctest::Approx(accel.z).epsilon(0.01) == 9.81f);
    }

    SUBCASE("ジャイロセンサーのデータ") {
        Vector3 gyro(0.1f, -0.2f, 0.05f);
        CHECK(doctest::Approx(gyro.x).epsilon(0.001) == 0.1f);
        CHECK(doctest::Approx(gyro.y).epsilon(0.001) == -0.2f);
    }

    SUBCASE("磁気センサーのデータ") {
        Vector3 mag(25.5f, -10.3f, 40.2f);
        CHECK(doctest::Approx(mag.x).epsilon(0.01) == 25.5f);
    }
}

TEST_CASE("Vector3 - 負の値のテスト") {
    Vector3 v(-1.0f, -2.0f, -3.0f);
    CHECK(doctest::Approx(v.x).epsilon(0.001) == -1.0f);
    CHECK(doctest::Approx(v.y).epsilon(0.001) == -2.0f);
    CHECK(doctest::Approx(v.z).epsilon(0.001) == -3.0f);
}

TEST_CASE("Vector3 - ゼロと極小値") {
    SUBCASE("ゼロベクトル") {
        Vector3 v1(0.0f, 0.0f, 0.0f);
        CHECK(doctest::Approx(v1.x).epsilon(0.001) == 0.0f);
        CHECK(doctest::Approx(v1.y).epsilon(0.001) == 0.0f);
        CHECK(doctest::Approx(v1.z).epsilon(0.001) == 0.0f);
    }

    SUBCASE("極小値") {
        Vector3 v2(0.001f, 0.002f, 0.003f);
        CHECK(doctest::Approx(v2.x).epsilon(0.0001) == 0.001f);
        CHECK(doctest::Approx(v2.y).epsilon(0.0001) == 0.002f);
        CHECK(doctest::Approx(v2.z).epsilon(0.0001) == 0.003f);
    }
}
