// include/SecurityConfigInterface.hpp

#include "ErrorCode.hpp"

#ifndef SECURITY_CONFIG_INTERFACE_HPP
#define SECURITY_CONFIG_INTERFACE_HPP

class SecurityConfigInterface
{
public:
    virtual ConfiguratorErrorCode set_minPasswordLength(const unsigned length) = 0;
    virtual ConfiguratorErrorCode set_passwordHistoryDepth(const unsigned depth) = 0;
    virtual ConfiguratorErrorCode set_passwordExpirationDays(const unsigned days) = 0;
    virtual ConfiguratorErrorCode set_maxInactiveTimeMin(const unsigned minutes) = 0;
    virtual ConfiguratorErrorCode set_maxFailedAttempts(const unsigned attempts) = 0;
    virtual ConfiguratorErrorCode set_lockoutTimeMin(const unsigned minutes) = 0;

    virtual ConfiguratorErrorCode get_minPasswordLength(unsigned &length) const = 0;
    virtual ConfiguratorErrorCode get_passwordHistoryDepth(unsigned &depth) const = 0;
    virtual ConfiguratorErrorCode get_passwordExpirationDays(unsigned &days) const = 0;
    virtual ConfiguratorErrorCode get_maxInactiveTimeMin(unsigned &minutes) const = 0;
    virtual ConfiguratorErrorCode get_maxFailedAttempts(unsigned &attempts) const = 0;
    virtual ConfiguratorErrorCode get_lockoutTimeMin(unsigned &minutes) const = 0;

    virtual ~SecurityConfigInterface() = default;
};

#endif