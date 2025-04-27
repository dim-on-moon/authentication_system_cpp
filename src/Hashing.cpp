// src/Hashing.cpp

#include <sodium.h>
#include <cstring>

#include "Hashing.hpp"

Hashing::Hashing() { sodium_init(); }

// Метод хеширования пароля
ConfiguratorErrorCode Hashing::pwHashMake(const std::string &password, std::string &hashedPassword)
{
    // Пользовательский пароль
    const char *c_str_password = password.c_str();
    // Буфер для хэшированного пароля
    char c_str_hashed_password[crypto_pwhash_STRBYTES];

    // Захешировать пароль
    if (crypto_pwhash_str(
            c_str_hashed_password,              // Выходной (строковый) хэш пароля
            c_str_password,                     // Пароль
            std::strlen(c_str_password),        // Длина пароля
            crypto_pwhash_OPSLIMIT_INTERACTIVE, // Ограничения по операциям (безопасно и относительно быстро для интерактивных задач)
            crypto_pwhash_MEMLIMIT_INTERACTIVE  // Ограничения по памяти
            ) != 0)
    {
        return ConfiguratorErrorCode::HASHING_ERROR;
    }

    hashedPassword = c_str_hashed_password;
    return ConfiguratorErrorCode::SUCCESS;
}

// Метод сравнения записанного хеша и хеша данного пароля
ConfiguratorErrorCode Hashing::pwHashVerify(const std::string &password, const std::string &hashedPassword)
{
    if (crypto_pwhash_str_verify(hashedPassword.c_str(), password.c_str(), password.length()) != 0)
    {
        return ConfiguratorErrorCode::PASSWORDS_DONT_MATCH;
    }

    return ConfiguratorErrorCode::SUCCESS;
}