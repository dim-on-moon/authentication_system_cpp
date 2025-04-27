// tests/test_ConfiguratorAccountsEditor.cpp

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "AccountsEditor.hpp"
#include "ConfiguratorDatabaseInterface.hpp"
#include "SecurityConfigInterface.hpp"
#include "HashingInterface.hpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgReferee;

class MockConfiguratorDatabase : public ConfiguratorDatabaseInterface
{
public:
    MOCK_METHOD(ConfiguratorErrorCode, getFirstActiveUser, (std::string & userData), (override));
    MOCK_METHOD(ConfiguratorErrorCode, getNextActiveUser, (std::string & userData), (override));
    MOCK_METHOD(ConfiguratorErrorCode, getActiveUserByLogin, (const std::string &login, std::string &userData), (override));

    MOCK_METHOD(ConfiguratorErrorCode, getFirstArchiveUser, (std::string & userData), (override));
    MOCK_METHOD(ConfiguratorErrorCode, getNextArchiveUser, (std::string & userData), (override));
    MOCK_METHOD(ConfiguratorErrorCode, getArchiveUserByLogin, (const std::string &login, std::string &userData), (override));

    MOCK_METHOD(ConfiguratorErrorCode, addUser, (const std::string &login, const std::string &hashedPassword, const std::vector<UserRole> &roles), (override));
    MOCK_METHOD(ConfiguratorErrorCode, removeUser, (const std::string &login), (override));
    MOCK_METHOD(ConfiguratorErrorCode, updatePassword, (const std::string &login, const std::string &newHashedPassword, const unsigned &passwordHistoryDepth), (override));
    MOCK_METHOD(ConfiguratorErrorCode, updateRoles, (const std::string &login, const std::vector<UserRole> &newRoles), (override));
};

class MockSecurityConfig : public SecurityConfigInterface
{
public:
    MOCK_METHOD(ConfiguratorErrorCode, set_minPasswordLength, (const unsigned length), (override));
    MOCK_METHOD(ConfiguratorErrorCode, set_passwordHistoryDepth, (const unsigned depth), (override));
    MOCK_METHOD(ConfiguratorErrorCode, set_passwordExpirationDays, (const unsigned days), (override));
    MOCK_METHOD(ConfiguratorErrorCode, set_maxInactiveTimeMin, (const unsigned minutes), (override));
    MOCK_METHOD(ConfiguratorErrorCode, set_maxFailedAttempts, (const unsigned attempts), (override));
    MOCK_METHOD(ConfiguratorErrorCode, set_lockoutTimeMin, (const unsigned minutes), (override));

    MOCK_METHOD(ConfiguratorErrorCode, get_minPasswordLength, (unsigned &length), (const, override));
    MOCK_METHOD(ConfiguratorErrorCode, get_passwordHistoryDepth, (unsigned &depth), (const, override));
    MOCK_METHOD(ConfiguratorErrorCode, get_passwordExpirationDays, (unsigned &days), (const, override));
    MOCK_METHOD(ConfiguratorErrorCode, get_maxInactiveTimeMin, (unsigned &minutes), (const, override));
    MOCK_METHOD(ConfiguratorErrorCode, get_maxFailedAttempts, (unsigned &attempts), (const, override));
    MOCK_METHOD(ConfiguratorErrorCode, get_lockoutTimeMin, (unsigned &minutes), (const, override));
};

class MockHashing : public HashingInterface
{
public:
    MOCK_METHOD(ConfiguratorErrorCode, pwHashMake, (const std::string &, std::string &), (override));
    MOCK_METHOD(ConfiguratorErrorCode, pwHashVerify, (const std::string &, const std::string &), (override));
};

class ConfiguratorAccountsEditorTest : public ::testing::Test
{
protected:
    // Создаем моки для базы данных и конфигурации безопасности
    MockConfiguratorDatabase mockDb;
    MockSecurityConfig mockConfig;
    MockHashing mockHasher;

    // Создаем объект класса, который будем тестировать
    ConfiguratorAccountsEditor accountsEditor{&mockDb, &mockConfig, &mockHasher};

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// Тесты для createAccount
TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_Success)
{
    std::string login = "new_user";
    std::string password = "ValidPass123";
    std::vector<UserRole> roles = {UserRole::ROLE1};

    // Два вызова getArchiveUserByLogin: в checkLoginInArchive и checkPassword
    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND))  // Для checkLoginInArchive
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND)); // Для checkPassword

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockDb, addUser(login, _, roles))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashMake(password, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::SUCCESS);
}

TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_LoginAlreadyExists)
{
    std::string login = "existing_user";
    std::string password = "ValidPass123";
    std::vector<UserRole> roles = {UserRole::ROLE1};

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS)); // Логин уже существует в архиве

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS);
}

TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_PasswordTooShort)
{
    std::string login = "new_user";
    std::string password = "Short";
    std::vector<UserRole> roles = {UserRole::ROLE1};

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::PASSWORD_TOO_SHORT);
}

TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_ValidSpecialPasswordChars)
{
    std::string login = "new_user";
    std::string password = "Valid@Pass123"; // Пароль с разрешенным символом @
    std::vector<UserRole> roles = {UserRole::ROLE1};

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockDb, addUser(login, _, roles))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashMake(password, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::SUCCESS);
}

TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_InvalidPasswordChars)
{
    std::string login = "new_user";
    std::string password = "Invalid$Pass";
    std::vector<UserRole> roles = {UserRole::ROLE1};

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::PASSWORD_INVALID_CHARS);
}

TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_DatabaseErrorOnArchiveCheck)
{
    std::string login = "new_user";
    std::string password = "ValidPass123";
    std::vector<UserRole> roles = {UserRole::ROLE1};

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_DatabaseErrorOnPasswordCheck)
{
    std::string login = "new_user";
    std::string password = "ValidPass123";
    std::vector<UserRole> roles = {UserRole::ROLE1};

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_DatabaseErrorOnAddUser)
{
    std::string login = "new_user";
    std::string password = "ValidPass123";
    std::vector<UserRole> roles = {UserRole::ROLE1};

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockDb, addUser(login, _, roles))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    EXPECT_CALL(mockHasher, pwHashMake(password, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, CreateAccount_PasswordHashingError)
{
    std::string login = "new_user";
    std::string password = "ValidPass123";
    std::vector<UserRole> roles = {UserRole::ROLE1};

    // Два вызова getArchiveUserByLogin: в checkLoginInArchive и checkPassword
    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND))  // Для checkLoginInArchive
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND)); // Для checkPassword

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockHasher, pwHashMake(password, _))
        .WillOnce(Return(ConfiguratorErrorCode::HASHING_ERROR));

    auto result = accountsEditor.createAccount(login, password, roles);
    EXPECT_EQ(result, ConfiguratorErrorCode::HASHING_ERROR);
}

// Тесты для deleteAccount
TEST_F(ConfiguratorAccountsEditorTest, DeleteAccount_Success)
{
    std::string login = "existing_login";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, removeUser(login))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    auto result = accountsEditor.deleteAccount(login);
    EXPECT_EQ(result, ConfiguratorErrorCode::SUCCESS);
}

TEST_F(ConfiguratorAccountsEditorTest, DeleteAccount_LoginNotFound)
{
    std::string login = "nonexisting_login";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND));

    EXPECT_CALL(mockDb, removeUser(login))
        .Times(0); // removeUser не должен вызываться

    auto result = accountsEditor.deleteAccount(login);
    EXPECT_EQ(result, ConfiguratorErrorCode::LOGIN_NOT_FOUND);
}

TEST_F(ConfiguratorAccountsEditorTest, DeleteAccount_DatabaseErrorOnLoginCheck)
{
    std::string login = "existing_login";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    EXPECT_CALL(mockDb, removeUser(login))
        .Times(0); // removeUser не должен вызываться

    auto result = accountsEditor.deleteAccount(login);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, DeleteAccount_DatabaseErrorOnRemove)
{
    std::string login = "existing_login";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, removeUser(login))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.deleteAccount(login);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

