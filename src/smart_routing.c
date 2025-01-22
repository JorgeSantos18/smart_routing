#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdlib.h>
#include <syslog.h>

#include "smart_routing.h"


#define SIGNAL_THRESHOLD 20

#define MAX_LINE 200



extern int  current_interface_id;



void update_routing_table(int interface_id, const char* type);
void send_mqtt_message(const char* topic, const char* message);
void control_leds(const char* active_interface);


char* get_interface_name(int id, const char* type);
char* get_default_gateway();
char* generate_route_command(const char* gateway_ip, const char* interface_name);


const char* select_best_network(Network networks[], int size) {

    int best_signal_strength = -100;
    int current_interface_strength = -100;
    int best_network_id = -1;
    int best_network_idx = -1;
    const char* best_network_type = NULL;
    const char* current_type = NULL;

    bool switch_enable = false;

    syslog(LOG_DEBUG, "Running routing updating...");

    for (int i = 0; i < size; i++) {

        if (networks[i].is_connected) {
          if(networks[i].signal_strength > best_signal_strength) {
            best_signal_strength = networks[i].signal_strength;
            best_network_idx = i;
            best_network_type = networks[i].type;
          } else if (networks[i].signal_strength == best_signal_strength
                     && strcmp(networks[i].type, "5G") == 0) {
            best_network_idx = i;
            best_network_type = networks[i].type;
          }
        }

        if(networks[i].id == current_interface_id) {
            current_interface_strength = networks[i].signal_strength;
            current_type = networks[i].type;
        }
    }

    if(best_network_idx == -1){
        syslog(LOG_DEBUG, "No connection available");
        return "No connection available";
    }


    // Check if we should switch interfaces
    if (current_interface_id == -1) {
        switch_enable = true;
    } else if (current_interface_id != best_network_id) {
        
        if(abs(best_signal_strength - current_interface_strength) >= SIGNAL_THRESHOLD) 
            switch_enable = true;

        //??? Which criteria has precedence:
        //  o The system should avoid switching between interfaces if the signal strength
        //    difference is too small, to avoid constant toggling.
        //  o The selection process should respect the following priority:
        //    If two interfaces have the same signal strength, 5G should be preferred
        //    over Wi-Fi.
        // if(best_signal_strength == current_interface_strength){
        //     if (strcmp(current_type, "5G ")) {
        //         switch_enable = false;
        //     } else if (
        //         strcmp(best_network_type, "5G ")
        //     ) {
        //         switch_enable = true;
        //     }
        // } 
    }

    if(switch_enable) {
        update_routing_table(networks[best_network_idx].id, best_network_type);
        control_leds(best_network_type);
        send_mqtt_message(best_network_type, current_type);

        current_interface_id = networks[best_network_idx].id;
        current_type = best_network_type;
    }

    return current_type;


}


#ifndef TEST_MODE 

void update_routing_table(int interface_id, const char* type) {
    syslog(LOG_DEBUG, "Updating routing table id %d type %s", interface_id, type);

    char * interface_name = get_interface_name(interface_id, type);
    char * gateway = get_default_gateway();

    char * update_routing_table_cmd = generate_route_command(gateway, interface_name);

    if(system(update_routing_table_cmd) != 0) {
        syslog(LOG_ERR, "Update routing command error");
    }
}

void send_mqtt_message(const char* topic, const char* message) {
    syslog(LOG_DEBUG, "Sending MQTT message: Topic: %s, Message: %s\n", topic, message);
    // Implementation depends on  MQTT library
    // Example: mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false);
}

void control_leds(const char* active_interface) {
    if (strcmp(active_interface, "5G") == 0) {
        syslog(LOG_DEBUG, "Turning on green LED for 5G\n");
        // Implementation depends on LED control method
        // Example: system("echo 1 > /sys/class/leds/green/brightness");
        // Example: system("echo 0 > /sys/class/leds/blue/brightness");
    } else {
        syslog(LOG_DEBUG, "Turning on blue LED for WiFi\n");
        // Example: system("echo 0 > /sys/class/leds/green/brightness");
        // Example: system("echo 1 > /sys/class/leds/blue/brightness");
    }
}


char* get_interface_name(int id, const char* type) {
    char* result = NULL;
    char id_str[10];  // Buffer to hold the string representation of id

    // Convert id to string
    snprintf(id_str, sizeof(id_str), "%d", id);

    if (strcmp(type, "5G") == 0) {
        result = malloc(strlen("wwlan") + strlen(id_str) + 1);
        if (result) {
            strcpy(result, "wwan");
            strcat(result, id_str);
        }
    } else if (strcmp(type, "WIFI") == 0) {
        result = malloc(strlen("wlan") + strlen(id_str) + 1);
        if (result) {
            strcpy(result, "wlan");
            strcat(result, id_str);
        }
    }

    return result;
}

char* get_default_gateway() {
    FILE *fp;
    char line[MAX_LINE];
    char *gateway = NULL;
    char *token;

    fp = popen("ip route", "r");
    if (fp == NULL) {
        perror("Failed to run command");
        return NULL;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "default via", 11) == 0) {
            token = strtok(line, " ");
            for (int i = 0; i < 3; i++) {
                token = strtok(NULL, " ");
                if (i == 2) {
                    gateway = strdup(token);
                    break;
                }
            }
            break;
        }
    }

    pclose(fp);
    return gateway;
}

char* generate_route_command(const char* gateway_ip, const char* interface_name) {
    const char* command_template = "ip route replace default via %s dev %s";
    int command_length = snprintf(NULL, 0, command_template, gateway_ip, interface_name) + 1;
    
    char* command = (char*)malloc(command_length);
    if (command == NULL) {
        return NULL;  // Memory allocation failed
    }
    
    snprintf(command, command_length, command_template, gateway_ip, interface_name);
    return command;
}

#endif