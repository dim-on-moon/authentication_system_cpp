// tests/test_ConfiguratorDatabase.cpp

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "ConfiguratorDatabase.hpp"

class ConfiguratorDatabaseTest : public ::testing::Test
{
protected:
    ConfiguratorDatabase *db;

    std::string testArchivePath;
    std::string testActiveUsersPath;
    std::string testTmpPath;

    void SetUp() override
    {
        testArchivePath = "./tests/files/test_archive.txt";
        testActiveUsersPath = "./tests/files/test_active_users.txt";
        testTmpPath = "./tests/files/test_tmp";

        // Подготовка тестовых файлов
        std::ofstream(testActiveUsersPath) << "user1 hashedpass1 01.01.2001 1\n"
                                           << "user2 hashedpass2 02.02.2002 2\n";
        std::ofstream(testArchivePath) << "user1 hashedpass1\n"
                                       << "user2 hashedpass2\n";

        db = new ConfiguratorDatabase(testArchivePath, testActiveUsersPath, testTmpPath);
    }

    void TearDown() override
    {
        std::remove(testArchivePath.c_str());
        std::remove(testActiveUsersPath.c_str());
        std::remove(testTmpPath.c_str());
        delete db;
    }
};

// Создание и удаление аккаунта с несуществующим в базе логином
TEST_F(ConfiguratorDatabaseTest, AddAndDeleteUser)
{
    ConfiguratorErrorCode code;
    std::string userData;

    code = db->addUser("testuser", "12345678", {UserRole::ROLE1});
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    code = db->getActiveUserByLogin("testuser", userData);
    EXPECT_TRUE(userData.rfind("testuser", 0) == 0);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    code = db->removeUser("testuser");
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    // После удаления пользователя нельзя получить
    code = db->getActiveUserByLogin("testuser", userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::LOGIN_NOT_FOUND);

    // Повторное удаление должно быть невозможно
    code = db->removeUser("testuser");
    EXPECT_EQ(code, ConfiguratorErrorCode::LOGIN_NOT_FOUND);
}

// Попытка создания аккаунта с существующим в базе логином
TEST_F(ConfiguratorDatabaseTest, AddUser_DuplicateLogin)
{
    ConfiguratorErrorCode code;

    code = db->addUser("user1", "newpass", {UserRole::ROLE2});
    EXPECT_EQ(code, ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS);
}

// Получение первого и второго активного пользователя
TEST_F(ConfiguratorDatabaseTest, GetFirstAndNextActiveUsers)
{
    std::string userData;
    ConfiguratorErrorCode code;

    code = db->getFirstActiveUser(userData);
    EXPECT_TRUE(userData.rfind("user1", 0) == 0);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    code = db->getNextActiveUser(userData);
    EXPECT_TRUE(userData.rfind("user2", 0) == 0);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    code = db->getNextActiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::END_OF_TABLE);
}

TEST_F(ConfiguratorDatabaseTest, GetFirstAndNextArchiveUsers)
{
    std::string userData;
    ConfiguratorErrorCode code;

    code = db->getFirstArchiveUser(userData);
    EXPECT_TRUE(userData.rfind("user1", 0) == 0);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    code = db->getNextArchiveUser(userData);
    EXPECT_TRUE(userData.rfind("user2", 0) == 0);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    code = db->getNextArchiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::END_OF_TABLE);
}

// Проверка корректности обновления пароля
TEST_F(ConfiguratorDatabaseTest, UpdatePasswordCorrectly)
{
    std::string userData;
    ConfiguratorErrorCode code;
    std::string login = "user1";
    std::string newPassword = "newhashed";
    unsigned historyDepth = 3;

    // Обновление пароля
    code = db->updatePassword(login, newPassword, historyDepth);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    // Проверка activeUsers.txt: пароль обновился
    std::ifstream activeFile(testActiveUsersPath);
    ASSERT_TRUE(activeFile.is_open());

    // Получение текущей даты
    std::time_t t = std::time(nullptr);
    std::tm *now = std::localtime(&t);
    std::string date = std::to_string(now->tm_mday) + "." + std::to_string(now->tm_mon + 1) + "." + std::to_string(now->tm_year + 1900);

    std::string line;
    bool found = false;
    while (std::getline(activeFile, line))
    {
        if (line.rfind(login, 0) == 0)
        {
            found = true;
            EXPECT_NE(line.find(newPassword), std::string::npos) << "Пароль в activeUsers.txt не обновился";
            EXPECT_NE(line.find(date), std::string::npos) << "Дата задания пароля не обновилась";
        }
    }
    EXPECT_TRUE(found) << "Пользователь не найден в activeUsers.txt";

    // Проверка archive.txt: пароль добавлен в историю
    std::ifstream archiveFile(testArchivePath);
    ASSERT_TRUE(archiveFile.is_open());

    found = false;
    while (std::getline(archiveFile, line))
    {
        if (line.rfind(login, 0) == 0)
        {
            found = true;
            EXPECT_NE(line.find(newPassword), std::string::npos) << "Пароль в archive.txt не обновился";
        }
    }
    EXPECT_TRUE(found) << "Пользователь не найден в archive.txt";
}

