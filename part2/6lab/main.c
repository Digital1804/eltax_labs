#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_STORES 5
#define NUM_BUYERS 3
#define INITIAL_STORE_CAPACITY 2000
#define BUYER_DEMAND_LIMIT 20000
#define RESTOCK_AMOUNT 5000


int stores[NUM_STORES];// Массив магазинов


int buyer_needs[NUM_BUYERS];// Потребности покупателей


pthread_mutex_t store_mutexes[NUM_STORES];// Мьютексы для синхронизации доступа к магазинам


bool buyers_done = false;// Флаг для завершения работы потоков

/** @brief Функция, выполняемая покупателями
    @param arg Идентификатор покупателя
    @return NULL
 */
void *buyer(void *arg) {
    int buyer_id = *(int *)arg;
    while (buyer_needs[buyer_id] > 0) {
        for (int i = 0; i < NUM_STORES; i++) {
            if (pthread_mutex_trylock(&store_mutexes[i]) == 0) {
                if (stores[i] > 0) {
                    int taken = stores[i] < buyer_needs[buyer_id] ? stores[i] : buyer_needs[buyer_id];
                    stores[i] -= taken;
                    buyer_needs[buyer_id] -= taken;
                    printf("Buyer %d bought %d units from store %d, needs %d\n", buyer_id, taken, i, buyer_needs[buyer_id]);
                }
                pthread_mutex_unlock(&store_mutexes[i]);
            }
        }
        sleep(2);  // Засыпает на 2 секунды
    }
    return NULL;
}

/** @brief Функция, выполняемая поставщиком
*/
void *supplier(void *arg) {
    while (!buyers_done) {
        for (int i = 0; i < NUM_STORES; i++) {
            if (pthread_mutex_trylock(&store_mutexes[i]) == 0) {
                if (stores[i] < INITIAL_STORE_CAPACITY) {
                    stores[i] += RESTOCK_AMOUNT;
                    printf("Supplier add %d units in store %d, now in store %d\n", 
                           RESTOCK_AMOUNT, i, stores[i]);
                }
                pthread_mutex_unlock(&store_mutexes[i]);
            }
        }
        sleep(1);  // Засыпает на 1 секунду
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    // Инициализация магазинов
    for (int i = 0; i < NUM_STORES; i++) {
        stores[i] = rand() % INITIAL_STORE_CAPACITY + 9000;
        pthread_mutex_init(&store_mutexes[i], NULL);
        printf("Store %d has an initial stock %d units\n", i, stores[i]);
    }

    // Инициализация потребностей покупателей
    pthread_t buyers[NUM_BUYERS];
    int buyer_ids[NUM_BUYERS];
    for (int i = 0; i < NUM_BUYERS; i++) {
        buyer_needs[i] = rand() % BUYER_DEMAND_LIMIT + 90000;
        buyer_ids[i] = i;
        printf("Buyer %d has an initial need %d units\n", i, buyer_needs[i]);
        pthread_create(&buyers[i], NULL, buyer, &buyer_ids[i]);
    }

    // Создание потока поставщика
    pthread_t supplier_thread;
    pthread_create(&supplier_thread, NULL, supplier, NULL);

    // Ожидание завершения всех покупателей
    for (int i = 0; i < NUM_BUYERS; i++) {
        pthread_join(buyers[i], NULL);
    }

    // Все покупатели удовлетворены, завершаем работу поставщика
    buyers_done = true;
    pthread_join(supplier_thread, NULL);

    // Очистка ресурсов
    for (int i = 0; i < NUM_STORES; i++) {
        pthread_mutex_destroy(&store_mutexes[i]);
    }

    printf("All buyers are satisfied, the work is completed..\n");
    return 0;
}
