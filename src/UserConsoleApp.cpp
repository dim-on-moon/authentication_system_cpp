// src/UserConsoleApp.cpp

#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <ctime>

#include "UserConsoleApp.hpp"
#include "ConfiguratorDatabase.hpp"
#include "SecurityConfig.hpp"
#include "Hashing.hpp"

std::string UserConsoleApp::errorCodeToString(UserErrorCode code) const
{
    switch (code)
    {
    case UserErrorCode::SUCCESS:
        return "Success";
    case UserErrorCode::LOGIN_ENTERING_ERROR:
        return "Error entering login";
    case UserErrorCode::PASSWORD_ENTERING_ERROR:
        return "Error entering password";
    case UserErrorCode::LOGIN_NOT_EXISTS:
        return "There is no user with this username";
    case UserErrorCode::GETTING_DATA_FROM_DB_ERROR:
        return "Database error";
    case UserErrorCode::WRONG_PASSWORD:
        return "Incorrect password was entered.";
    case UserErrorCode::PASSWORD_HAS_EXPIRED:
        return "The password has expired";
    default:
        return "Unknown error";
    }
}

UserErrorCode UserConsoleApp::loginEntering(const std::string &prompt, std::string &login)
{
    std::cout << prompt << std::endl;
    if (!std::getline(std::cin, login) || login.empty())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return UserErrorCode::LOGIN_ENTERING_ERROR;
    }
    std::cout << std::endl;
    return UserErrorCode::SUCCESS;
}

UserErrorCode UserConsoleApp::loginVerification(const std::string &login, std::string &strUserData)
{
    ConfiguratorErrorCode code = db->getActiveUserByLogin(login, strUserData);
    switch (code)
    {
    case ConfiguratorErrorCode::SUCCESS:
        return UserErrorCode::SUCCESS;
    case ConfiguratorErrorCode::LOGIN_NOT_FOUND:
        return UserErrorCode::LOGIN_NOT_EXISTS;
    default:
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }
}

UserErrorCode UserConsoleApp::parseUserData(const std::string &strUserData)
{
    std::istringstream iss(strUserData);

    // Логин
    if (!(iss >> userData.login) || userData.login.empty())
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    // Хеш пароля
    if (!(iss >> userData.passwordHash) || userData.passwordHash.empty())
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    // Дата
    if (!(iss >> userData.date) || userData.date.empty())
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    // Роли
    std::string rolesString;
    if (!(iss >> rolesString) || rolesString.empty())
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    // Парсим роли через запятую
    std::istringstream rolesStream(rolesString);
    std::string roleStr;
    while (std::getline(rolesStream, roleStr, ','))
    {
        if (roleStr.empty())
        {
            return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
        }

        // Сконвертировать строку роли в число
        int roleNumber = 0;
        for (char ch : roleStr)
        {
            roleNumber = roleNumber * 10 + (ch - '0');
        }

        userData.roles.push_back(static_cast<UserRole>(roleNumber));
    }

    if (userData.roles.empty())
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    return UserErrorCode::SUCCESS;
}

UserErrorCode UserConsoleApp::passwordEntering(const std::string &prompt, std::string &password)
{
    std::cout << prompt << std::endl;
    termios oldt, newt;

    // Сохраняем старые настройки терминала
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Устанавливаем новые настройки терминала, чтобы вводимый пароль не отображался
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if (!std::getline(std::cin, password) || password.empty())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return UserErrorCode::PASSWORD_ENTERING_ERROR;
    }

    // Возвращаем старые настройки терминала
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    if (password.empty())
    {
        return UserErrorCode::PASSWORD_ENTERING_ERROR;
    }

    std::cout << std::endl;
    return UserErrorCode::SUCCESS;
}

UserErrorCode UserConsoleApp::passwordVerification()
{
    unsigned maxFailedAttempts;
    ConfiguratorErrorCode configuratorCode = config->get_maxFailedAttempts(maxFailedAttempts);
    if (configuratorCode != ConfiguratorErrorCode::SUCCESS)
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    unsigned currentFailedAttempts = 0;
    UserErrorCode code;
    while (currentFailedAttempts < maxFailedAttempts)
    {
        std::string password;
        std::string prompt;
        if (currentFailedAttempts == 0)
        {
            prompt = "Enter your password:";
        }
        else
        {
            prompt = "Wrong password, try again:";
        }
        code = passwordEntering(prompt, password);
        if (code != UserErrorCode::SUCCESS)
        {
            return code;
        }

        // Проверка правильности введенного пароля
        configuratorCode = hasher->pwHashVerify(password, userData.passwordHash);
        if (configuratorCode == ConfiguratorErrorCode::SUCCESS)
        {
            return UserErrorCode::SUCCESS;
        }
        else
        {
            // Если нет запросим еще раз
            currentFailedAttempts++;
        }
    }

    return UserErrorCode::WRONG_PASSWORD;
}

