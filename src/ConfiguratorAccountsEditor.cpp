// src/ConfiguratorAccountsEditor.cpp

#include "AccountsEditor.hpp"

// Конструктор класса
ConfiguratorAccountsEditor::ConfiguratorAccountsEditor(ConfiguratorDatabaseInterface *database, SecurityConfigInterface *security, HashingInterface *hash) : db(database), config(security), hasher(hash) {}

// Проверка логина на существование в архиве
ConfiguratorErrorCode ConfiguratorAccountsEditor::checkLoginInArchive(const std::string &login)
{
    std::string userData;
    ConfiguratorErrorCode errorCode = db->getArchiveUserByLogin(login, userData);

    switch (errorCode)
    {
    case ConfiguratorErrorCode::LOGIN_NOT_FOUND:
        return ConfiguratorErrorCode::LOGIN_NOT_FOUND;
    case ConfiguratorErrorCode::SUCCESS:
        return ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS;
    default:
        return errorCode; // может быть DATABASE_ERROR
    }
}

// Проверка логина на существование в таблице активных пользователей
ConfiguratorErrorCode ConfiguratorAccountsEditor::checkLoginInActive(const std::string &login)
{
    std::string userData;
    ConfiguratorErrorCode errorCode = db->getActiveUserByLogin(login, userData);

    switch (errorCode)
    {
    case ConfiguratorErrorCode::LOGIN_NOT_FOUND:
        return ConfiguratorErrorCode::LOGIN_NOT_FOUND;
    case ConfiguratorErrorCode::SUCCESS:
        return ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS;
    default:
        return errorCode; // может быть DATABASE_ERROR
    }
}

// Проверка символа из пароля на допустимость
ConfiguratorErrorCode ConfiguratorAccountsEditor::isValidChar(char c)
{
    // Проверяем, является ли символ цифрой или буквой
    if (std::isdigit(c) || std::isalpha(c))
    {
        return ConfiguratorErrorCode::SUCCESS;
    }

    // Проверяем, является ли символ одним из допустимых специальных символов
    switch (c)
    {
    case '@':
    case '#':
    case '%':
    case '(':
    case ')':
    case '*':
    case '_':
        return ConfiguratorErrorCode::SUCCESS;
    default:
        return ConfiguratorErrorCode::PASSWORD_INVALID_CHARS;
    }
}

