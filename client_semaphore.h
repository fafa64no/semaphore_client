//
// Created by sebas on 11/5/25.
//

#ifndef SEMAPHORE_CLIENT_CLIENT_SEMAPHORE_H
#define SEMAPHORE_CLIENT_CLIENT_SEMAPHORE_H

#include "segdef.h"

typedef struct client_request_struct {
    long tab[maxval];
    long theorical_result;
    long server_result;

    int request_id;
} client_request;

typedef struct client_semaphore_struct {
    segment* client_segment;
    int semid;
    int shmid;
    pid_t pid;

    client_request* requests;
    int request_count;
} sembuf;

#define CLIENT_SEMAPHORE_TYPE_SIZE sizeof(sembuf)

sembuf* init_client_semaphore();
void delete_client_semaphore(sembuf* client_semaphore);

void attach_client_semaphore(sembuf* client_semaphore);
void detach_client_semaphore(const sembuf* client_semaphore);

int get_new_req(int request_id);

void prepare_n_requests(sembuf* client_semaphore, int num_requests);
void prepare_request(const sembuf* client_semaphore, int num_requests);
void start_transaction_client_semaphore(const sembuf* client_semaphore, int request_id);

void print_client_status(const sembuf* client_semaphore);
void check_if_request_successful(const sembuf* client_semaphore, int request_id, int only_display_failure);

void single_request_test_client(sembuf* client_semaphore);
void test_n_requests(sembuf* client_semaphore, int num_requests);
void multiple_request_test_client(sembuf* client_semaphore);
void many_many_request_test_client(sembuf* client_semaphore);

#endif //SEMAPHORE_CLIENT_CLIENT_SEMAPHORE_H