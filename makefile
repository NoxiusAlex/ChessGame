# Компилятор
CXX = g++

# Флаги компиляции
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -g -Iinclude

# Флаги линковки для SFML
LDFLAGS = -L/run/current-system/sw/lib
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lfreetype

# Имя исполняемого файла
TARGET = myapp

# Директории
SRC_DIR = src
BUILD_DIR = .build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Поиск всех .cpp файлов
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Создание необходимых директорий
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Полный путь к исполняемому файлу
TARGET_PATH = $(BIN_DIR)/$(TARGET)

# Правило по умолчанию
all: $(TARGET_PATH)

# Сборка исполняемого файла (ИСПРАВЛЕНО: добавлены LIBS)
$(TARGET_PATH): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET_PATH) $(LDFLAGS) $(LIBS)

# Компиляция каждого .cpp файла в .o файл
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка
clean:
	rm -rf $(BUILD_DIR)

# Запуск приложения
run: $(TARGET_PATH)
	./$(TARGET_PATH)

.PHONY: all clean run
