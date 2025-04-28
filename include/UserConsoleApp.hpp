// include/UserConsoleApp.hpp
#include <string>

#include "ErrorCode.hpp"
#include "ConfiguratorDatabaseInterface.hpp"
#include "SecurityConfigInterface.hpp"
#include "HashingInterface.hpp"
#include "AccountsEditorInterface.hpp"

#ifndef USER_CONSOLE_APP_HPP
#define USER_CONSOLE_APP_HPP

struct UserData
{
    std::string login;
    std::string passwordHash;
    std::string date;
    std::vector<UserRole> roles;
};

class UserConsoleApp
{
private:
    // Пути к файлам
    const std::string configPath;
    const std::string activeUsersPath;
    const std::string archivePath;
    const std::string tmpPath;

    ConfiguratorDatabaseInterface *db;
    SecurityConfigInterface *config;
    HashingInterface *hasher;

    UserData userData;

    std::string errorCodeToString(UserErrorCode code) const;

    UserErrorCode loginEntering(const std::string &prompt, std::string &login);
    UserErrorCode loginVerification(const std::string &login, std::string &userData);

    UserErrorCode parseUserData(const std::string &userData);

    UserErrorCode passwordEntering(const std::string &prompt, std::string &password);
    UserErrorCode passwordVerification();

    UserErrorCode PasswordExpirationCheck();
    bool isLeapYear(int year);
    unsigned getDaysInMonth(int month, int year);
    UserErrorCode parseDate(const std::string &dateStr, unsigned &day, unsigned &month, unsigned &year);
    unsigned daysFromEpoch(unsigned day, unsigned month, unsigned year);
    UserErrorCode differenceInDays(const std::string &userDateStr, int &difference);

public:
    UserConsoleApp();
    void run();
    ~UserConsoleApp();
};

#endif