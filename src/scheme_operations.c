#include "../include/scheme_operations.h"

int relink_table_delta = 10;

void create_relinking(relink_item **relink_table, const int *relink_table_size) {

    for (int i = 0; i < *relink_table_size; i++) {

        if ((*relink_table)[i].place != NULL) {

            int j = 0;
            while (j < *relink_table_size && (*relink_table)[j].place != NULL ||
                   (*relink_table)[j].old_value != (*relink_table)[i].old_value) {
                j++;
            }

            *(*relink_table)[i].place = (*relink_table)[j].new_value;
        }
    }
}

void add_relinking(node_scheme **place, node_scheme *old_value, node_scheme *new_value,
                   relink_item **relink_table, int *relink_table_size, int *relink_table_capacity) {

    if (*relink_table_size == *relink_table_capacity) {
        *relink_table_capacity += relink_table_delta;
        *relink_table = (relink_item *) realloc(*relink_table, (*relink_table_capacity) * sizeof(relink_item));
    }

    (*relink_table)[*relink_table_size].place = place;
    (*relink_table)[*relink_table_size].old_value = old_value;
    (*relink_table)[*relink_table_size].new_value = new_value;
    (*relink_table_size)++;
}

void fwrite_to_database(void *buf, int item_size, int n_items, database *db) {

    char *_buf = (char *) buf;
    int n_free = buffer_size - db->write_buffer_count;
    int n_bytes = item_size * n_items;
    int i = db->write_buffer_count;

    if (db->read_buffer_iter < db->read_buffer_count) {
        fseek(db->file_db, db->read_buffer_iter - db->read_buffer_count, SEEK_CUR);
        db->read_buffer_iter = 0;
        db->read_buffer_count = 0;
    }

    if (n_free > 0) {
        int to_write = n_free < n_bytes ? n_free : n_bytes;
        db->write_buffer_count += to_write;
        n_bytes -= to_write;
        for (; to_write > 0; to_write--, i++) {
            db->write_buffer[i] = *_buf++;
        }
    }

    if (db->write_buffer_count == buffer_size) {
        fwrite(db->write_buffer, 1, buffer_size, db->file_db);
        fwrite(_buf, 1, n_bytes, db->file_db);
        db->write_buffer_count = 0;
    }
}

void fread_from_database(void *buf, int item_size, int n_items, database *db) {

    char *_buf = (char *) buf;
    int n_have = db->read_buffer_count - db->read_buffer_iter;
    int n_bytes = item_size * n_items;

    for (; n_bytes > 0 && n_have > 0; n_have--, n_bytes--) {
        *_buf++ = db->read_buffer[db->read_buffer_iter++];
    }

    if (n_bytes > 0) {
        fread(_buf, 1, n_bytes, db->file_db);
        db->read_buffer_iter = 0;
        db->read_buffer_count = (int) fread(db->read_buffer, 1, buffer_size, db->file_db);
    }
}

void fflush_to_database(database *db) {

    if (db->read_buffer_iter < db->read_buffer_count) {
        fseek(db->file_db, db->read_buffer_iter - db->read_buffer_count, SEEK_CUR);
        db->read_buffer_iter = 0;
        db->read_buffer_count = 0;
    }

    if (db->write_buffer_count > 0) {
        fwrite(db->write_buffer, 1, db->write_buffer_count, db->file_db);
        fflush(db->file_db);
        db->write_buffer_count = 0;
    }
}

int feof_of_database(database *db) {
    fflush_to_database(db);
    return feof(db->file_db);
}

void fclose_database(database *db) {
    fflush_to_database(db);
    fclose(db->file_db);
}

long int ftell_of_database(database *db) {
    fflush_to_database(db);
    return ftell(db->file_db);
}

void fseek_to_database(database *db, long int offset, int whence) {
    fflush_to_database(db);
    fseek(db->file_db, offset, whence);
}

void save_link_list(database *db, node_link *list_directed_to) {

    while (list_directed_to != NULL) {
        fwrite_to_database(&list_directed_to->node_scheme, sizeof(*list_directed_to->node_scheme), 1, db);
        list_directed_to = list_directed_to->next;
    }

    fwrite_to_database(&list_directed_to, sizeof(*list_directed_to), 1, db);
}

void save_attribute_list(database *db, attribute_entry *list_attribute) {

    int string_name_length;

    while (list_attribute != NULL) {
        string_name_length = (int) (1 + strlen(list_attribute->string_name));
        fwrite_to_database(&string_name_length, sizeof(string_name_length), 1, db);
        fwrite_to_database(list_attribute->string_name, string_name_length, 1, db);
        fwrite_to_database(&list_attribute->type, sizeof(list_attribute->type), 1, db);
        list_attribute = list_attribute->next;
    }

    string_name_length = 0;
    fwrite_to_database(&string_name_length, sizeof(string_name_length), 1, db);
}

void save_node_scheme(database *db, node_scheme *node_scheme) {

    int type_string_length = (int) (1 + strlen(node_scheme->type_in_string));
    fwrite_to_database(&node_scheme, sizeof(*node_scheme), 1, db);
    fwrite_to_database(&type_string_length, sizeof(type_string_length), 1, db);
    fwrite_to_database(node_scheme->type_in_string, type_string_length, 1, db);
    save_link_list(db, node_scheme->link_to_first);
    save_attribute_list(db, node_scheme->attribute_first);
}