UserErrorCode UserConsoleApp::PasswordExpirationCheck()
{
    unsigned passwordExpirationDays;
    ConfiguratorErrorCode configuratorCode = config->get_passwordExpirationDays(passwordExpirationDays);
    if (configuratorCode != ConfiguratorErrorCode::SUCCESS)
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    int daysDifference;
    UserErrorCode code = differenceInDays(userData.date, daysDifference);
    if (code != UserErrorCode::SUCCESS)
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    if (daysDifference > passwordExpirationDays)
    {
        return UserErrorCode::PASSWORD_HAS_EXPIRED;
    }

    return UserErrorCode::SUCCESS;
}

// Проверка, високосный ли год
bool UserConsoleApp::isLeapYear(int year)
{
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

// Количество дней в каждом месяце
unsigned UserConsoleApp::getDaysInMonth(int month, int year)
{
    static const unsigned daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }
    return daysInMonth[month - 1];
}

// Парсинг даты
UserErrorCode UserConsoleApp::parseDate(const std::string &dateStr, unsigned &day, unsigned &month, unsigned &year)
{
    std::istringstream iss(dateStr);
    char dot1, dot2;

    if (!(iss >> day >> dot1 >> month >> dot2 >> year))
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }
    if (dot1 != '.' || dot2 != '.')
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }
    if (day < 1 || day > 31 || month < 1 || month > 12)
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }
    return UserErrorCode::SUCCESS;
}

// Преобразование даты в количество дней с начала отсчета
unsigned UserConsoleApp::daysFromEpoch(unsigned day, unsigned month, unsigned year)
{
    unsigned days = day;

    // Дни в прошедших месяцах текущего года
    for (unsigned m = 1; m < month; ++m)
    {
        days += getDaysInMonth(m, year);
    }

    // Дни в прошедших годах
    for (unsigned y = 0; y < year; ++y)
    {
        days += isLeapYear(y) ? 366 : 365;
    }

    return days;
}

// Разница в днях между двумя датами
UserErrorCode UserConsoleApp::differenceInDays(const std::string &userDateStr, int &difference)
{
    unsigned userDay, userMonth, userYear;
    UserErrorCode code = parseDate(userDateStr, userDay, userMonth, userYear);
    if (code != UserErrorCode::SUCCESS)
    {
        return code;
    }

    // Текущая дата
    std::time_t t = std::time(nullptr);
    std::tm *now = std::localtime(&t);
    int currentDay = now->tm_mday;
    int currentMonth = now->tm_mon + 1;
    int currentYear = now->tm_year + 1900;

    int userDays = daysFromEpoch(userDay, userMonth, userYear);
    int currentDays = daysFromEpoch(currentDay, currentMonth, currentYear);

    difference = currentDays - userDays;
    if (difference < 0)
    {
        return UserErrorCode::GETTING_DATA_FROM_DB_ERROR;
    }

    return UserErrorCode::SUCCESS;
}

UserConsoleApp::UserConsoleApp() : configPath("./configDb/config.txt"),
                                   activeUsersPath("./configDb/active_users.txt"),
                                   archivePath("./configDb/archive.txt"),
                                   tmpPath("./configDb/tmp_file.txt")
{
    db = new ConfiguratorDatabase(archivePath, activeUsersPath, tmpPath);
    config = new SecurityConfig(configPath);
    hasher = new Hashing();
}

void UserConsoleApp::run()
{
    // Ввод логина
    std::string login;
    UserErrorCode code = loginEntering("Enter your login: ", login);
    if (code != UserErrorCode::SUCCESS)
    {
        std::cout << errorCodeToString(code) << std::endl;
        return;
    }

    // Проверка наличия логина в базе пользователей
    std::string strUserData;
    code = loginVerification(login, strUserData);
    if (code != UserErrorCode::SUCCESS)
    {
        std::cout << errorCodeToString(code) << std::endl;
        return;
    }


    // Парсинг строки данных пользователя на логин, хешированный пароль,
    // дату изменения пароля и список ролей
    code = parseUserData(strUserData);
    if (code != UserErrorCode::SUCCESS)
    {
        std::cout << errorCodeToString(code) << std::endl;
        return;
    }

    // Ввод пароля и его проверка на соответствие заданному
    code = passwordVerification();
    if (code != UserErrorCode::SUCCESS)
    {
        std::cout << errorCodeToString(code) << std::endl;
        return;
    }

    // Проверка, не истек ли срок действия пароля
    code = PasswordExpirationCheck();
    if (code != UserErrorCode::SUCCESS)
    {
        std::cout << errorCodeToString(code) << std::endl;
        return;
    }

    // Дошли до этого места -> Доступ разрешен
    std::cout << "Access is allowed" << std::endl;
}

UserConsoleApp::~UserConsoleApp()
{
    delete db;
    delete config;
    delete hasher;
}