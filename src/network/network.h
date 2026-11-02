#ifndef HAMLAB_NETWORK_H
#define HAMLAB_NETWORK_H
#include <stdbool.h>
struct hamlab_network_status {
    const char *mode, *state;
    int attempts, error;
};
void hamlab_network_status_get(struct hamlab_network_status *status);
int hamlab_network_boot(bool ap);
int hamlab_network_start(void);
#endif
