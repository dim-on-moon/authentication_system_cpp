// include/ConfiguratorDatabase.hpp

#include <string>
#include <fstream>

#include "ConfiguratorDatabaseInterface.hpp"

#ifndef CONFIGURATOR_DATABASE_HPP
#define CONFIGURATOR_DATABASE_HPP

// Класс для работы с базой данных конфигурации: управление активными пользователями и архивом
class ConfiguratorDatabase : public ConfiguratorDatabaseInterface
{
    std::string archiveFilePath;     // Путь к файлу с архивом (архив - список логинов и ранее используемых паролей)
    std::string activeUsersFilePath; // Путь к файлу с таблицей активных пользователей
    std::string tmpFilePath;         // Путь к временному файлу, используемому при перезаписи

    std::fstream activeUsersFile; // Файловый поток, связанный с таблицей активных пользователей
    std::fstream archiveFile;     // Файловый поток, связанный с файлом архива

public:
    // Конструктор класса ConfiguratorDatabase для инициализации путей к файлам
    ConfiguratorDatabase(std::string archivePath = "./configDb/archive.txt",
                         std::string activePath = "./configDb/active_users.txt",
                         std::string tmpPath = "./configDb/tmp_file.txt");

    // Получение данных первого активного пользователя из файла
    ConfiguratorErrorCode getFirstActiveUser(std::string &userData) override;

    // Получение данных следующего активного пользователя из файла
    ConfiguratorErrorCode getNextActiveUser(std::string &userData) override;

    // Получение данных активного пользователя по логину
    ConfiguratorErrorCode getActiveUserByLogin(const std::string &login, std::string &userData) override;

    // Получение данных первого пользователя из архива
    ConfiguratorErrorCode getFirstArchiveUser(std::string &userData) override;

    // Получение данных следующего пользователя из архива
    ConfiguratorErrorCode getNextArchiveUser(std::string &userData) override;

    // Получение данных пользователя из архива по логину
    ConfiguratorErrorCode getArchiveUserByLogin(const std::string &login, std::string &userData) override;

    // Добавление нового пользователя в активных пользователей и архив
    ConfiguratorErrorCode addUser(const std::string &login, const std::string &hashedPassword, const std::vector<UserRole> &roles) override;

    // Удаление пользователя по логину из активных пользователей
    ConfiguratorErrorCode removeUser(const std::string &login) override;

    // Обновление пароля пользователя в активных пользователях и архиве
    ConfiguratorErrorCode updatePassword(const std::string &login, const std::string &newHashedPassword, const unsigned &passwordHistoryDepth) override;

    // Обновление ролей пользователя в таблице активных пользователей
    ConfiguratorErrorCode updateRoles(const std::string &login, const std::vector<UserRole> &newRoles) override;

    // Деструктор для закрытия файлов перед уничтожением объекта
    ~ConfiguratorDatabase();
};

#endif