#ifndef NODE_OPERATIONS_H
#define NODE_OPERATIONS_H

#include "scheme_operations.h"

extern int reserved_for_links_in_node;

enum {
    file_empty_entry = 0,
    file_string_entry,
    file_node_data_entry
};

void close_node(node_scheme *node_scheme);

void create_node(node_scheme *node_scheme);

int open_node(database *db, node_scheme *node_scheme);

int next_node(database *db, node_scheme *node_scheme);

void go_to_first_node(node_scheme *node_scheme);

int delete_node(database *db, node_scheme *node_scheme);

void commit_node(database *db, node_scheme *node_scheme);

int create_link_from_node_to_node(node_scheme *from_node_scheme, node_scheme *to_node_scheme);

float *get_list_of_link(database *db, node_scheme *node_scheme, int *n);

#endif //NODE_OPERATIONS_H