// Попытка обновить пароль у несуществующего пользователя
TEST_F(ConfiguratorDatabaseTest, UpdatePassword_NonExistanceUser)
{
    ConfiguratorErrorCode code;
    std::string login = "non-existance_user";
    std::string newPassword = "newhashed";
    unsigned historyDepth = 3;

    // Обновление пароля
    code = db->updatePassword(login, newPassword, historyDepth);
    EXPECT_EQ(code, ConfiguratorErrorCode::LOGIN_NOT_FOUND);
}

// Многократное обновление паролей, чтобы убедиться, что старые пароли стираются при превышении глубины хранения
TEST_F(ConfiguratorDatabaseTest, MultipleUpdatePasswordCorrectly)
{
    ConfiguratorErrorCode code;
    std::string login = "user1";
    std::string newPassword = "newHashedPass";
    std::string userData;
    std::string oldPassword;
    unsigned historyDepth = 3;

    code = db->getActiveUserByLogin(login, userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    // Вытаскиваем из строки текущий пароль пользователя
    size_t firstSpace = userData.find(' ');
    size_t secondSpace = userData.find(' ', firstSpace + 1);
    oldPassword = userData.substr(firstSpace + 1, secondSpace - firstSpace - 1);

    for (size_t i = 0; i < historyDepth; ++i)
    {
        newPassword += std::to_string(i);
        code = db->updatePassword(login, newPassword, historyDepth);
        EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);
    }

    // Проверяем, что первоначальный пароль больше не записан в архиве
    code = db->getArchiveUserByLogin(login, userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);
    EXPECT_EQ(userData.find(oldPassword), std::string::npos);
}

// Убедимся, что ошибка при открытии файлов возвращает корректный код ошибки
TEST_F(ConfiguratorDatabaseTest, FileOpenError)
{
    ConfiguratorDatabase dbWithWrongPath("./invalid_path/archive.txt", "./invalid_path/active_users.txt", "./invalid_path/tmp_file.txt");
    ConfiguratorErrorCode code;
    std::string userData;

    code = dbWithWrongPath.getFirstActiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::DATABASE_ERROR);

    code = dbWithWrongPath.addUser("testuser", "hashedpass", {UserRole::ROLE1});
    EXPECT_EQ(code, ConfiguratorErrorCode::DATABASE_ERROR);
}

// Проверка корректности обновления ролей
TEST_F(ConfiguratorDatabaseTest, UpdateRolesCorrectly)
{
    std::vector<UserRole> newRoles = {UserRole::ROLE1, UserRole::ROLE3};
    std::string login = "user1";
    ConfiguratorErrorCode code;
    std::string userData;

    // Записываем в строку новый список ролей для будущего сравнения
    std::string strRoles;
    for (size_t i = 0; i < newRoles.size() - 1; ++i)
    {
        strRoles += std::to_string(static_cast<int>(newRoles[i])) + ",";
    }
    strRoles += std::to_string(static_cast<int>(newRoles[newRoles.size() - 1]));

    code = db->updateRoles(login, newRoles);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);

    code = db->getActiveUserByLogin(login, userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);
    EXPECT_NE(userData.find(strRoles), std::string::npos);
}

// Попытка обновить роли у несуществующего пользователя
TEST_F(ConfiguratorDatabaseTest, UpdateRoles_NonExistanceUser)
{
    ConfiguratorErrorCode code;
    std::string login = "non-existance_user";
    std::vector<UserRole> newRoles = {UserRole::ROLE1, UserRole::ROLE3};

    // Обновление пароля
    code = db->updateRoles(login, newRoles);
    EXPECT_EQ(code, ConfiguratorErrorCode::LOGIN_NOT_FOUND);
}

