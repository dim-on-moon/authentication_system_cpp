CXX = g++
# Флаги для компиляции (включая покрытие для всех файлов)
CXXFLAGS = -Wall -Wextra -std=c++17 -I./include -MMD -MP -O0 --coverage
TEST_CXXFLAGS = $(CXXFLAGS) -I/usr/include/gtest -I/usr/include/gmock
# Флаги линковки для приложения
LDFLAGS = -lgcov -lsodium
# Флаги линковки для тестов
TEST_LDFLAGS = -lgtest -lgtest_main -lgmock -lpthread -lgcov -lsodium

SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
OBJ_DIR = obj
BIN_DIR = bin
BIN_TEST_DIR = $(BIN_DIR)/tests
CONFIGURATOR_DIR = configurator
USER_SYSTEM_DIR = user_system

CONFIGURATOR_SRC = $(CONFIGURATOR_DIR)/main.cpp
CONFIGURATOR_OBJ = $(OBJ_DIR)/configurator.o
CONFIGURATOR_BIN = $(BIN_DIR)/configurator

USER_SYSTEM_SRC = $(USER_SYSTEM_DIR)/main.cpp
USER_SYSTEM_OBJ = $(OBJ_DIR)/user_system.o
USER_SYSTEM_BIN = $(BIN_DIR)/user_system

SRC_NO_MAIN = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_NO_MAIN = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_NO_MAIN))

# Исходники тестов
TEST_SRC = $(wildcard $(TEST_DIR)/*.cpp)
# Объектные файлы всех тестов
TEST_OBJ = $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(TEST_SRC))
# Бинарные файлы тестов
TEST_BIN = $(patsubst $(TEST_DIR)/%.cpp, $(BIN_TEST_DIR)/%, $(TEST_SRC))

# Цель по умолчанию
all: $(USER_SYSTEM_BIN) $(CONFIGURATOR_BIN) $(TEST_BIN)

# Создание необходимых директорий
dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(BIN_TEST_DIR)

# Генерация зависимостей
DEP_FILES = $(OBJ_NO_MAIN:.o=.d) $(TEST_OBJ:.o=.d) $(CONFIGURATOR_OBJ:.o=.d) $(USER_SYSTEM_OBJ:.o=.d) 
-include $(DEP_FILES)

# Компиляция исходников в объектные файлы
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Компиляция main.cpp для configurator в объектный файл
$(CONFIGURATOR_OBJ): $(CONFIGURATOR_SRC) | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Сборка приложения configurator
$(CONFIGURATOR_BIN): $(CONFIGURATOR_OBJ) $(OBJ_NO_MAIN) | dirs
	$(CXX) $(CONFIGURATOR_OBJ) $(OBJ_NO_MAIN) -o $@ $(LDFLAGS)

# Запуск приложения configurator
run_configurator: $(CONFIGURATOR_BIN)
	./$(CONFIGURATOR_BIN)

# Компиляция main.cpp для user_system в объектный файл
$(USER_SYSTEM_OBJ): $(USER_SYSTEM_SRC) | dirs
	$(CXX) $(CXXFLAGS) -c $(USER_SYSTEM_SRC) -o $@

# Сборка приложения user_system
$(USER_SYSTEM_BIN): $(USER_SYSTEM_OBJ) $(OBJ_NO_MAIN) | dirs
	$(CXX) $(USER_SYSTEM_OBJ) $(OBJ_NO_MAIN) -o $@ $(LDFLAGS)

# Запуск приложения user_system
run_user_system: $(USER_SYSTEM_BIN)
	./$(USER_SYSTEM_BIN)

# Компиляция исходников тестов в объектные файлы
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | dirs
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(BIN_TEST_DIR)/%: $(OBJ_DIR)/%.o $(OBJ_NO_MAIN) | dirs
	$(CXX) $^ -o $@ $(TEST_LDFLAGS)

# Запуск всех тестов
run_tests: $(TEST_BIN)
	@for bin in $(TEST_BIN); do echo "Running $$bin..."; ./$$bin || exit 1; done

# Анализ покрытия
coverage: clean run_tests
	lcov --capture --directory $(OBJ_DIR)/ --output-file coverage.info --rc geninfo_unexecuted_blocks=1 --rc lcov_branch_coverage=1 --ignore-errors mismatch,mismatch
	lcov --remove coverage.info '/usr/*' '$(TEST_DIR)/*' --output-file coverage.info --rc lcov_branch_coverage=1 --ignore-errors unused
	genhtml coverage.info --output-directory coverage_report --rc lcov_branch_coverage=1
	@echo "Coverage report generated in coverage_report/index.html"

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all run_configurator run_user_system run_tests clean dirs coverage