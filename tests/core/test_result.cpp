// Result<T, E> のユニットテスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/result.hpp>

#include "../doctest.h"

using namespace omusubi;

// テスト用の構造体
struct TestStruct {
    int value;

    constexpr TestStruct(int v) : value(v) {}

    constexpr bool operator==(const TestStruct& other) const { return value == other.value; }
};

// カスタムエラー型
enum class CustomError : uint8_t { ERROR_A = 1, ERROR_B = 2, ERROR_C = 3 };

// ========================================
// Result::ok() ファクトリメソッド
// ========================================

TEST_CASE("Result<T, E> - ok() ファクトリメソッド") {
    SUBCASE("基本的な成功値の作成") {
        auto result = Result<int>::ok(42);
        CHECK(result.is_ok());
        CHECK_FALSE(result.is_err());
        CHECK_EQ(result.value(), 42);
    }

    SUBCASE("構造体での成功値作成") {
        auto result = Result<TestStruct>::ok(TestStruct(100));
        CHECK(result.is_ok());
        CHECK_EQ(result.value().value, 100);
    }
}

// ========================================
// Result::err() ファクトリメソッド
// ========================================

TEST_CASE("Result<T, E> - err() ファクトリメソッド") {
    SUBCASE("基本的なエラー値の作成") {
        auto result = Result<int>::err(Error::NOT_CONNECTED);
        CHECK(result.is_err());
        CHECK_FALSE(result.is_ok());
        CHECK_EQ(static_cast<uint8_t>(result.error()), static_cast<uint8_t>(Error::NOT_CONNECTED));
    }

    SUBCASE("カスタムエラー型") {
        auto result = Result<int, CustomError>::err(CustomError::ERROR_B);
        CHECK(result.is_err());
        CHECK_EQ(static_cast<uint8_t>(result.error()), static_cast<uint8_t>(CustomError::ERROR_B));
    }
}

// ========================================
// コピーコンストラクタ
// ========================================

TEST_CASE("Result<T, E> - コピーコンストラクタ") {
    SUBCASE("成功値のコピー") {
        auto result1 = Result<int>::ok(42);
        auto result2(result1);
        CHECK(result2.is_ok());
        CHECK_EQ(result2.value(), 42);
    }

    SUBCASE("エラー値のコピー") {
        auto result1 = Result<int>::err(Error::TIMEOUT);
        auto result2(result1);
        CHECK(result2.is_err());
        CHECK_EQ(static_cast<uint8_t>(result2.error()), static_cast<uint8_t>(Error::TIMEOUT));
    }
}

// ========================================
// ムーブコンストラクタ
// ========================================

TEST_CASE("Result<T, E> - ムーブコンストラクタ") {
    SUBCASE("成功値のムーブ") {
        auto result1 = Result<int>::ok(42);
        auto result2(static_cast<Result<int>&&>(result1));
        CHECK(result2.is_ok());
        CHECK_EQ(result2.value(), 42);
    }

    SUBCASE("エラー値のムーブ") {
        auto result1 = Result<int>::err(Error::BUFFER_FULL);
        auto result2(static_cast<Result<int>&&>(result1));
        CHECK(result2.is_err());
        CHECK_EQ(static_cast<uint8_t>(result2.error()), static_cast<uint8_t>(Error::BUFFER_FULL));
    }
}

// ========================================
// コピー代入演算子
// ========================================

TEST_CASE("Result<T, E> - コピー代入演算子") {
    SUBCASE("成功→成功") {
        auto result1 = Result<int>::ok(42);
        auto result2 = Result<int>::ok(10);
        result2 = result1;
        CHECK(result2.is_ok());
        CHECK_EQ(result2.value(), 42);
    }

    SUBCASE("エラー→成功") {
        auto result1 = Result<int>::err(Error::INVALID_PARAMETER);
        auto result2 = Result<int>::ok(10);
        result2 = result1;
        CHECK(result2.is_err());
        CHECK_EQ(static_cast<uint8_t>(result2.error()), static_cast<uint8_t>(Error::INVALID_PARAMETER));
    }

    SUBCASE("成功→エラー") {
        auto result1 = Result<int>::ok(99);
        auto result2 = Result<int>::err(Error::TIMEOUT);
        result2 = result1;
        CHECK(result2.is_ok());
        CHECK_EQ(result2.value(), 99);
    }

    SUBCASE("エラー→エラー") {
        auto result1 = Result<int>::err(Error::READ_FAILED);
        auto result2 = Result<int>::err(Error::WRITE_FAILED);
        result2 = result1;
        CHECK(result2.is_err());
        CHECK_EQ(static_cast<uint8_t>(result2.error()), static_cast<uint8_t>(Error::READ_FAILED));
    }
}

