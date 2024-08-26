CC = gcc
CFLAGS = -fPIC
LDFLAGS_CLIENT = -lrt -lpthread 
LDFLAGS_SERVER = -L./servlib -lserv -Wl,-rpath,./servlib -lrt -lpthread 

# Пути к файлам
SRCS_SERVER = server.c
SRCS_CLIENT = client.c
SRCS_SERVLIB = $(wildcard servlib/*.c)

OBJS_SERVER = $(SRCS_SERVER:.c=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)
OBJS_SERVLIB = $(SRCS_SERVLIB:.c=.o)

TARGET_SERVER = $(SRCS_SERVER:.c=)
TARGET_CLIENT = $(SRCS_CLIENT:.c=)
TARGET_SERVLIB = servlib/libserv.so

.PHONY: all clean full_clean

# Сборка всех целей
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Сборка сервера
$(TARGET_SERVER): $(OBJS_SERVER) $(TARGET_SERVLIB)
	$(CC) -o $@ $(OBJS_SERVER) $(LDFLAGS_SERVER)

# Сборка клиента
$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) -o $@ $(OBJS_CLIENT) $(LDFLAGS_CLIENT)

# Сборка динамической библиотеки клиентских функций
$(TARGET_SERVLIB): $(OBJS_SERVLIB)
	$(CC) -shared -o $@ $(OBJS_SERVLIB)

# Генерация object-файлов из C-файлов
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка скомпилированных файлов
clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_SERVLIB) 

full_clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_SERVLIB) $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_SERVLIB)
