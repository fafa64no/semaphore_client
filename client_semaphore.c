//
// Created by sebas on 11/5/25.
//

#include "client_semaphore.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define seg_cnt 3

#define SYS_ERR (-1)
#define SYS_ERR_PTR ((void*)-1)
#define MAX_RAND_VALUE (100)
#define CLIENT_REQ_ID_BIT_OFFSET (16)
#define DELAY_BEFORE_FORK 500000

#define TRUE (1)
#define FALSE (0)

#define NB_REQUEST_TEST_SINGLE (1)
#define NB_REQUEST_TEST_MULTIPLE (100)
#define NB_REQUEST_TEST_MANY_MANY (10000)

#define NB_CLIENTS_TO_TEST (100)
#define NB_REQUEST_PER_CLIENT (1000)


sembuf* init_client_semaphore(const int client_id) {
    sembuf* client = malloc(CLIENT_SEMAPHORE_TYPE_SIZE);
    if (client == NULL) {
        fprintf(stderr, "init_client_semaphore: malloc failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    client->pid = getpid();
    if (client->pid == SYS_ERR) {
        fprintf(stderr, "init_client_semaphore: getpid failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    client->semid = semget(
        cle,
        seg_cnt,
        0
    );
    if (client->semid == SYS_ERR) {
        fprintf(stderr, "init_client_semaphore: semget failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    client->shmid = shmget(
        cle,
        segsize,
        0
    );
    if (client->shmid == SYS_ERR) {
        fprintf(stderr, "init_client_semaphore: shmget failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }

    client->request_count = 0;
    client->client_id = client_id;
    attach_client_semaphore(client);
    return client;
}

void delete_client_semaphore(sembuf* client_semaphore) {
    detach_client_semaphore(client_semaphore);
    if (client_semaphore->request_count > 0) {
        free(client_semaphore->requests);
    }
    free(client_semaphore);
}


void attach_client_semaphore(sembuf* client_semaphore) {
    client_semaphore->client_segment = shmat(
        client_semaphore->shmid,
        NULL,
        0
    );
    if (client_semaphore->client_segment == SYS_ERR_PTR) {
        fprintf(stderr, "attach_client_semaphore: shmat failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }
}

void detach_client_semaphore(const sembuf* client_semaphore) {
    if (shmdt(client_semaphore->client_segment) == SYS_ERR) {
        fprintf(stderr, "detach_client_semaphore: shmdt failed with errno: %d", errno);
    }
}


int get_new_request_id(const int client_id, const int request_id) {
    return request_id | (client_id << CLIENT_REQ_ID_BIT_OFFSET);
}


void prepare_n_requests(sembuf* client_semaphore, const int num_requests) {
    if (client_semaphore->request_count != 0) {
        free(client_semaphore->requests);
        client_semaphore->request_count = 0;
    }

    client_semaphore->requests = malloc(num_requests * sizeof(client_request));
    if (client_semaphore->requests == NULL) {
        fprintf(stderr, "prepare_n_requests: malloc failed with errno: %d", errno);
        exit(EXIT_FAILURE);
    }
    client_semaphore->request_count = num_requests;

    for (int i = 0; i < num_requests; i++) {
        prepare_request(client_semaphore, i);
    }
}

void prepare_request(const sembuf* client_semaphore, const int request_id) {
    if (request_id >= client_semaphore->request_count) {
        fprintf(
            stderr,
            "start_transaction_client_semaphore: Can't start request %d, only %d requests allocated",
            request_id,
            client_semaphore->request_count
        );
        exit(EXIT_FAILURE);
    }
    client_request* request = &client_semaphore->requests[request_id];

    request->theorical_result = 0;
    for (int i = 0; i < maxval; i++) {
        request->tab[i] = getrand() % MAX_RAND_VALUE;
        request->theorical_result += request->tab[i];
    }
    request->theorical_result /= maxval;
    request->request_id = get_new_request_id(client_semaphore->client_id, request_id);
}

void start_transaction_client_semaphore(const sembuf* client_semaphore, const int request_id) {
    if (request_id >= client_semaphore->request_count) {
        fprintf(
            stderr,
            "start_transaction_client_semaphore: Can't start request %d, only %d requests allocated",
            request_id,
            client_semaphore->request_count
        );
        exit(EXIT_FAILURE);
    }
    client_request* request = &client_semaphore->requests[request_id];

    client_semaphore->client_segment->pid = client_semaphore->pid;
    client_semaphore->client_segment->req = request->request_id;

    acq_sem(client_semaphore->semid, seg_dispo);

    memcpy(client_semaphore->client_segment->tab, request->tab, maxval * sizeof(long));
    acq_sem(client_semaphore->semid, seg_init);

    wait_sem(client_semaphore->semid, res_ok);
    request->server_result = client_semaphore->client_segment->result;

    lib_sem(client_semaphore->semid, seg_init);
    lib_sem(client_semaphore->semid, seg_dispo);
}


void print_client_status(const sembuf* client_semaphore) {
    printf(
        "Client created with \n\tsemid:\t%d\n\tshmid:\t%d\n\tsegmt:\t%p\n\tpid:\t%d\n",
        client_semaphore->semid,
        client_semaphore->shmid,
        client_semaphore->client_segment,
        client_semaphore->pid
    );
}

void check_if_request_successful(const sembuf* client_semaphore, const int request_id, const int only_display_failure) {
    if (request_id >= client_semaphore->request_count) {
        fprintf(
            stderr,
            "check_if_request_successful: Can't check request %d, only %d requests allocated",
            request_id,
            client_semaphore->request_count
        );
        exit(EXIT_FAILURE);
    }
    const client_request* request = &client_semaphore->requests[request_id];

    if (request->theorical_result == request->server_result) {
        if (!only_display_failure) {
            printf(
                "Good:\tcl: %ld\tsv: %ld\n",
                request->theorical_result,
                request->server_result
            );
        }
    } else {
        printf(
            "Fail:\tcl: %ld\tsv: %ld\n",
            request->theorical_result,
            request->server_result
        );
    }
}


void single_request_test_client(sembuf* client_semaphore) {
    printf("================== single_request_test_client ==================\n");
    prepare_n_requests(client_semaphore, NB_REQUEST_TEST_SINGLE);
    start_transaction_client_semaphore(client_semaphore, 0);
    check_if_request_successful(client_semaphore, 0, FALSE);
    printf("\n");
}

void test_n_requests(sembuf* client_semaphore, const int num_requests) {
    prepare_n_requests(client_semaphore, num_requests);
    for (int i = 0; i < num_requests; i++) {
        start_transaction_client_semaphore(client_semaphore, i);
        check_if_request_successful(client_semaphore, i, TRUE);
    }
}

void multiple_request_test_client(sembuf* client_semaphore) {
    printf("================== multiple_request_test_client ==================\n");
    test_n_requests(client_semaphore, NB_REQUEST_TEST_MULTIPLE);
    printf("\n");
}

void many_many_request_test_client(sembuf* client_semaphore) {
    printf("================== many_many_request_test_client ==================\n");
    test_n_requests(client_semaphore, NB_REQUEST_TEST_MANY_MANY);
    printf("\n");
}


void multiple_client_test() {
    printf("================== multiple_client_test ==================\n");
    usleep(DELAY_BEFORE_FORK);

    for (int i = 0; i < NB_CLIENTS_TO_TEST; i++) {
        const pid_t pid = fork();
        if (pid == SYS_ERR) {
            fprintf(stderr, "multiple_client_test: fork failed with errno: %d", errno);
            continue;
        }

        if (pid != 0) {
            sembuf* client = init_client_semaphore(i);
            test_n_requests(client, NB_REQUEST_PER_CLIENT);
            delete_client_semaphore(client);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < NB_CLIENTS_TO_TEST; i++) {
        int status;
        wait(&status);
    }

    printf("\n");
}