// ========================================
// ムーブ代入演算子
// ========================================

TEST_CASE("Result<T, E> - ムーブ代入演算子") {
    SUBCASE("成功値のムーブ代入") {
        auto result1 = Result<int>::ok(42);
        auto result2 = Result<int>::ok(10);
        result2 = static_cast<Result<int>&&>(result1);
        CHECK(result2.is_ok());
        CHECK_EQ(result2.value(), 42);
    }

    SUBCASE("エラー値のムーブ代入") {
        auto result1 = Result<int>::err(Error::FILE_NOT_FOUND);
        auto result2 = Result<int>::ok(10);
        result2 = static_cast<Result<int>&&>(result1);
        CHECK(result2.is_err());
        CHECK_EQ(static_cast<uint8_t>(result2.error()), static_cast<uint8_t>(Error::FILE_NOT_FOUND));
    }
}

// ========================================
// value_or() メソッド
// ========================================

TEST_CASE("Result<T, E> - value_or() メソッド") {
    SUBCASE("成功時は元の値を返す") {
        auto result = Result<int>::ok(42);
        CHECK_EQ(result.value_or(0), 42);
    }

    SUBCASE("エラー時はデフォルト値を返す") {
        auto result = Result<int>::err(Error::UNKNOWN);
        CHECK_EQ(result.value_or(99), 99);
    }
}

// ========================================
// bool変換演算子
// ========================================

TEST_CASE("Result<T, E> - bool変換演算子") {
    SUBCASE("成功時はtrue") {
        auto result = Result<int>::ok(42);
        CHECK(static_cast<bool>(result));
    }

    SUBCASE("エラー時はfalse") {
        auto result = Result<int>::err(Error::TIMEOUT);
        CHECK_FALSE(static_cast<bool>(result));
    }
}

// ========================================
// operator* (間接参照)
// ========================================

TEST_CASE("Result<T, E> - operator* (間接参照)") {
    SUBCASE("非const参照") {
        auto result = Result<int>::ok(42);
        CHECK_EQ(*result, 42);
    }

    SUBCASE("const参照") {
        const auto result = Result<int>::ok(42);
        CHECK_EQ(*result, 42);
    }
}

// ========================================
// operator-> (ポインタ風アクセス)
// ========================================

TEST_CASE("Result<T, E> - operator-> (ポインタ風アクセス)") {
    SUBCASE("非const参照") {
        auto result = Result<TestStruct>::ok(TestStruct(42));
        CHECK_EQ(result->value, 42);
    }

    SUBCASE("const参照") {
        const auto result = Result<TestStruct>::ok(TestStruct(42));
        CHECK_EQ(result->value, 42);
    }
}

// ========================================
// エラー型のカスタマイズ
// ========================================

TEST_CASE("Result<T, E> - カスタムエラー型") {
    auto ok_result = Result<int, CustomError>::ok(100);
    auto err_result = Result<int, CustomError>::err(CustomError::ERROR_C);

    CHECK(ok_result.is_ok());
    CHECK_EQ(ok_result.value(), 100);
    CHECK(err_result.is_err());
    CHECK_EQ(static_cast<uint8_t>(err_result.error()), static_cast<uint8_t>(CustomError::ERROR_C));
}

// ========================================
// 複雑な型でのテスト
// ========================================

TEST_CASE("Result<T, E> - 構造体を値とする") {
    auto result = Result<TestStruct>::ok(TestStruct(999));
    CHECK(result.is_ok());
    CHECK_EQ(result.value().value, 999);
    CHECK_EQ(result->value, 999);
}
