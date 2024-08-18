# CC := gcc
# compile: server.o client.o
# 	$(MAKE) -C clientlib
# 	$(CC) -o server server.o -lrt
# 	$(CC) -o client client.o -L./clientlib -lclient -Wl,-rpath,./clientlib -lrt -lncurses -lpthread

# server.o: server.c
# 	$(CC) -c server.c info.h

# client.o: client.c
# 	$(CC) -c client.c

# clean:
# 	rm -rf *.o clientlib/*.o *.gch
# Основной Makefile

# Параметры компилятора
CC = gcc
CFLAGS = -fPIC #-Wall 
LDFLAGS_CLIENT = -L./clientlib -lclient -Wl,-rpath,./clientlib -lrt -lncurses -lpthread
LDFLAGS_SERVER = info.h -lrt

# Пути к файлам
SRCS_SERVER = server.c
SRCS_CLIENT = client.c
SRCS_CLIENTLIB = clientlib/shm_sem.c clientlib/win_func.c

OBJS_SERVER = $(SRCS_SERVER:.c=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)
OBJS_CLIENTLIB = $(SRCS_CLIENTLIB:.c=.o)

TARGET_SERVER = server
TARGET_CLIENT = client
TARGET_CLIENTLIB = clientlib/libclient.so

.PHONY: all clean

# Сборка всех целей
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Сборка сервера
$(TARGET_SERVER): $(OBJS_SERVER) $(TARGET_CLIENTLIB)
	$(CC) -o $@ $(OBJS_SERVER) $(LDFLAGS_SERVER)

# Сборка клиента
$(TARGET_CLIENT): $(OBJS_CLIENT) $(TARGET_CLIENTLIB)
	$(CC) -o $@ $(OBJS_CLIENT) $(LDFLAGS_CLIENT)

# Сборка динамической библиотеки клиентских функций
$(TARGET_CLIENTLIB): $(OBJS_CLIENTLIB)
	$(CC) -shared -o $@ $(OBJS_CLIENTLIB)

# Генерация object-файлов из C-файлов
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка скомпилированных файлов
clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_CLIENTLIB) 
# $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_CLIENTLIB)
