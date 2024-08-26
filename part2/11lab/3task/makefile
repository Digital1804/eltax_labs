# Компилятор и флаги компиляции
CC = gcc
HEADER = config.h
# Переменные
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
TARGETS = $(SOURCES:.c=)

# Сборка всех исполняемых файлов
all: $(TARGETS)

# Создание каждого исполняемого файла
%: %.o
	$(CC) $< -o $@

# Создание объектных файлов
%.o: %.c
	$(CC) $(HEADER) -c $< -o $@

# Правило для очистки проекта
clean:
	rm -f $(OBJECTS) $(TARGETS)

# Файлы, которые не являются файлами
.PHONY: all clean
