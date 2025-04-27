// include/HashingInterface.hpp

#include <string>

#include "ErrorCode.hpp"

#ifndef HASHING_INTERFACE_HPP
#define HASHING_INTERFACE_HPP

class HashingInterface
{
public:
    // Метод хеширования пароля
    virtual ConfiguratorErrorCode pwHashMake(const std::string &password, std::string &hashedPassword) = 0;

    // Метод сравнения записанного хеша и хеша данного пароля
    virtual ConfiguratorErrorCode pwHashVerify(const std::string &password, const std::string &hashedPassword) = 0;

    virtual ~HashingInterface() = default;
};

#endif