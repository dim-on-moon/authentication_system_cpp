// src/ConfiguratorDatabase.cpp

#include <ctime>

#include "ConfiguratorDatabase.hpp"

// Конструктор класса ConfiguratorDatabase для инициализации путей к файлам
ConfiguratorDatabase::ConfiguratorDatabase(std::string archivePath,
                                           std::string activePath,
                                           std::string tmpPath) : archiveFilePath(archivePath), activeUsersFilePath(activePath), tmpFilePath(tmpPath) {}

// Получение данных первого активного пользователя из файла
ConfiguratorErrorCode ConfiguratorDatabase::getFirstActiveUser(std::string &userData)
{

    // Закрытие файла, если он открыт
    if (activeUsersFile.is_open())
    {
        activeUsersFile.close();
    }

    // Открытие файла для чтения
    activeUsersFile.open(activeUsersFilePath, std::ios::in);
    if (!activeUsersFile)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Чтение первой строки из файла
    if (std::getline(activeUsersFile, userData))
    {
        // Успешное чтение данных
        return ConfiguratorErrorCode::SUCCESS;
    }

    // Конец файла или пустой файл
    return ConfiguratorErrorCode::END_OF_TABLE;
}

// Получение данных следующего активного пользователя из файла
ConfiguratorErrorCode ConfiguratorDatabase::getNextActiveUser(std::string &userData)
{

    // Проверка, открыт ли файл
    if (!activeUsersFile)
    {
        // Ошибка при работе с файлом
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Чтение следующей строки из файла
    if (std::getline(activeUsersFile, userData))
    {
        // Успешное чтение данных
        return ConfiguratorErrorCode::SUCCESS;
    }

    // Конец файла
    return ConfiguratorErrorCode::END_OF_TABLE;
}

// Получение данных активного пользователя по логину
ConfiguratorErrorCode ConfiguratorDatabase::getActiveUserByLogin(const std::string &login, std::string &userData)
{

    // Открытие файла для чтения
    std::ifstream file(activeUsersFilePath);
    if (!file)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    std::string line;
    // Чтение строк файла
    while (std::getline(file, line))
    {
        // Извлечение ключа (логина) из строки
        std::string key = line.substr(0, line.find(' '));
        if (key == login)
        {
            // Если логин совпадает, возвращаем данные пользователя
            userData = line;
            file.close();
            return ConfiguratorErrorCode::SUCCESS;
        }
    }

    file.close();

    // Логин не найден
    return ConfiguratorErrorCode::LOGIN_NOT_FOUND;
}

// Получение данных первого пользователя из архива
ConfiguratorErrorCode ConfiguratorDatabase::getFirstArchiveUser(std::string &userData)
{

    // Закрытие файла, если он открыт
    if (archiveFile.is_open())
    {
        archiveFile.close();
    }

    // Открытие файла архива для чтения
    archiveFile.open(archiveFilePath, std::ios::in);
    if (!archiveFile)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Чтение первой строки из файла
    if (std::getline(archiveFile, userData))
    {
        // Успешное чтение данных
        return ConfiguratorErrorCode::SUCCESS;
    }

    // Конец файла или пустой файл
    return ConfiguratorErrorCode::END_OF_TABLE;
}

// Получение данных следующего пользователя из архива
ConfiguratorErrorCode ConfiguratorDatabase::getNextArchiveUser(std::string &userData)
{

    // Проверка, открыт ли файл архива
    if (!archiveFile)
    {
        // Ошибка при работе с файлом
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Чтение следующей строки из файла
    if (std::getline(archiveFile, userData))
    {
        // Успешное чтение данных
        return ConfiguratorErrorCode::SUCCESS;
    }

    // Конец файла
    return ConfiguratorErrorCode::END_OF_TABLE;
}

// Получение данных пользователя из архива по логину
ConfiguratorErrorCode ConfiguratorDatabase::getArchiveUserByLogin(const std::string &login, std::string &userData)
{

    // Открытие файла архива для чтения
    std::ifstream file(archiveFilePath);
    if (!file)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    std::string line;
    // Чтение строк файла архива
    while (std::getline(file, line))
    {
        // Извлечение ключа (логина) из строки
        std::string key = line.substr(0, line.find(' '));
        if (key == login)
        {
            // Если логин совпадает, возвращаем данные пользователя
            userData = line;
            file.close();
            return ConfiguratorErrorCode::SUCCESS;
        }
    }

    file.close();

    // Логин не найден
    return ConfiguratorErrorCode::LOGIN_NOT_FOUND;
}

// Добавление нового пользователя в активных пользователей и архив
ConfiguratorErrorCode ConfiguratorDatabase::addUser(const std::string &login, const std::string &hashedPassword, const std::vector<UserRole> &roles)
{
    //Если пользователь с таким логином уже есть в базе - добавление невозможно
    std::string userData;
    ConfiguratorErrorCode code;
    code = getArchiveUserByLogin(login, userData);
    if (code == ConfiguratorErrorCode::SUCCESS)
    {
        return ConfiguratorErrorCode::LOGIN_ALREADY_EXISTS;
    }

    // Открытие файла активных пользователей для добавления данных
    std::ofstream file(activeUsersFilePath, std::ios::app);
    if (!file)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Получение текущей даты для записи в файл
    std::time_t t = std::time(nullptr);
    std::tm *now = std::localtime(&t);

    // Запись данных пользователя: логин, хеш пароля, дата создания и ролей
    file << login << " " << hashedPassword << " " << now->tm_mday << "." << now->tm_mon + 1 << "." << now->tm_year + 1900 << " ";
    for (size_t i = 0; i < roles.size() - 1; ++i)
    {
        file << static_cast<int>(roles[i]) << ",";
    }
    file << static_cast<int>(roles[roles.size() - 1]) << "\n";

    file.close();

    // Открытие файла архива для добавления данных
    file.open(archiveFilePath, std::ios::app);
    if (!file)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Запись базовых данных пользователя в архив (логин и хеш пароля)
    file << login << " " << hashedPassword << "\n";

    file.close();

    return ConfiguratorErrorCode::SUCCESS;
}

// Удаление пользователя по логину из активных пользователей
ConfiguratorErrorCode ConfiguratorDatabase::removeUser(const std::string &login)
{

    // Открытие файла активных пользователей для чтения
    std::ifstream inFile(activeUsersFilePath);
    if (!inFile)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Открытие временного файла для записи
    std::ofstream outFile(tmpFilePath);
    if (!outFile)
    {
        // Ошибка при открытии временного файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    std::string line;
    bool found = false;
    // Чтение строк из файла активных пользователей
    while (std::getline(inFile, line))
    {
        // Пропуск строки с указанным логином
        if (line.substr(0, line.find(' ')) == login)
        {
            found = true;
            continue;
        }
        // Запись строки в временный файл, если логин не совпадает
        outFile << line << "\n";
    }

    inFile.close();
    outFile.close();

    // Удаление старого файла и переименование временного в основной
    remove(activeUsersFilePath.c_str());
    rename(tmpFilePath.c_str(), activeUsersFilePath.c_str());

    return found ? ConfiguratorErrorCode::SUCCESS : ConfiguratorErrorCode::LOGIN_NOT_FOUND;
}

// Обновление пароля пользователя в активных пользователях и архиве
ConfiguratorErrorCode ConfiguratorDatabase::updatePassword(const std::string &login, const std::string &newHashedPassword, const unsigned &passwordHistoryDepth)
{

    // Обновление пароля в таблице активных пользователей

    // Открытие файла активных пользователей для чтения
    std::ifstream inFile(activeUsersFilePath);
    if (!inFile)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Открытие временного файла для записи
    std::ofstream outFile(tmpFilePath);
    if (!outFile)
    {
        // Ошибка при открытии временного файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    std::string line;
    bool found = false;
    // Чтение строк из файла активных пользователей
    while (std::getline(inFile, line))
    {
        // Если логин совпадает, обновляем строку с новым паролем и текущей датой
        if (line.substr(0, line.find(' ')) == login)
        {
            found = true;
            std::string roles = line.substr(line.find(' ', line.find(' ', line.find(' ') + 1) + 1));

            // Получение текущей даты
            std::time_t t = std::time(nullptr);
            std::tm *now = std::localtime(&t);

            // Формирование новой строки с обновленным паролем
            line = login + " " + newHashedPassword + " " + std::to_string(now->tm_mday) + "." + std::to_string(now->tm_mon + 1) + "." + std::to_string(now->tm_year + 1900) + " " + roles;
        }
        outFile << line << "\n";
    }


    inFile.close();
    outFile.close();

    // Удаление старого файла и переименование временного в основной
    remove(activeUsersFilePath.c_str());
    rename(tmpFilePath.c_str(), activeUsersFilePath.c_str());

    if (!found)
    {
        return ConfiguratorErrorCode::LOGIN_NOT_FOUND;
    }

    // Запись нового пароля в архив

    // найти в архиве нужную строчку
    // Определить, сколько старых паролей там записано
    // если их число равно passwordHistoryDepth, придется перезаписывать (пароли располагаются от старых к новым)
    // если их меньше, дописываем еще один пароль

    // Открытие файла архива для чтения
    inFile.open(archiveFilePath);
    if (!inFile)
    {
        // Ошибка при открытии файла архива
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Открытие временного файла для записи
    outFile.open(tmpFilePath);
    if (!outFile)
    {
        // Ошибка при открытии временного файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Чтение строк из архива
    found = false;
    while (std::getline(inFile, line))
    {
        // Если логин совпадает, обновляем список паролей
        if (line.substr(0, line.find(' ')) == login)
        {
            found = true;
            std::vector<std::string> oldPasswords;
            line = line.substr(line.find(' ') + 1); // строка без логина
            // Разделение строки на старые пароли
            while (line.find(' ') != std::string::npos)
            {
                oldPasswords.push_back(line.substr(0, line.find(' ')));
                line = line.substr(line.find(' ') + 1);
            }
            oldPasswords.push_back(line);

            // Если количество паролей меньше глубины хранения, добавляем новый
            if (oldPasswords.size() < passwordHistoryDepth)
            {
                line = login;
                for (size_t i = 0; i < oldPasswords.size(); ++i)
                {
                    line += " " + oldPasswords[i];
                }
            }
            else
            { // Если количество хранимых больше или равно глубине хранения, затираем старые пароли 
                line = login;
                for (size_t i = oldPasswords.size() - passwordHistoryDepth + 1; i < oldPasswords.size(); ++i)
                {
                    line += " " + oldPasswords[i];
                }
            }
            line += " " + newHashedPassword; // Добавление нового пароля
        }
        outFile << line << "\n";
    }

    inFile.close();
    outFile.close();

    // Удаление старого архива и переименование временного в новый
    remove(archiveFilePath.c_str());
    rename(tmpFilePath.c_str(), archiveFilePath.c_str());

    return found ? ConfiguratorErrorCode::SUCCESS : ConfiguratorErrorCode::LOGIN_NOT_FOUND;
}

// Обновление ролей пользователя в таблице активных пользователей
ConfiguratorErrorCode ConfiguratorDatabase::updateRoles(const std::string &login, const std::vector<UserRole> &newRoles)
{

    // Открытие файла активных пользователей для чтения
    std::ifstream inFile(activeUsersFilePath);
    if (!inFile)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Открытие временного файла для записи
    std::ofstream outFile(tmpFilePath);
    if (!outFile)
    {
        // Ошибка при открытии временного файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    std::string line;
    bool found = false;
    // Чтение строк из файла активных пользователей
    while (std::getline(inFile, line))
    {
        // Если логин совпадает, обновляем строку с новыми ролями
        if (line.substr(0, line.find(' ')) == login)
        {
            found = true;
            // Оставляем первые части строки (логин, пароль и дату задания пароля)
            line = line.substr(0, line.find(' ', line.find(' ', line.find(' ') + 1) + 1)) + " ";
            // Добавляем новые роли в строку
            for (size_t i = 0; i < newRoles.size() - 1; ++i)
            {
                line += std::to_string(static_cast<int>(newRoles[i])) + ",";
            }
            line += std::to_string(static_cast<int>(newRoles[newRoles.size() - 1]));
        }
        outFile << line << "\n";
    }

    inFile.close();
    outFile.close();

    // Удаление старого файла и переименование временного в основной
    remove(activeUsersFilePath.c_str());
    rename(tmpFilePath.c_str(), activeUsersFilePath.c_str());

    return found ? ConfiguratorErrorCode::SUCCESS : ConfiguratorErrorCode::LOGIN_NOT_FOUND;
}

// Деструктор для закрытия файлов перед уничтожением объекта
ConfiguratorDatabase::~ConfiguratorDatabase()
{
    // Закрытие файла активных пользователей, если он открыт
    if (activeUsersFile.is_open())
    {
        activeUsersFile.close();
    }
    // Закрытие файла архива, если он открыт
    if (archiveFile.is_open())
    {
        archiveFile.close();
    }
}