// Проверка пароля на удовлетворение всем требованиям безопасности
ConfiguratorErrorCode ConfiguratorAccountsEditor::checkPassword(const std::string &login, const std::string &password)
{
    
    ConfiguratorErrorCode errorCode;
    std::string userData;
    
    // Пытаемся найти пользователя в архивной базе данных по логину
    errorCode = db->getArchiveUserByLogin(login, userData);
    if (errorCode == ConfiguratorErrorCode::SUCCESS)
    {
        // Если пользователь найден в архиве:
        // Удаляем логин, оставляя только строку с прошлыми паролями
        userData = userData.substr(userData.find(' ') + 1);

        // Разбиваем оставшуюся строку на отдельные старые пароли по пробелам
        std::vector<std::string> oldPasswords;
        while (userData.find(' ') != std::string::npos)
        {
            oldPasswords.push_back(userData.substr(0, userData.find(' ')));
            userData = userData.substr(userData.find(' ') + 1);
        }
        oldPasswords.push_back(userData); // Добавляем последний (или единственный) пароль

        // Проверяем, не совпадает ли хеш введённого пароля с одним из старых
        for (size_t i = 0; i < oldPasswords.size(); ++i)
        {
            if (hasher->pwHashVerify(password,oldPasswords[i]) == ConfiguratorErrorCode::SUCCESS)   
            {
                return ConfiguratorErrorCode::PASSWORD_REUSED;
            }
        }
    }
    else if (errorCode == ConfiguratorErrorCode::DATABASE_ERROR)
    {
        // Если произошла ошибка при доступе к архиву — возвращаем ошибку
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Получаем минимально допустимую длину пароля из конфигурации
    unsigned minPasswordLength;
    errorCode = config->get_minPasswordLength(minPasswordLength);
    if (errorCode == ConfiguratorErrorCode::DATABASE_ERROR)
    {
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Проверка длины пароля
    if (password.size() < minPasswordLength)
    {
        return ConfiguratorErrorCode::PASSWORD_TOO_SHORT;
    }

    // Проверка всех символов пароля на допустимость
    for (char c : password)
    {
        errorCode = isValidChar(c);
        if (errorCode == ConfiguratorErrorCode::PASSWORD_INVALID_CHARS)
        {
            return ConfiguratorErrorCode::PASSWORD_INVALID_CHARS;
        }
    }

    // Все проверки пройдены успешно
    return ConfiguratorErrorCode::SUCCESS;
}

// Добавление нового пользователя в БД
ConfiguratorErrorCode ConfiguratorAccountsEditor::createAccount(const std::string &login, const std::string &password, const std::vector<UserRole> &roles)
{
    // Проверка логина на уникальность (не должен присутствовать в архиве)
    ConfiguratorErrorCode errorCode;
    errorCode = checkLoginInArchive(login);

    // Если логин найден в архиве, считаем его уже использованным — возвращаем соответствующую ошибку
    if (errorCode != ConfiguratorErrorCode::LOGIN_NOT_FOUND)
    {
        return errorCode;
    }

    // Проверка пароля на соответствие требованиям безопасности
    errorCode = checkPassword(login, password);
    if (errorCode != ConfiguratorErrorCode::SUCCESS)
    {
        return errorCode;
    }

    // Хеширование пароля
    std::string hashedPassword;
    errorCode = hasher->pwHashMake(password, hashedPassword);
    if (errorCode != ConfiguratorErrorCode::SUCCESS)
    {
        return errorCode;
    }

    // Добавление нового пользователя в БД
    return db->addUser(login, hashedPassword, roles);
}

// Удаление пользователя из БД
ConfiguratorErrorCode ConfiguratorAccountsEditor::deleteAccount(const std::string &login)
{

    // Проверка: существует ли пользователь с таким логином в активной базе данных
    ConfiguratorErrorCode errorCode;
    errorCode = checkLoginInActive(login);

    // Если логин не найден или ошибка обращения к БД, то удаление невозможно
    if (errorCode != ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS)
    {
        return errorCode;
    }

    // Удаление пользователя из базы
    return db->removeUser(login);
}

// Изменение пароля от учетной записи
ConfiguratorErrorCode ConfiguratorAccountsEditor::editPassword(const std::string &login, const std::string &newPassword)
{
    // Проверка: существует ли активная учетная запись с таким логином
    ConfiguratorErrorCode errorCode;
    errorCode = checkLoginInActive(login);
    if (errorCode != ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS)
    {
        return errorCode;
    }

    // Проверка нового пароля на соответствие требованиям безопасности
    errorCode = checkPassword(login, newPassword);
    if (errorCode != ConfiguratorErrorCode::SUCCESS)
    {
        return errorCode;
    }

    // Получение зхначения глубины хранения паролей из конфигурационного файла
    unsigned historyDepth;
    errorCode = config->get_passwordHistoryDepth(historyDepth);
    if (errorCode != ConfiguratorErrorCode::SUCCESS)
    {
        return errorCode;
    }

    // Хеширование пароля
    std::string newHashedPassword;
    errorCode = hasher->pwHashMake(newPassword, newHashedPassword);
    if (errorCode != ConfiguratorErrorCode::SUCCESS)
    {
        return errorCode;
    }

    // Обновление пароля пользователя
    return db->updatePassword(login, newHashedPassword, historyDepth);
}

// Изменение списка ролей пользователя
ConfiguratorErrorCode ConfiguratorAccountsEditor::editRoles(const std::string &login, const std::vector<UserRole> &newRoles)
{
    // Проверка: существует ли активная учетная запись с заданным логином
    ConfiguratorErrorCode errorCode;
    errorCode = checkLoginInActive(login);
    if (errorCode != ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS)
    {
        return errorCode;
    }

    // Обновление ролей пользователя в базе данных
    return db->updateRoles(login, newRoles);
}