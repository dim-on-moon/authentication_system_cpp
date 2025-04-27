// include/Hashing.hpp

#include <string>

#include "HashingInterface.hpp"

#ifndef HASHING_HPP
#define HASHING_HPP

class Hashing : public HashingInterface
{
public:
    // Конструктор класса
    Hashing();

    // Метод хеширования пароля
    ConfiguratorErrorCode pwHashMake(const std::string &password, std::string &hashedPassword) override;

    // Метод сравнения записанного хеша и хеша данного пароля
    ConfiguratorErrorCode pwHashVerify(const std::string &password, const std::string &hashedPassword) override;
};

#endif