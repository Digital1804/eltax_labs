# Компилятор и флаги компиляции
CC = gcc
# Переменные
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
TARGETS = $(SOURCES:.c=)

# Сборка всех исполняемых файлов
all: $(TARGETS)

# Создание каждого исполняемого файла
$(TARGETS): %: %.o
	$(CC) -o $@ $<

# Создание объектных файлов
%.o: %.c 
	$(CC) -c $<

# Правило для очистки проекта
clean:
	rm -f $(OBJECTS) $(TARGETS)

# Файлы, которые не являются файлами
.PHONY: all clean