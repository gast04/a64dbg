#include <stdio.h>

#include "Connector.h"

bool Connector::attach(pid_t tracee_pid) {

    printf("[*] Attaching to PID: %d\n", tracee_pid);

    int wait_status;
    wait(&wait_status);

    while(WIFSTOPPED(wait_status)) {
        printf("WAIT STATUS %d\n" , wait_status);

        return true;
    }

    return true;
}
