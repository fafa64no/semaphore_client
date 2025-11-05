//
// Created by sebas on 11/5/25.
//

#include "client_semaphore.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


sembuf* init_client_semaphore() {
    sembuf* client = malloc(CLIENT_SEMAPHORE_TYPE_SIZE);
    if (client == NULL) {
        fprintf(stderr, "init_client_semaphore: malloc failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    client->client_segment = malloc(segsize);
    if (client->client_segment == NULL) {
        fprintf(stderr, "init_client_semaphore: malloc failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    client->pid = getpid();
    if (client->pid < 0) {
        fprintf(stderr, "init_client_semaphore: getpid failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }
    client->client_segment->pid = client->pid;

    client->shmid = shmget(
        cle,
        segsize,
        IPC_CREAT+0666
    );
    if (client->shmid < 0) {
        fprintf(stderr, "init_client_semaphore: semget failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    return client;
}

void delete_client_semaphore(sembuf* client_semaphore) {
    free(client_semaphore->client_segment);
    free(client_semaphore);
}













