#ifndef CYPHER_H
#define CYPHER_H

#include "condition.h"

typedef struct node_set_result {
    node_scheme *node_scheme;
    int prev_offset;
    int curr_offset;
    struct node_set_result *next;
    struct node_set_result *prev;
} node_set_result;


void go_to_node_set(node_set_result *node_set);

void free_node_set(node_set_result *node_set);

node_set_result *search(database *db, node_scheme *result_type, int parameter, ...);

void delete(database *db, node_scheme *result_type, int parameter, ...);

void update(database *db, node_scheme *result_type, char *attribute_name, float attribute_value, int parameter, ...);

#endif //CYPHER_H