#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client_semaphore.h"
#include "segdef.h"

int main() {
    init_rand();

    sembuf* client = init_client_semaphore();
    print_client_status(client);

    single_request_test_client(client);
    multiple_request_test_client(client);
    many_many_request_test_client(client);

    delete_client_semaphore(client);
    return EXIT_SUCCESS;
}