// Проверка закрытия файла в getFirstActiveUser
TEST_F(ConfiguratorDatabaseTest, GetFirstActiveUser_CloseIfOpen)
{
    std::string userData;
    ConfiguratorErrorCode code;

    // Первый вызов открывает файл
    code = db->getFirstActiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);
    EXPECT_TRUE(userData.rfind("user1", 0) == 0);

    // Второй вызов должен закрыть и снова открыть файл
    userData.clear();
    code = db->getFirstActiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);
    EXPECT_TRUE(userData.rfind("user1", 0) == 0);
}

// Проверка закрытия файла в getFirstArchiveUser
TEST_F(ConfiguratorDatabaseTest, GetFirstArchiveUser_CloseIfOpen)
{
    std::string userData;
    ConfiguratorErrorCode code;

    // Первый вызов открывает файл
    code = db->getFirstArchiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);
    EXPECT_TRUE(userData.rfind("user1", 0) == 0);

    // Второй вызов должен закрыть и снова открыть файл
    userData.clear();
    code = db->getFirstArchiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::SUCCESS);
    EXPECT_TRUE(userData.rfind("user1", 0) == 0);
}

// Проверка ошибки открытия файла в getActiveUserByLogin
TEST_F(ConfiguratorDatabaseTest, GetActiveUserByLogin_FileOpenError)
{
    ConfiguratorDatabase dbWithWrongPath("./invalid_path/archive.txt", "./invalid_path/active_users.txt", "./invalid_path/tmp_file.txt");
    std::string userData;

    auto result = dbWithWrongPath.getActiveUserByLogin("user1", userData);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

// Проверка ошибки открытия файла в getArchiveUserByLogin
TEST_F(ConfiguratorDatabaseTest, GetArchiveUserByLogin_FileOpenError)
{
    ConfiguratorDatabase dbWithWrongPath("./invalid_path/archive.txt", "./invalid_path/active_users.txt", "./invalid_path/tmp_file.txt");
    std::string userData;

    auto result = dbWithWrongPath.getArchiveUserByLogin("user1", userData);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

// Проверка ошибки открытия архивного файла в addUser
TEST_F(ConfiguratorDatabaseTest, AddUser_ArchiveFileOpenError)
{
    // Удаляем файл testArchivePath, чтобы можно было создать директорию
    std::filesystem::remove(testArchivePath);

    // Делаем archivePath недоступным (например, создаем директорию с таким именем)
    std::filesystem::create_directory(testArchivePath);

    ConfiguratorDatabase localDb(testArchivePath, testActiveUsersPath, testTmpPath);
    auto result = localDb.addUser("testuser", "hashedpass", {UserRole::ROLE1});
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

// Проверка ошибки открытия временного файла в removeUser
TEST_F(ConfiguratorDatabaseTest, RemoveUser_TmpFileOpenError)
{
    // Делаем tmpPath недоступным
    std::filesystem::create_directory(testTmpPath);

    auto result = db->removeUser("user1");
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

// Проверка ошибки открытия временного файла в updatePassword (для active_users)
TEST_F(ConfiguratorDatabaseTest, UpdatePassword_TmpFileOpenError_ActiveUsers)
{
    std::string login = "user1";
    std::string newPassword = "newhashed";
    unsigned historyDepth = 3;

    // Делаем tmpPath недоступным
    std::filesystem::create_directory(testTmpPath);

    auto result = db->updatePassword(login, newPassword, historyDepth);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorDatabaseTest, getFirstActiveUser_EmptyFile)
{
    std::ofstream(testActiveUsersPath) << "";
    std::string userData;
    ConfiguratorErrorCode code;

    code = db->getFirstActiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::END_OF_TABLE);
}

TEST_F(ConfiguratorDatabaseTest, getFirstArchiveUser_EmptyFile)
{
    std::ofstream(testArchivePath) << "";
    std::string userData;
    ConfiguratorErrorCode code;

    code = db->getFirstArchiveUser(userData);
    EXPECT_EQ(code, ConfiguratorErrorCode::END_OF_TABLE);
}