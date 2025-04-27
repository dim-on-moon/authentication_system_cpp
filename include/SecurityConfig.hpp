// include/SecurityConfig.hpp

#include <string>
#include <map>

#include "SecurityConfigInterface.hpp"

#ifndef SECURITY_CONFIGUR_HPP
#define SECURITY_CONFIGUR_HPP
// Класс для работы с конфигурацией безопасности, управляет параметрами безопасности, реализует интерфейс SecurityConfigInterface
class SecurityConfig : public SecurityConfigInterface
{
    // Хранение данных конфигурации в виде пары "ключ-значение"
    std::map<std::string, unsigned> data;

    // Пути к файлу конфигурации и временному файлу
    std::string configFilePath;
    std::string tmpFilePath;

    // Константы для параметров безопасности
    const std::string minPasswordLength = "minPasswordLength";
    const std::string passwordHistoryDepth = "passwordHistoryDepth";
    const std::string passwordExpirationDays = "passwordExpirationDays";
    const std::string maxInactiveTimeMin = "maxInactiveTimeMin";
    const std::string maxFailedAttempts = "maxFailedAttempts";
    const std::string lockoutTimeMin = "lockoutTimeMin";

    // Загрузка данных конфигурации из файла
    ConfiguratorErrorCode loadFromConfigFile();

    // Сохранение данных конфигурации в файл
    ConfiguratorErrorCode saveToConfigFile() const;

public:
    // Конструктор класса SecurityConfig, инициализирует путь к файлу конфигурации и    загружает данные из файла
    SecurityConfig(std::string configPath = "./configDb/config.txt");

    // Установка минимальной длины пароля и сохранение изменений в файл конфигурации
    ConfiguratorErrorCode set_minPasswordLength(const unsigned length) override;

    // Установка глубины истории паролей и сохранение изменений в файл конфигурации
    ConfiguratorErrorCode set_passwordHistoryDepth(const unsigned depth) override;
    // Установка срока действия пароля (в днях) и сохранение изменений в файл конфигурации
    ConfiguratorErrorCode set_passwordExpirationDays(const unsigned days) override;

    // Установка максимального времени неактивности пользователя (в минутах) и сохранение изменений в файл конфигурации
    ConfiguratorErrorCode set_maxInactiveTimeMin(const unsigned minutes) override;

    // Установка максимального числа неудачных попыток входа и сохранение изменений в файл конфигурации
    ConfiguratorErrorCode set_maxFailedAttempts(const unsigned attempts) override;

    // Установка времени блокировки учетной записи (в минутах) и сохранение изменений в файл конфигурации
    ConfiguratorErrorCode set_lockoutTimeMin(const unsigned minutes) override;

    // Получение минимальной длины пароля из конфигурации
    ConfiguratorErrorCode get_minPasswordLength(unsigned &length) const override;

    // Получение глубины истории паролей из конфигурации
    ConfiguratorErrorCode get_passwordHistoryDepth(unsigned &depth) const override;

    // Получение срока действия пароля (в днях) из конфигурации
    ConfiguratorErrorCode get_passwordExpirationDays(unsigned &days) const override;

    // Получение максимального времени неактивности пользователя (в минутах) из конфигурации
    ConfiguratorErrorCode get_maxInactiveTimeMin(unsigned &minutes) const override;

    // Получение максимального числа неудачных попыток входа из конфигурации
    ConfiguratorErrorCode get_maxFailedAttempts(unsigned &attempts) const override;

    // Получение времени блокировки пользователя (в минутах) из конфигурации
    ConfiguratorErrorCode get_lockoutTimeMin(unsigned &minutes) const override;
};

#endif