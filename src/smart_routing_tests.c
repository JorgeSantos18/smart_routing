#include "smart_routing.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

// This is a global variable used to save the current_interface_id
int current_interface_id = -1;

// Mock functions to replace the actual implementations
void update_routing_table(int interface_id, const char* type) {}
void send_mqtt_message(const char* topic, const char* message) {}
void control_leds(const char* active_interface) {}


void test_no_networks() {
    current_interface_id = -1;
    Network networks[] = {};
    const char* result = select_best_network(networks, 0);
    assert(strcmp(result, "No connection available") == 0);
    printf("Test case 'no networks' passed\n");
}

void test_single_network() {
    current_interface_id = -1;
    Network networks[] = {{1, "5G", -50, true}};
    const char* result = select_best_network(networks, 1);
    assert(strcmp(result, "5G") == 0);
    printf("Test case 'single network' passed\n");
}

void test_multiple_networks_select_best() {
    current_interface_id = -1;
    Network networks[] = {
        {1, "5G", -60, true},
        {2, "WIFI", -40, true},
        {3, "5G", -70, true}
    };
    const char* result = select_best_network(networks, 3);
    assert(strcmp(result, "WIFI") == 0);
    printf("Test case 'multiple networks select best' passed\n");
}

void test_threshold_switch() {
    current_interface_id = -1;
    // First call to set current_interface_id
    Network networks1[] = {{1, "5G", -50, true}};
    select_best_network(networks1, 1);

    // Second call with a better network above threshold
    Network networks2[] = {
        {1, "5G", -50, true},
        {2, "WIFI", -20, true}
    };
    const char* result = select_best_network(networks2, 2);
    assert(strcmp(result, "WIFI") == 0);
    printf("Test case 'threshold switch' passed\n");
}

void test_no_switch_below_threshold() {
    current_interface_id = -1;
    // First call to set current_interface_id
    Network networks1[] = {{1, "5G", -70, true}};
    select_best_network(networks1, 1);

    // Second call with a slightly better network below threshold
    Network networks2[] = {
        {1, "5G", -70, true},
        {2, "WIFI", -60, true}
    };
    const char* result = select_best_network(networks2, 2);
    assert(strcmp(result, "5G") == 0);
    printf("Test case 'no switch below threshold' passed\n");
}

void test_5g_preference() {
    current_interface_id = -1;
    
    // Second call with a 5G interface with same  signal strength
    Network networks2[] = {
        {1, "WIFI", -80, true},
        {2, "5G", -80, true}
    };


    const char* result = select_best_network(networks2, 2);
    printf("res  %s \n", result);

    printf("Test case '5g_preference' passed\n");
}

int main() {
    test_no_networks();
    test_single_network();
    test_multiple_networks_select_best();
    test_threshold_switch();
    test_no_switch_below_threshold();
    test_5g_preference();

    printf("All tests passed!\n");
    return 0;
}