void save_scheme(database *db, db_scheme *scheme) {

    node_scheme *node_scheme = scheme->first_node_scheme;

    while (node_scheme != NULL) {
        save_node_scheme(db, node_scheme);
        node_scheme = node_scheme->next_node_scheme;
    }

    fwrite_to_database(&node_scheme, sizeof(*node_scheme), 1, db);
    node_scheme = scheme->first_node_scheme;

    while (node_scheme != NULL) {
        int empty_offset = 0;
        node_scheme->root_offset = ftell_of_database(db);
        fwrite_to_database(&empty_offset, sizeof(empty_offset), 1, db);
        fwrite_to_database(&empty_offset, sizeof(empty_offset), 1, db);
        node_scheme = node_scheme->next_node_scheme;
    }
}

void load_attribute_list(database *db, node_scheme *node_scheme) {

    int string_name_length;
    char *string_name;
    unsigned char type;

    do {
        fread_from_database(&string_name_length, sizeof(string_name_length), 1, db);
        if (string_name_length > 0) {
            string_name = (char *) malloc(string_name_length * sizeof(char));
            busy_memory += (int) (string_name_length * sizeof(char));
            fread_from_database(string_name, string_name_length, 1, db);
            fread_from_database(&type, sizeof(type), 1, db);
            add_attribute_to_node_scheme(node_scheme, string_name, type);
            busy_memory -= string_name_length;
            free(string_name);
        }
    } while (string_name_length != 0);
}

void load_directed_list(database *db, node_scheme *node_scheme, relink_item **relink_table,
                        int *relink_table_size, int *relink_table_capacity) {
    struct node_scheme *loaded_node_scheme;
    do {
        fread_from_database(&loaded_node_scheme, sizeof(*loaded_node_scheme), 1, db);
        if (loaded_node_scheme) {
            node_link *rec = add_link_to_node_scheme(node_scheme, loaded_node_scheme);
            add_relinking(&rec->node_scheme, loaded_node_scheme, NULL, relink_table,
                          relink_table_size, relink_table_capacity);
        }
    } while (loaded_node_scheme != NULL);
}

node_scheme *load_node_scheme(database *db, relink_item **relink_table, int *relink_table_size,
                              int *relink_table_capacity) {
    node_scheme *loaded_node_scheme;
    node_scheme *node_scheme;
    char *type_string;
    int type_string_length;

    fread_from_database(&loaded_node_scheme, sizeof(*loaded_node_scheme), 1, db);

    if (loaded_node_scheme == NULL) {
        return NULL;
    }

    fread_from_database(&type_string_length, sizeof(type_string_length), 1, db);

    type_string = (char *) malloc(type_string_length * sizeof(char));
    busy_memory += (int) (type_string_length * sizeof(char));

    fread_from_database(type_string, type_string_length, 1, db);

    node_scheme = add_node_type_to_scheme(db->scheme, type_string);
    busy_memory -= type_string_length;
    free(type_string);

    add_relinking(NULL, loaded_node_scheme, node_scheme, relink_table, relink_table_size,
                  relink_table_capacity);

    load_directed_list(db, node_scheme, relink_table, relink_table_size, relink_table_capacity);
    load_attribute_list(db, node_scheme);

    return node_scheme;
}

void load_scheme(database *db, db_scheme *scheme) {

    node_scheme *node_scheme;
    relink_item *relink_table = (relink_item *) malloc(relink_table_delta * sizeof(relink_item));
    busy_memory += (int) (relink_table_delta * sizeof(relink_item));

    int relink_table_capacity = relink_table_delta;
    int relink_table_size = 0;

    while (load_node_scheme(db, &relink_table, &relink_table_size, &relink_table_capacity));
    create_relinking(&relink_table, &relink_table_size);

    busy_memory -= (int) (relink_table_capacity * sizeof(relink_item));
    free(relink_table);

    node_scheme = scheme->first_node_scheme;
    while (node_scheme != NULL) {
        node_scheme->root_offset = ftell_of_database(db);
        fread_from_database(&node_scheme->first_offset, sizeof(node_scheme->first_offset), 1, db);
        fread_from_database(&node_scheme->last_offset, sizeof(node_scheme->last_offset), 1, db);
        node_scheme->this_offset = node_scheme->first_offset;
        node_scheme = node_scheme->next_node_scheme;
    }
}

db_scheme *create_database_scheme() {
    db_scheme *result = (db_scheme *) malloc(sizeof(db_scheme));
    busy_memory += sizeof(db_scheme);
    result->first_node_scheme = NULL;
    result->last_node_scheme = NULL;
    return result;
}

void free_database_scheme(db_scheme *scheme) {

    node_scheme *result = scheme->first_node_scheme;

    while (result != NULL) {
        node_scheme *to_delete = result;
        result = result->next_node_scheme;
        free_scheme_node(to_delete);
    }

    busy_memory -= sizeof(db_scheme);
    free(scheme);
}

