#ifndef _MAIN_H
#define _MAIN_H 

#define ARG_NUM 6

#define memory_lock(statement) \
sem_wait(xhubin04_semaphore_mutex); \
statement; \
sem_post(xhubin04_semaphore_mutex);

#endif