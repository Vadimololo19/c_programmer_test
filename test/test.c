#include <generic/rte_cycles.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_cycles.h>
#include <time.h>

void init_dpdk(int argc, char **argv);

struct rte_hash *create_hash(size_t entry_data);

void fill_hash(struct rte_hash *hash, size_t total_entry_data,size_t report_interval, size_t final_itertaion);

void print(struct rte_hash *hash,size_t start_time ,size_t curent, size_t total);

int main(int argc, char **argv) 
{
    size_t entry_data;
    size_t total_entry_data;
    size_t report_interval;
    size_t final_iteration;
    struct rte_hash *hash;

    srand(time(NULL));
    entry_data = 1 << 16;
    total_entry_data = 16 << 20;
    report_interval = 1 << 20;
    final_iteration = 123456;
    init_dpdk(argc, argv);
    hash = create_hash(entry_data);
    if (hash == NULL)
    {
        printf("Не удалось создать хэш таблицу: %" PRIu64 "\n", entry_data);
    }
    fill_hash(hash,total_entry_data,report_interval,final_iteration);
    return 0;
}

void init_dpdk(int argc, char **argv)
{
    int ret;
    
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
    {
        printf("Не удалось инициализировать DPDK\n");
        exit(EXIT_FAILURE);
    }
}

struct rte_hash *create_hash(size_t entry_data)
{
    struct rte_hash_parameters hash_params;

    hash_params.name = "cuckoo_hash"; 
    hash_params.entries = (uint32_t)entry_data;
    hash_params.key_len= sizeof(uint64_t);
    hash_params.hash_func = rte_jhash;
    hash_params.socket_id = 0;
    hash_params.extra_flag = 0;
    return rte_hash_create(&hash_params);
}

void fill_hash(struct rte_hash *hash, size_t total_entry_data,size_t report_interval, size_t final_itertaion)
{
    uint64_t start_time;
    uint64_t final_key;
    uint64_t *final_value;
    int ret;
    uint64_t print_end;
    uint64_t key;
    uint64_t value;

    final_value = malloc(sizeof(uint64_t));
    start_time = rte_get_tsc_cycles();
    final_key = 42;
    *final_value = 123456;
    ret = rte_hash_lookup_data(hash, &final_key, (void **)&final_value);
    print_end = rte_get_tsc_cycles() - start_time;
    for(size_t i = 1; i <= total_entry_data; i++)
    {
        key = ((uint64_t)rand() << 32 | rand());
        value = ((uint64_t)rand() << 32 | rand());
        if (i == final_itertaion)
        {   
            if (rte_hash_lookup_data(hash, &final_key, NULL) < 0)
            {
            ret = rte_hash_add_key_data(hash, &final_key, final_value);
            }
        }
        if (rte_hash_lookup_data(hash, &key, NULL) < 0)
        {
            ret = rte_hash_add_key_data(hash, &key, &value);
        }
        if (ret < 0)
        {
            printf("Не удалось добавить ключ %" PRIu64 " в хэш %" PRIu64 "\n", key,i);
            exit(EXIT_FAILURE);
        }
        if (i % report_interval == 0 || i == total_entry_data)
        {
            print(hash,start_time,i,total_entry_data);
        }
    }
    if (ret>= 0)
    {
        printf("Размер хэша: %d, ключ: %" PRIu64 ", значение: %" PRIu64 ", времяс: %" PRIu64 " \n",
       (int)rte_hash_count(hash), final_key, *final_value, print_end);
    }
    free(final_value);
}  

void print(struct rte_hash *hash,size_t start_time, size_t current,size_t total)
{
    uint64_t current_time;
    (void)hash;

    current_time = (rte_get_tsc_cycles() - start_time) *1e9 / rte_get_tsc_hz();
    printf("%zu  / %zu , вркмя: %" PRIu64 " \n",current,total,current_time);
}
