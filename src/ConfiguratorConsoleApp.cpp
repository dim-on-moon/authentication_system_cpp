// src/ConfiguratorConsoleApp.cpp

#include <iostream>
#include <string>
#include <limits>

#include "ConfiguratorConsoleApp.hpp"
#include "ConfiguratorDatabase.hpp"
#include "SecurityConfig.hpp"
#include "Hashing.hpp"
#include "AccountsEditor.hpp"


void ConfiguratorConsoleApp::printMenu() const
{
    std::cout << "\n=== Security Configuration and User Management ===\n";
    std::cout << "1. View current security settings\n";
    std::cout << "2. Set minimum password length\n";
    std::cout << "3. Set password history depth\n";
    std::cout << "4. Set password expiration days\n";
    std::cout << "5. Set maximum inactive time (minutes)\n";
    std::cout << "6. Set maximum failed login attempts\n";
    std::cout << "7. Set lockout time (minutes)\n";
    std::cout << "8. Add new user\n";
    std::cout << "9. Update user password\n";
    std::cout << "10. Update users roles\n";
    std::cout << "11. Remove user\n";
    std::cout << "12. List active users\n";
    std::cout << "13. List archive users\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter command (0-13): ";
}

// Преобразование ConfiguratorErrorCode в строку
std::string
ConfiguratorConsoleApp::errorCodeToString(ConfiguratorErrorCode code) const
{
    switch (code)
    {
    case ConfiguratorErrorCode::SUCCESS:
        return "Success";
    case ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS:
        return "Login already exists";
    case ConfiguratorErrorCode::LOGIN_NOT_FOUND:
        return "Login not found";
    case ConfiguratorErrorCode::PASSWORD_TOO_SHORT:
        return "Password too short";
    case ConfiguratorErrorCode::PASSWORD_INVALID_CHARS:
        return "Password contains invalid chars";
    case ConfiguratorErrorCode::PASSWORD_REUSED:
        return "Reused password";
    case ConfiguratorErrorCode::ROLE_NOT_FOUND:
        return "Role not found";
    case ConfiguratorErrorCode::DATABASE_ERROR:
        return "Database error";
    case ConfiguratorErrorCode::END_OF_TABLE:
        return "End of table";
    case ConfiguratorErrorCode::HASHING_ERROR:
        return "Hashing error";
    case ConfiguratorErrorCode::PASSWORDS_DONT_MATCH:
        return "Passwords don't match";

        default:
        return "Unknown error";
    }
}

// Вывод текущих настроек
void ConfiguratorConsoleApp::viewSettings() const
{
    unsigned val;
    ConfiguratorErrorCode code;

    std::cout << "\nCurrent Security Settings:\n";
    code = config->get_minPasswordLength(val);
    std::cout << "Minimum Password Length: " << (code == ConfiguratorErrorCode::SUCCESS ? std::to_string(val) : "N/A") << "\n";

    code = config->get_passwordHistoryDepth(val);
    std::cout << "Password History Depth: " << (code == ConfiguratorErrorCode::SUCCESS ? std::to_string(val) : "N/A") << "\n";

    code = config->get_passwordExpirationDays(val);
    std::cout << "Password Expiration Days: " << (code == ConfiguratorErrorCode::SUCCESS ? std::to_string(val) : "N/A") << "\n";

    code = config->get_maxInactiveTimeMin(val);
    std::cout << "Max Inactive Time (minutes): " << (code == ConfiguratorErrorCode::SUCCESS ? std::to_string(val) : "N/A") << "\n";

    code = config->get_maxFailedAttempts(val);
    std::cout << "Max Failed Attempts: " << (code == ConfiguratorErrorCode::SUCCESS ? std::to_string(val) : "N/A") << "\n";

    code = config->get_lockoutTimeMin(val);
    std::cout << "Lockout Time (minutes): " << (code == ConfiguratorErrorCode::SUCCESS ? std::to_string(val) : "N/A") << "\n";
}

// Установка значения с обработкой ввода
ConfiguratorErrorCode
ConfiguratorConsoleApp::setUnsignedConfigValue(const std::string &prompt, std::function<ConfiguratorErrorCode(unsigned)> setter)
{
    std::cout << prompt;
    unsigned value;
    if (!(std::cin >> value))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a positive number.\n";
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }
    return setter(value);
}

// Добавление нового пользователя
void ConfiguratorConsoleApp::addUser()
{
    std::string login, Password;
    std::cout << "Enter login: ";
    std::cin >> login;
    std::cout << "Enter password: ";
    std::cin >> Password;

    std::cout << "Enter roles (0 for ROLE1, 1 for ROLE2, 2 for ROLE3, 3 for ROLE4)"
              << "-1 for finish the input): ";
    int role;
    std::vector<UserRole> roles;
    while (std::cin >> role)
    {
        if (role == -1)
        {
            break;
        }
        roles.push_back(static_cast<UserRole>(role));
    }

    ConfiguratorErrorCode code = editor->createAccount(login, Password, roles);
    std::cout << "Result: " << errorCodeToString(code) << "\n";
}

// Обновление пароля пользователя
void ConfiguratorConsoleApp::updatePassword()
{
    std::string login, newPassword;

    std::cout << "Enter login: ";
    std::cin >> login;
    std::cout << "Enter new password: ";
    std::cin >> newPassword;

    ConfiguratorErrorCode code = editor->editPassword(login, newPassword);
    std::cout << "Result: " << errorCodeToString(code) << "\n";
}

