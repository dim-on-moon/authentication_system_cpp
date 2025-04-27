// include/AccountsEditor.hpp

#include "AccountsEditorInterface.hpp"
#include "ConfiguratorDatabaseInterface.hpp"
#include "HashingInterface.hpp"
#include "SecurityConfigInterface.hpp"

#ifndef ACCOUNTS_EDITOR_HPP
#define ACCOUNTS_EDITOR_HPP

class ConfiguratorAccountsEditor : public ConfiguratorAccountsEditorInterface
{
    ConfiguratorDatabaseInterface *db;
    SecurityConfigInterface *config;
    HashingInterface *hasher;

private:
    // Проверка логина на существование в архиве
    ConfiguratorErrorCode checkLoginInArchive(const std::string &login);

    // Проверка логина на существование в таблице активных пользователей
    ConfiguratorErrorCode checkLoginInActive(const std::string &login);

    // Проверка символа из пароля на допустимость
    ConfiguratorErrorCode isValidChar(char c);

    // Проверка пароля на удовлетворение всем требованиям безопасности
    ConfiguratorErrorCode checkPassword(const std::string &login, const std::string &password);

public:
    ConfiguratorAccountsEditor(ConfiguratorDatabaseInterface *database, SecurityConfigInterface *security, HashingInterface *hash);

    // Добавление нового пользователя в БД
    ConfiguratorErrorCode createAccount(const std::string &login, const std::string &password, const std::vector<UserRole> &roles) override;

    // Удаление пользователя из БД
    ConfiguratorErrorCode deleteAccount(const std::string &login) override;

    // Изменение пароля от учетной записи
    ConfiguratorErrorCode editPassword(const std::string &login, const std::string &newPassword) override;

    // Изменение списка ролей пользователя
    ConfiguratorErrorCode editRoles(const std::string &login, const std::vector<UserRole> &newRoles) override;
};

#endif