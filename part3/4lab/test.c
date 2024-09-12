#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/rwlock.h>

#define DEVICE_NAME "test"
#define MSG_SIZE 100

static int major;
static rwlock_t lock;
static char msg[MSG_SIZE] = "HELLO\0";
ssize_t test_read(struct file *fd, char __user *buff, size_t size, loff_t *off);
ssize_t test_write(struct file *fd, const char __user *buff, size_t size, loff_t *off);

/* Функция чтения из устройства (передача строки в пространство пользователя) */
ssize_t test_read(struct file *fd, char __user *buff, size_t size, loff_t *off) {
    if (*msg == 0) return 0; // Если строка пустая, возвращаем 0
    size_t rc;

    read_lock(&lock);
    rc = simple_read_from_buffer(buff, size, off, msg, MSG_SIZE);
    read_unlock(&lock);
    

    return rc;
}

/* Функция записи в устройство (получение строки от пользователя) */
ssize_t test_write(struct file *fd, const char __user *buff, size_t size, loff_t *off) {
    if (size > MSG_SIZE - 1) {
        pr_info("my_module: input too long\n");
        return -EINVAL; // Ошибка, если строка слишком длинная
    }
    size_t rc;

    write_lock(&lock);
    msg[size] = '\0'; // Добавляем нуль-терминатор
    rc = simple_write_to_buffer(msg, MSG_SIZE, off, buff, size);
    write_unlock(&lock);


    return rc;
}

/* Структура операций с файлом */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = test_read,
    .write = test_write,
};

/* Функция инициализации модуля */
int init_module(void) {
    pr_info("my_module: module is loaded\n");
    rwlock_init(&lock);
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
        pr_info("my_module: failed to register device\n");
        return major;
    }

    pr_info("my_module: registered with major number %d\n", major);
    return 0;
}

/* Функция очистки модуля */
void cleanup_module(void) {
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("my_module: unregistered device\n");
}

MODULE_LICENSE("GPL");