// Тесты для editPassword
TEST_F(ConfiguratorAccountsEditorTest, EditPassword_Success)
{
    std::string login = "existing_user";
    std::string newPassword = "NewPass123";
    unsigned passwordHistoryDepth = 3;

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashVerify(newPassword, _))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(ConfiguratorErrorCode::PASSWORDS_DONT_MATCH));

    EXPECT_CALL(mockHasher, pwHashMake(newPassword, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockConfig, get_passwordHistoryDepth(_))
        .WillOnce(DoAll(SetArgReferee<0>(passwordHistoryDepth), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockDb, updatePassword(login, _, passwordHistoryDepth))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::SUCCESS);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_LoginNotFound)
{
    std::string login = "nonexistent_user";
    std::string newPassword = "NewPass123";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::LOGIN_NOT_FOUND);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_PasswordTooShort)
{
    std::string login = "existing_user";
    std::string newPassword = "Short";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashVerify(newPassword, _))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(ConfiguratorErrorCode::PASSWORDS_DONT_MATCH));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::PASSWORD_TOO_SHORT);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_InvalidPasswordChars)
{
    std::string login = "existing_user";
    std::string newPassword = "Invalid$Pass";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashVerify(newPassword, _))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(ConfiguratorErrorCode::PASSWORDS_DONT_MATCH));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::PASSWORD_INVALID_CHARS);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_PasswordReused)
{
    std::string login = "existing_user";
    std::string newPassword = "ReusedPass123";
    std::string userData = login + " ReusedPass123 OldPass1";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(DoAll(SetArgReferee<1>(userData), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockHasher, pwHashVerify(newPassword, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::PASSWORD_REUSED);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_DatabaseErrorOnLoginCheck)
{
    std::string login = "existing_user";
    std::string newPassword = "NewPass123";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_DatabaseErrorOnPasswordCheck)
{
    std::string login = "existing_user";
    std::string newPassword = "NewPass123";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_DatabaseErrorOnGetMinPasswordLength)
{
    std::string login = "existing_user";
    std::string newPassword = "NewPass123";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashVerify(newPassword, _))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(ConfiguratorErrorCode::PASSWORDS_DONT_MATCH));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_DatabaseErrorOnHistoryDepth)
{
    std::string login = "existing_user";
    std::string newPassword = "NewPass123";

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashVerify(newPassword, _))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(ConfiguratorErrorCode::PASSWORDS_DONT_MATCH));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockConfig, get_passwordHistoryDepth(_))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_DatabaseErrorOnUpdate)
{
    std::string login = "existing_user";
    std::string newPassword = "NewPass123";
    unsigned passwordHistoryDepth = 3;

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashVerify(newPassword, _))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(ConfiguratorErrorCode::PASSWORDS_DONT_MATCH));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockConfig, get_passwordHistoryDepth(_))
        .WillOnce(DoAll(SetArgReferee<0>(passwordHistoryDepth), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockHasher, pwHashMake(newPassword, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, updatePassword(login, _, passwordHistoryDepth))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, EditPassword_PasswordHashingError)
{
    std::string login = "existing_user";
    std::string newPassword = "NewPass123";
    unsigned passwordHistoryDepth = 3;

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, getArchiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockHasher, pwHashVerify(newPassword, _))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(Return(ConfiguratorErrorCode::PASSWORDS_DONT_MATCH));

    EXPECT_CALL(mockConfig, get_minPasswordLength(_))
        .WillOnce(DoAll(SetArgReferee<0>(8), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockConfig, get_passwordHistoryDepth(_))
        .WillOnce(DoAll(SetArgReferee<0>(passwordHistoryDepth), Return(ConfiguratorErrorCode::SUCCESS)));

    EXPECT_CALL(mockHasher, pwHashMake(newPassword, _))
        .WillOnce(Return(ConfiguratorErrorCode::HASHING_ERROR));

    auto result = accountsEditor.editPassword(login, newPassword);
    EXPECT_EQ(result, ConfiguratorErrorCode::HASHING_ERROR);
}
// Тесты для editRoles
TEST_F(ConfiguratorAccountsEditorTest, EditRoles_Success)
{
    std::string login = "existing_user";
    std::vector<UserRole> newRoles = {UserRole::ROLE2};

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, updateRoles(login, newRoles))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    auto result = accountsEditor.editRoles(login, newRoles);
    EXPECT_EQ(result, ConfiguratorErrorCode::SUCCESS);
}

TEST_F(ConfiguratorAccountsEditorTest, EditRoles_LoginNotFound)
{
    std::string login = "nonexistent_user";
    std::vector<UserRole> newRoles = {UserRole::ROLE2};

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::LOGIN_NOT_FOUND));

    // updateRoles не должен вызываться
    EXPECT_CALL(mockDb, updateRoles(_, _)).Times(0);

    auto result = accountsEditor.editRoles(login, newRoles);
    EXPECT_EQ(result, ConfiguratorErrorCode::LOGIN_NOT_FOUND);
}

TEST_F(ConfiguratorAccountsEditorTest, EditRoles_DatabaseErrorOnLoginCheck)
{
    std::string login = "existing_user";
    std::vector<UserRole> newRoles = {UserRole::ROLE2};

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    // updateRoles не должен вызываться
    EXPECT_CALL(mockDb, updateRoles(_, _)).Times(0);

    auto result = accountsEditor.editRoles(login, newRoles);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}

TEST_F(ConfiguratorAccountsEditorTest, EditRoles_DatabaseErrorOnUpdate)
{
    std::string login = "existing_user";
    std::vector<UserRole> newRoles = {UserRole::ROLE2};

    EXPECT_CALL(mockDb, getActiveUserByLogin(login, _))
        .WillOnce(Return(ConfiguratorErrorCode::SUCCESS));

    EXPECT_CALL(mockDb, updateRoles(login, newRoles))
        .WillOnce(Return(ConfiguratorErrorCode::DATABASE_ERROR));

    auto result = accountsEditor.editRoles(login, newRoles);
    EXPECT_EQ(result, ConfiguratorErrorCode::DATABASE_ERROR);
}