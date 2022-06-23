#include <unistd.h>
#include <sys/wait.h>

class Connector {
    pid_t tracee_pid;

    Connector() {}

public:
    static Connector& getInstance()
    {
        static Connector instance;
        return instance;
    }

    bool attach(pid_t tracee_pid);
};
