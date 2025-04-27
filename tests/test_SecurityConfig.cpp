// tests/test_SecurityConfig

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "SecurityConfig.hpp"
//#include "configurator.hpp"

class SecurityConfigTest : public ::testing::Test
{
protected:
    SecurityConfig *config;

    std::string testConfigPath = "./tests/files/config.txt";

    unsigned lockoutTimeMin = 30;
    unsigned maxFailedAttempts = 5;
    unsigned maxInactiveTimeMin = 10;
    unsigned minPasswordLength = 4;
    unsigned passwordExpirationDays = 30;
    unsigned passwordHistoryDepth = 3;

    void SetUp() override
    {
        std::ofstream(testConfigPath) << "lockoutTimeMin " << lockoutTimeMin << "\n"
                                      << "maxFailedAttempts " << maxFailedAttempts << "\n"
                                      << "maxInactiveTimeMin " << maxInactiveTimeMin << "\n"
                                      << "minPasswordLength " << minPasswordLength << "\n"
                                      << "passwordExpirationDays " << passwordExpirationDays << "\n"
                                      << "passwordHistoryDepth " << passwordHistoryDepth << "\n";

        config = new SecurityConfig(testConfigPath);
    }

    void TearDown() override
    {
        if (config != nullptr)
        {
            delete config;
            config = nullptr;
        }
        std::remove(testConfigPath.c_str());
    }
};

