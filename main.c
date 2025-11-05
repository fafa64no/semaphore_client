#include <stdio.h>
#include <stdlib.h>

#include "client_semaphore.h"
#include "segdef.h"

int main() {
    printf("Starting program");

    sembuf* client = init_client_semaphore();
    init_rand();

    delete_client_semaphore(client);
    return EXIT_SUCCESS;
}