// Обновление списка ролей пользователя
void ConfiguratorConsoleApp::updateRoles()
{
    std::string login;
    std::vector<UserRole> newRoles;

    std::cout << "Enter login: ";
    std::cin >> login;
    std::cout << "Enter roles (0 for ROLE1, 1 for ROLE2, 2 for ROLE3, 3 for ROLE4)"
              << "-1 for finish the input): ";
    int role;
    std::vector<UserRole> roles;
    while (std::cin >> role)
    {
        if (role == -1)
        {
            break;
        }
        roles.push_back(static_cast<UserRole>(role));
    }

    ConfiguratorErrorCode code = editor->editRoles(login, newRoles);
    std::cout << "Result: " << errorCodeToString(code) << "\n";
}

// Удаление пользователя
void ConfiguratorConsoleApp::removeUser()
{
    std::string login;
    std::cout << "Enter login to remove: ";
    std::cin >> login;

    ConfiguratorErrorCode code = editor->deleteAccount(login);
    std::cout << "Result: " << errorCodeToString(code) << "\n";
}

// Вывод списка активных пользователей
void ConfiguratorConsoleApp::listActiveUsers()
{
    std::string userData;
    ConfiguratorErrorCode code = db->getFirstActiveUser(userData);

    if (code != ConfiguratorErrorCode::SUCCESS)
    {
        std::cout << "Failed to get active users: " << errorCodeToString(code) << "\n";
        return;
    }

    std::cout << "\nActive Users:\n";
    do
    {
        std::cout << userData << "\n";
        code = db->getNextActiveUser(userData);
    } while (code == ConfiguratorErrorCode::SUCCESS);
}

// Вывод архива пользователей
void ConfiguratorConsoleApp::listArchiveUsers()
{
    std::string userData;
    ConfiguratorErrorCode code = db->getFirstArchiveUser(userData);

    if (code != ConfiguratorErrorCode::SUCCESS)
    {
        std::cout << "Failed to get archive users: " << errorCodeToString(code) << "\n";
        return;
    }

    std::cout << "\nArchive Users:\n";
    do
    {
        std::cout << userData << "\n";
        code = db->getNextArchiveUser(userData);
    } while (code == ConfiguratorErrorCode::SUCCESS);
}

ConfiguratorConsoleApp::ConfiguratorConsoleApp() : configPath("./configDb/config.txt"),
                                                   activeUsersPath("./configDb/active_users.txt"),
                                                   archivePath("./configDb/archive.txt"),
                                                   tmpPath("./configDb/tmp_file.txt")
{
    db = new ConfiguratorDatabase(archivePath, activeUsersPath, tmpPath);
    config = new SecurityConfig(configPath);
    hasher = new Hashing();
    editor = new ConfiguratorAccountsEditor(db, config, hasher);
}

void ConfiguratorConsoleApp::run()
{
    while (true)
    {
        printMenu();
        int command;
        if (!(std::cin >> command))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between 0 and 11.\n";
            continue;
        }

        if (command == 0)
        {
            std::cout << "Exiting program.\n";
            break;
        }

        switch (command)
        {
        case 1: // Просмотр настроек
            viewSettings();
            break;

        case 2: // Установить минимальную длину пароля
        {
            auto result = setUnsignedConfigValue("Enter minimum password length: ",
                                                 [this](unsigned val)
                                                 { return config->set_minPasswordLength(val); });
            std::cout << "Result: " << errorCodeToString(result) << "\n";
            break;
        }

        case 3: // Установить глубину истории паролей
        {
            auto result = setUnsignedConfigValue("Enter password history depth: ",
                                                 [this](unsigned val)
                                                 { return config->set_passwordHistoryDepth(val); });
            std::cout << "Result: " << errorCodeToString(result) << "\n";
            break;
        }

        case 4: // Установить срок действия пароля
        {
            auto result = setUnsignedConfigValue("Enter password expiration days: ",
                                                 [this](unsigned val)
                                                 { return config->set_passwordExpirationDays(val); });
            std::cout << "Result: " << errorCodeToString(result) << "\n";
            break;
        }

        case 5: // Установить максимальное время неактивности
        {
            auto result = setUnsignedConfigValue("Enter maximum inactive time (minutes): ",
                                                 [this](unsigned val)
                                                 { return config->set_maxInactiveTimeMin(val); });
            std::cout << "Result: " << errorCodeToString(result) << "\n";
            break;
        }

        case 6: // Установить максимальное число неудачных попыток
        {
            auto result = setUnsignedConfigValue("Enter maximum failed attempts: ",
                                                 [this](unsigned val)
                                                 { return config->set_maxFailedAttempts(val); });
            std::cout << "Result: " << errorCodeToString(result) << "\n";
            break;
        }

        case 7: // Установить время блокировки
        {
            auto result = setUnsignedConfigValue("Enter lockout time (minutes): ",
                                                 [this](unsigned val)
                                                 { return config->set_lockoutTimeMin(val); });
            std::cout << "Result: " << errorCodeToString(result) << "\n";
            break;
        }

        case 8: // Добавить пользователя
            addUser();
            break;

        case 9: // Обновить пароль
            updatePassword();
            break;

        case 10: // Обновить роли
            updateRoles();
            break;

        case 11: // Удалить пользователя
            removeUser();
            break;

        case 12: // Список активных пользователей
            listActiveUsers();
            break;

        case 13: // Список активных пользователей
            listArchiveUsers();
            break;

        default:
            std::cout << "Invalid command. Please enter a number between 0 and 13.\n";
            break;
        }
    }
}

ConfiguratorConsoleApp::~ConfiguratorConsoleApp()
{
    delete db;
    delete config;
    delete editor;
}