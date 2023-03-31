#ifndef NODE_INTERNALS_H
#define NODE_INTERNALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

extern int buffer_size;

extern int busy_memory;

typedef struct attribute_entry {
    char *string_name;
    unsigned char type;
    struct attribute_entry *next;
} attribute_entry;

typedef struct node_link {
    struct node_scheme *node_scheme;
    struct node_link *next;
} node_link;

typedef struct node_scheme {
    char *type_in_string;
    int root_offset;
    int first_offset;
    int last_offset;
    char *buffer;
    int buffer_count;
    int new_node_flag;
    int prev_offset;
    int this_offset;
    node_link *link_to_first;
    node_link *link_to_last;
    struct attribute_entry *attribute_first;
    struct attribute_entry *attribute_last;
    struct node_scheme *next_node_scheme;
} node_scheme;

typedef struct db_scheme {
    node_scheme *first_node_scheme;
    node_scheme *last_node_scheme;
} db_scheme;

int get_busy_memory();

void free_busy_memory(int);

void write_buffer(char *, int *, float);

void free_attribute(attribute_entry *attribute);

void free_scheme_node(node_scheme *node_scheme);

attribute_entry *find_attribute_by_name(node_scheme *node_scheme, char *name, int *n);

void set_attribute(node_scheme *node_scheme, char *attribute_name, float value);

float get_attribute(node_scheme *node_scheme, char *attribute_name);

node_link *check_can_linked_to(node_scheme *node_scheme, struct node_scheme *to_node_scheme);

node_scheme *find_node_scheme_by_type_name(db_scheme *scheme, char *type_name, int *counter);

node_scheme *add_node_type_to_scheme(db_scheme *scheme, char *type_name);

void delete_node_type_from_scheme(db_scheme *scheme, node_scheme *node_scheme);

node_link *add_link_to_node_scheme(node_scheme *node_scheme, struct node_scheme *to_node_scheme);

void delete_link_from_node_scheme(node_scheme *node_scheme, struct node_scheme *to_delete);

attribute_entry *add_attribute_to_node_scheme(node_scheme *node_scheme, char *name, unsigned char type);

void delete_attribute_from_node_scheme(node_scheme *node_scheme, attribute_entry *to_delete);

#endif //NODE_INTERNALS_H