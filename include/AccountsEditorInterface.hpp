// include/AccountsEditorInterface.hpp

#include <string>
#include <vector>

#include "ErrorCode.hpp"
#include "UserRole.hpp"

#ifndef ACCOUNTS_EDITOR_INTERFACE_HPP
#define ACCOUNTS_EDITOR_INTERFACE_HPP

class ConfiguratorAccountsEditorInterface
{
public:
    virtual ConfiguratorErrorCode createAccount(const std::string &login, const std::string &password, const std::vector<UserRole> &roles) = 0;
    virtual ConfiguratorErrorCode deleteAccount(const std::string &login) = 0;
    virtual ConfiguratorErrorCode editPassword(const std::string &login, const std::string &newPassword) = 0;
    virtual ConfiguratorErrorCode editRoles(const std::string &login, const std::vector<UserRole> &newRoles) = 0;

    virtual ~ConfiguratorAccountsEditorInterface() = default;
};

#endif