#include <gtest/gtest.h>
#include <sodium.h>
#include "Hashing.hpp"

// Тестовый класс для Hashing
class HashingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Инициализация libsodium
        if (sodium_init() == -1) {
            FAIL() << "Не удалось инициализировать libsodium";
        }
    }
};

// Тест успешного хеширования пароля
TEST_F(HashingTest, PwHashMake_Success) {
    Hashing hasher;
    std::string password = "testpassword123";
    std::string hashedPassword;

    ConfiguratorErrorCode result = hasher.pwHashMake(password, hashedPassword);

    EXPECT_EQ(result, ConfiguratorErrorCode::SUCCESS);
    EXPECT_FALSE(hashedPassword.empty());
    // Проверка, что хешированный пароль начинается с ожидаемого префикса (формат libsodium)
    EXPECT_TRUE(hashedPassword.find("$argon2") != std::string::npos);
}

// Тест хеширования пустого пароля
TEST_F(HashingTest, PwHashMake_EmptyPassword) {
    Hashing hasher;
    std::string password = "";
    std::string hashedPassword;

    ConfiguratorErrorCode result = hasher.pwHashMake(password, hashedPassword);

    EXPECT_EQ(result, ConfiguratorErrorCode::SUCCESS);
    EXPECT_FALSE(hashedPassword.empty());
}

// Тест успешной проверки пароля
TEST_F(HashingTest, PwHashVerify_Success) {
    Hashing hasher;
    std::string password = "testpassword123";
    std::string hashedPassword;

    // Сначала создаем хеш
    ConfiguratorErrorCode hashResult = hasher.pwHashMake(password, hashedPassword);
    ASSERT_EQ(hashResult, ConfiguratorErrorCode::SUCCESS);

    // Проверяем хеш
    ConfiguratorErrorCode verifyResult = hasher.pwHashVerify(password, hashedPassword);

    EXPECT_EQ(verifyResult, ConfiguratorErrorCode::SUCCESS);
}

// Тест проверки неверного пароля
TEST_F(HashingTest, PwHashVerify_WrongPassword) {
    Hashing hasher;
    std::string password = "testpassword123";
    std::string wrongPassword = "wrongpassword";
    std::string hashedPassword;

    // Создаем хеш для правильного пароля
    ConfiguratorErrorCode hashResult = hasher.pwHashMake(password, hashedPassword);
    ASSERT_EQ(hashResult, ConfiguratorErrorCode::SUCCESS);

    // Проверяем с неверным паролем
    ConfiguratorErrorCode verifyResult = hasher.pwHashVerify(wrongPassword, hashedPassword);

    EXPECT_EQ(verifyResult, ConfiguratorErrorCode::PASSWORDS_DONT_MATCH);
}

// Тест проверки с некорректным хешем
TEST_F(HashingTest, PwHashVerify_InvalidHash) {
    Hashing hasher;
    std::string password = "testpassword123";
    std::string invalidHashedPassword = "invalidhash";

    ConfiguratorErrorCode verifyResult = hasher.pwHashVerify(password, invalidHashedPassword);

    EXPECT_EQ(verifyResult, ConfiguratorErrorCode::PASSWORDS_DONT_MATCH);
}

// Тест проверки с пустым паролем и хешем
TEST_F(HashingTest, PwHashVerify_EmptyPasswordAndHash) {
    Hashing hasher;
    std::string password = "";
    std::string hashedPassword;

    // Создаем хеш для пустого пароля
    ConfiguratorErrorCode hashResult = hasher.pwHashMake(password, hashedPassword);
    ASSERT_EQ(hashResult, ConfiguratorErrorCode::SUCCESS);

    // Проверяем пустой пароль
    ConfiguratorErrorCode verifyResult = hasher.pwHashVerify(password, hashedPassword);

    EXPECT_EQ(verifyResult, ConfiguratorErrorCode::SUCCESS);
}