#ifndef SCHEME_OPERATIONS_H
#define SCHEME_OPERATIONS_H

#include "node_internals.h"

extern int relink_table_delta;

typedef struct relink_item {
    node_scheme **place;
    node_scheme *old_value;
    node_scheme *new_value;
} relink_item;

typedef struct database {
    db_scheme *scheme;
    char *write_buffer;
    int write_buffer_count;
    char *read_buffer;
    int read_buffer_count;
    int read_buffer_iter;
    FILE *file_db;
} database;

void fwrite_to_database(void *buf, int item_size, int n_items, database *db);

void fread_from_database(void *buf, int item_size, int n_items, database *db);

void fflush_to_database(database *db);

int feof_of_database(database *db);

void fclose_database(database *db);

long int ftell_of_database(database *db);

void fseek_to_database(database *db, long int offset, int whence);

void create_relinking(relink_item **relink_table, const int *relink_table_size);

void add_relinking(node_scheme **place, node_scheme *old_value, node_scheme *new_value, relink_item **relink_table,
                   int *relink_table_size, int *relink_table_capacity);

void save_link_list(database *db, node_link *list_directed_to);

void save_attribute_list(database *db, struct attribute_entry *list_attribute);

void save_node_scheme(database *db, node_scheme *node_scheme);

void save_scheme(database *db, db_scheme *scheme);

void load_attribute_list(database *db, node_scheme *node_scheme);

void load_directed_list(database *db, node_scheme *node_scheme, relink_item **relink_table, int *relink_table_size,
                        int *relink_table_capacity);

node_scheme *
load_node_scheme(database *db, relink_item **relink_table, int *relink_table_size, int *relink_table_capacity);

void load_scheme(database *db, db_scheme *scheme);

db_scheme *create_database_scheme();

void free_database_scheme(db_scheme *scheme);

#endif //SCHEME_OPERATIONS_H