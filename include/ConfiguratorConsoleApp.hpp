// /include/ConfiguratorConsoleApp.hpp

#include <functional>
#include <string>

//#include "iconfigurator.hpp"

#include "HashingInterface.hpp"
#include "ConfiguratorDatabaseInterface.hpp"
#include "SecurityConfigInterface.hpp"
#include "AccountsEditorInterface.hpp"

#ifndef CONFIGURATOR_CONSOLE_APP_HPP
#define CONFIGURATOR_CONSOLE_APP_HPP

class ConfiguratorConsoleApp
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
    ConfiguratorAccountsEditorInterface *editor;

    void printMenu() const;
    std::string errorCodeToString(ConfiguratorErrorCode code) const;
    void viewSettings() const;
    ConfiguratorErrorCode setUnsignedConfigValue(const std::string &prompt, std::function<ConfiguratorErrorCode(unsigned)> setter);
    void addUser();
    void updatePassword();
    void updateRoles();
    void removeUser();
    void listActiveUsers();
    void listArchiveUsers();

public:
    ConfiguratorConsoleApp();
    void run();
    ~ConfiguratorConsoleApp();
};

#endif