CC = gcc
CFLAGS = -Wall -fPIC -I./common
LDFLAGS = -L$(BUILD_DIR) -Wl,-rpath,$(BUILD_DIR)
LDLIBS = -luds

BUILD_DIR = ./build
LIBUDS_FILE = $(BUILD_DIR)/libuds.so
CLIENT_PROG = $(BUILD_DIR)/client
SERVER_PROG = $(BUILD_DIR)/server

# Цели, которые не являются файлами
.PHONY: all server client libuds clean

# Цель по умолчанию
all: libuds server client

# Псевдонимы для удобного вызова (make server, make client и т.д.)
libuds: $(LIBUDS_FILE)
server: $(SERVER_PROG)
client: $(CLIENT_PROG)

# Создание директории сборки
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Сборка библиотеки. Зависит от .c и .h файлов.
$(LIBUDS_FILE): ./common/uds_common.c ./common/uds_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -shared ./common/uds_common.c -o $(LIBUDS_FILE)

# Сборка сервера. Теперь зависит и от библиотеки, и от общего хедера.
$(SERVER_PROG): server/server.c ./common/uds_common.h $(LIBUDS_FILE)
	$(CC) $(CFLAGS) server/server.c $(LDFLAGS) $(LDLIBS) -o $(SERVER_PROG)

# Сборка клиента. Исправлена опечатка в имени выходного файла.
$(CLIENT_PROG): client/client.c ./common/uds_common.h $(LIBUDS_FILE)
	$(CC) $(CFLAGS) client/client.c $(LDFLAGS) $(LDLIBS) -o $(CLIENT_PROG)

# Очистка проекта
clean:
	rm -rf $(BUILD_DIR)