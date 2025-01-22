#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    int id;
    const char* type;
    int signal_strength;
    bool is_connected;
} Network;

const char* select_best_network(Network networks[], int size);
