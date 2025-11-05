//
// Created by sebas on 11/5/25.
//

#ifndef SEMAPHORE_CLIENT_CLIENT_SEMAPHORE_H
#define SEMAPHORE_CLIENT_CLIENT_SEMAPHORE_H

#include "segdef.h"

typedef struct client_semaphore_struct {
    segment* client_segment;
    int shmid;
    pid_t pid;
} sembuf;

#define CLIENT_SEMAPHORE_TYPE_SIZE sizeof(sembuf)

sembuf* init_client_semaphore();
void delete_client_semaphore(sembuf* client_semaphore);

#endif //SEMAPHORE_CLIENT_CLIENT_SEMAPHORE_H