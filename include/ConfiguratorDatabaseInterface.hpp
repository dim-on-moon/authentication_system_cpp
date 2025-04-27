// include/ConfiguratorDatabaseInterface.hpp

#include <string>
#include <vector>

#include "ErrorCode.hpp"
#include "UserRole.hpp"

#ifndef CONFIGURATOR_DATABASE_INTERFACE_HPP
#define CONFIGURATOR_DATABASE_INTERFACE_HPP

class ConfiguratorDatabaseInterface
{
public:
    virtual ConfiguratorErrorCode getFirstActiveUser(std::string &userData) = 0;
    virtual ConfiguratorErrorCode getNextActiveUser(std::string &userData) = 0;
    virtual ConfiguratorErrorCode getActiveUserByLogin(const std::string &login, std::string &userData) = 0;

    virtual ConfiguratorErrorCode getFirstArchiveUser(std::string &userData) = 0;
    virtual ConfiguratorErrorCode getNextArchiveUser(std::string &userData) = 0;
    virtual ConfiguratorErrorCode getArchiveUserByLogin(const std::string &login, std::string &userData) = 0;

    virtual ConfiguratorErrorCode addUser(const std::string &login, const std::string &hashedPassword, const std::vector<UserRole> &roles) = 0;
    virtual ConfiguratorErrorCode removeUser(const std::string &login) = 0;
    virtual ConfiguratorErrorCode updatePassword(const std::string &login, const std::string &newHashedPassword, const unsigned &passwordHistoryDepth) = 0;
    virtual ConfiguratorErrorCode updateRoles(const std::string &login, const std::vector<UserRole> &newRoles) = 0;

    virtual ~ConfiguratorDatabaseInterface() = default;
};

#endif