// Проверка корректной загрузки конфигурационных параметров из файла
TEST_F(SecurityConfigTest, LoadConfigValueCorrectly)
{
    unsigned val;
    EXPECT_EQ(config->get_lockoutTimeMin(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, lockoutTimeMin);

    EXPECT_EQ(config->get_maxFailedAttempts(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, maxFailedAttempts);

    EXPECT_EQ(config->get_maxInactiveTimeMin(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, maxInactiveTimeMin);   
    
    EXPECT_EQ(config->get_minPasswordLength(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, minPasswordLength); 

    EXPECT_EQ(config->get_passwordExpirationDays(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, passwordExpirationDays); 

    EXPECT_EQ(config->get_passwordHistoryDepth(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, passwordHistoryDepth); 
}

// Проверка корректности установки конфигурационных параметров в файл
TEST_F(SecurityConfigTest, SetValueAndReload) {
    lockoutTimeMin = 10;
    maxFailedAttempts = 10;
    maxInactiveTimeMin = 20;
    minPasswordLength = 20;
    passwordExpirationDays = 40;
    passwordHistoryDepth = 40;

    EXPECT_EQ(config->set_lockoutTimeMin(lockoutTimeMin), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(config->set_maxFailedAttempts(maxFailedAttempts), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(config->set_maxInactiveTimeMin(maxInactiveTimeMin), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(config->set_minPasswordLength(minPasswordLength), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(config->set_passwordExpirationDays(passwordExpirationDays), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(config->set_passwordHistoryDepth(passwordHistoryDepth), ConfiguratorErrorCode::SUCCESS);

    // Пересоздаем объект для проверки, что значение сохранилось
    SecurityConfig configReload(testConfigPath);
    unsigned val;

    EXPECT_EQ(configReload.get_lockoutTimeMin(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, lockoutTimeMin);

    EXPECT_EQ(configReload.get_maxFailedAttempts(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, maxFailedAttempts);

    EXPECT_EQ(configReload.get_maxInactiveTimeMin(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, maxInactiveTimeMin);

    EXPECT_EQ(configReload.get_minPasswordLength(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, minPasswordLength);

    EXPECT_EQ(configReload.get_passwordExpirationDays(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, passwordExpirationDays);

    EXPECT_EQ(configReload.get_passwordHistoryDepth(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(val, passwordHistoryDepth);
}

// Попытка получить конфигурационный параметр из пустого файла
TEST_F(SecurityConfigTest, LoadFromEmptyFile)
{
    std::ofstream(testConfigPath) << "";

    SecurityConfig emptyConfig(testConfigPath);

    unsigned val;
    EXPECT_NE(emptyConfig.get_minPasswordLength(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_NE(emptyConfig.get_lockoutTimeMin(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_NE(emptyConfig.get_maxFailedAttempts(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_NE(emptyConfig.get_maxInactiveTimeMin(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_NE(emptyConfig.get_passwordHistoryDepth(val), ConfiguratorErrorCode::SUCCESS);
    EXPECT_NE(emptyConfig.get_passwordExpirationDays(val), ConfiguratorErrorCode::SUCCESS);
}

// Обращение к несуществующему файлу должно вызывать ошибку
TEST(SecurityConfigStandaloneTest, MissingFileReturnsError) {
    SecurityConfig configMissing("non_existent.txt");

    unsigned val;
    EXPECT_NE(configMissing.get_minPasswordLength(val), ConfiguratorErrorCode::SUCCESS);
}

// Проверка загрузки из файла с некорректным форматом данных
TEST_F(SecurityConfigTest, LoadFromInvalidFormatFile)
{
    // Создаем файл с некорректным форматом (например, значение не является числом)
    std::ofstream(testConfigPath) << "minPasswordLength not_a_number\n";

    SecurityConfig invalidConfig(testConfigPath);

    unsigned val;
    // Ожидаем, что параметры не загрузятся, так как формат данных некорректен
    EXPECT_NE(invalidConfig.get_minPasswordLength(val), ConfiguratorErrorCode::SUCCESS);
}

// Проверка ошибки сохранения в файл (файл только для чтения)
TEST_F(SecurityConfigTest, SaveToFileNoWritePermission)
{
    // Делаем файл только для чтения
    std::filesystem::permissions(testConfigPath,
                                 std::filesystem::perms::owner_read | std::filesystem::perms::group_read | std::filesystem::perms::others_read,
                                 std::filesystem::perm_options::replace);

    // Пытаемся сохранить конфигурацию
    auto result = config->set_minPasswordLength(10);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);

    // Восстанавливаем права, чтобы TearDown мог удалить файл
    std::filesystem::permissions(testConfigPath,
                                 std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all,
                                 std::filesystem::perm_options::replace);
}

// Проверка загрузки из файла без прав чтения
TEST(SecurityConfigStandaloneTest, LoadFromFileNoReadPermission)
{
    std::string noReadPath = "./tests/files/no_read_config.txt";
    std::filesystem::create_directories("./tests/files");
    std::ofstream(noReadPath) << "minPasswordLength 4\n";

    // Снимаем права чтения
    std::filesystem::permissions(noReadPath,
                                 std::filesystem::perms::owner_write | std::filesystem::perms::group_write | std::filesystem::perms::others_write,
                                 std::filesystem::perm_options::replace);

    SecurityConfig noReadConfig(noReadPath);

    unsigned val;
    EXPECT_NE(noReadConfig.get_minPasswordLength(val), ConfiguratorErrorCode::SUCCESS);

    // Восстанавливаем права и удаляем файл
    std::filesystem::permissions(noReadPath,
                                 std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all,
                                 std::filesystem::perm_options::replace);
    std::filesystem::remove(noReadPath);
}

TEST_F(SecurityConfigTest, ConstructWithFileOpenError)
{
    // Создаём файл
    std::ofstream file(testConfigPath);
    file << "minPasswordLength 10\n";
    file.close();

    // Ограничиваем права доступа, чтобы файл нельзя было открыть
    std::filesystem::permissions(testConfigPath, std::filesystem::perms::owner_write | std::filesystem::perms::group_write | std::filesystem::perms::others_write);

    // Создаём объект (это вызовет loadFromConfigFile())
    SecurityConfig newConfig(testConfigPath);

    // Проверяем, что данные не загрузились из-за ошибки открытия файла
    unsigned length;
    ConfiguratorErrorCode result = newConfig.get_minPasswordLength(length);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
    EXPECT_EQ(length, 0); // Должно быть значение по умолчанию, так как data пустая

    // Восстанавливаем права доступа
    std::filesystem::permissions(testConfigPath, std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all);
}


