// src/SecurityConfig.cpp

#include <fstream>

#include "SecurityConfig.hpp"

// Загрузка данных конфигурации из файла
ConfiguratorErrorCode SecurityConfig::loadFromConfigFile()
{
    // Открытие файла конфигурации для чтения
    std::ifstream file(configFilePath);
    if (!file)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Очистка текущих данных конфигурации
    data.clear();

    std::string key;
    unsigned value;
    // Чтение данных из файла и сохранение в контейнер data
    while (file >> key >> value)
    {
        data[key] = value;
    }

    return ConfiguratorErrorCode::SUCCESS;
}

// Сохранение данных конфигурации в файл
ConfiguratorErrorCode SecurityConfig::saveToConfigFile() const
{
    // Открытие файла конфигурации для записи
    std::ofstream file(configFilePath);
    if (!file)
    {
        // Ошибка при открытии файла
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Запись данных из контейнера data в файл
    for (const auto &[key, value] : data)
    {
        file << key << " " << value << "\n";
    }

    return ConfiguratorErrorCode::SUCCESS;
}

// Конструктор класса SecurityConfig, инициализирует путь к файлу конфигурации и    загружает данные из файла
SecurityConfig::SecurityConfig(std::string configPath) : configFilePath(configPath)
{
    // Загрузка конфигурационных параметров из файла
    loadFromConfigFile();
}

// Установка минимальной длины пароля и сохранение изменений в файл конфигурации
ConfiguratorErrorCode SecurityConfig::set_minPasswordLength(const unsigned length)
{
    // Обновление значения минимальной длины пароля в контейнере данных
    data[minPasswordLength] = length;

    // Сохранение изменений в файл конфигурации
    return saveToConfigFile();
}

// Установка глубины истории паролей и сохранение изменений в файл конфигурации
ConfiguratorErrorCode SecurityConfig::set_passwordHistoryDepth(const unsigned depth)
{
    // Обновление значения глубины истории паролей в контейнере данных
    data[passwordHistoryDepth] = depth;

    // Сохранение изменений в файл конфигурации
    return saveToConfigFile();
}

// Установка срока действия пароля (в днях) и сохранение изменений в файл конфигурации
ConfiguratorErrorCode SecurityConfig::set_passwordExpirationDays(const unsigned days)
{
    // Обновление значения срока действия пароля в контейнере данных
    data[passwordExpirationDays] = days;

    // Сохранение изменений в файл конфигурации
    return saveToConfigFile();
}

// Установка максимального времени неактивности пользователя (в минутах) и сохранение изменений в файл конфигурации
ConfiguratorErrorCode SecurityConfig::set_maxInactiveTimeMin(const unsigned minutes)
{
    // Обновление значения максимального времени неактивности в контейнере данных
    data[maxInactiveTimeMin] = minutes;

    // Сохранение изменений в файл конфигурации
    return saveToConfigFile();
}

// Установка максимального числа неудачных попыток входа и сохранение изменений в файл конфигурации
ConfiguratorErrorCode SecurityConfig::set_maxFailedAttempts(const unsigned attempts)
{
    // Обновление значения максимального числа неудачных попыток входа в контейнере данных
    data[maxFailedAttempts] = attempts;

    // Сохранение изменений в файл конфигурации
    return saveToConfigFile();
}

// Установка времени блокировки учетной записи (в минутах) и сохранение изменений в файл конфигурации
ConfiguratorErrorCode SecurityConfig::set_lockoutTimeMin(const unsigned minutes)
{
    // Обновление значения времени блокировки в контейнере данных
    data[lockoutTimeMin] = minutes;

    // Сохранение изменений в файл конфигурации
    return saveToConfigFile();
}

// Получение минимальной длины пароля из конфигурации
ConfiguratorErrorCode SecurityConfig::get_minPasswordLength(unsigned &length) const
{
    // Проверка, существует ли параметр минимальной длины пароля в данных
    if (!data.count(minPasswordLength))
    {
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Присваивание значения минимальной длины пароля
    length = data.at(minPasswordLength);
    return ConfiguratorErrorCode::SUCCESS;
}

// Получение глубины истории паролей из конфигурации
ConfiguratorErrorCode SecurityConfig::get_passwordHistoryDepth(unsigned &depth) const
{
    // Проверка, существует ли параметр глубины истории паролей в данных
    if (!data.count(passwordHistoryDepth))
    {
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Присваивание значения глубины истории паролей
    depth = data.at(passwordHistoryDepth);
    return ConfiguratorErrorCode::SUCCESS;
}

// Получение срока действия пароля (в днях) из конфигурации
ConfiguratorErrorCode SecurityConfig::get_passwordExpirationDays(unsigned &days) const
{
    // Проверка, существует ли параметр срока действия пароля в данных
    if (!data.count(passwordExpirationDays))
    {
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Присваивание значения срока действия пароля
    days = data.at(passwordExpirationDays);
    return ConfiguratorErrorCode::SUCCESS;
}

// Получение максимального времени неактивности пользователя (в минутах) из конфигурации
ConfiguratorErrorCode SecurityConfig::get_maxInactiveTimeMin(unsigned &minutes) const
{
    // Проверка, существует ли параметр максимального времени неактивности в данных
    if (!data.count(maxInactiveTimeMin))
    {
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Присваивание значения максимального времени неактивности
    minutes = data.at(maxInactiveTimeMin);
    return ConfiguratorErrorCode::SUCCESS;
}

// Получение максимального числа неудачных попыток входа из конфигурации
ConfiguratorErrorCode SecurityConfig::get_maxFailedAttempts(unsigned &attempts) const
{
    // Проверка, существует ли параметр максимального числа неудачных попыток в данных
    if (!data.count(maxFailedAttempts))
    {
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Присваивание значения максимального числа неудачных попыток
    attempts = data.at(maxFailedAttempts);
    return ConfiguratorErrorCode::SUCCESS;
}

// Получение времени блокировки пользователя (в минутах) из конфигурации
ConfiguratorErrorCode SecurityConfig::get_lockoutTimeMin(unsigned &minutes) const
{
    // Проверка, существует ли параметр времени блокировки в данных
    if (!data.count(lockoutTimeMin))
    {
        return ConfiguratorErrorCode::DATABASE_ERROR;
    }

    // Присваивание значения времени блокировки
    minutes = data.at(lockoutTimeMin);
    return ConfiguratorErrorCode::SUCCESS;
}
