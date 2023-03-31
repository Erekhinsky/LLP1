#include "../include/node_operations.h"

int reserved_for_links_in_node = 8;

void close_node(node_scheme *node_scheme) {
    node_scheme->buffer_count = 0;
    node_scheme->new_node_flag = 0;
}

void create_node(node_scheme *node_scheme) {

    attribute_entry *attribute_list = node_scheme->attribute_first;
    int i;
    close_node(node_scheme);
    node_scheme->buffer_count = 0;
    node_scheme->new_node_flag = 1;

    while (attribute_list != NULL) {
        write_buffer(node_scheme->buffer, &node_scheme->buffer_count, 0);
        attribute_list = attribute_list->next;
    }

    write_buffer(node_scheme->buffer, &node_scheme->buffer_count, 0);

    for (i = 0; i < reserved_for_links_in_node; i++) {
        write_buffer(node_scheme->buffer, &node_scheme->buffer_count, 0);
        write_buffer(node_scheme->buffer, &node_scheme->buffer_count, 0);
    }
}

int open_node(database *db, node_scheme *node_scheme) {

    unsigned char type;
    int waste;
    int n;

    close_node(node_scheme);

    if (node_scheme->this_offset == 0) {
        return 0;
    }

    node_scheme->buffer_count = 0;
    node_scheme->new_node_flag = 0;
    fseek_to_database(db, node_scheme->this_offset, SEEK_SET);
    fread_from_database(&n, sizeof(n), 1, db);
    fread_from_database(&type, sizeof(type), 1, db);

    if (type != file_node_data_entry) {
        return 0;
    }

    fread_from_database(&waste, sizeof(int), 1, db);
    node_scheme->buffer_count = (int) (n - sizeof(n) - sizeof(type) - sizeof(int));
    fread_from_database(node_scheme->buffer, node_scheme->buffer_count, 1, db);

    return 1;
}

int next_node(database *db, node_scheme *node_scheme) {

    close_node(node_scheme);

    if (node_scheme->first_offset == 0 || node_scheme->last_offset == 0 || node_scheme->this_offset == 0) {
        return 0;
    }

    fseek_to_database(db, (int) (node_scheme->this_offset + sizeof(int) + sizeof(unsigned char)), SEEK_SET);
    node_scheme->prev_offset = node_scheme->this_offset;
    fread_from_database(&node_scheme->this_offset, sizeof(int), 1, db);

    return 1;
}

void go_to_first_node(node_scheme *node_scheme) {
    close_node(node_scheme);
    node_scheme->this_offset = node_scheme->first_offset;
    node_scheme->prev_offset = 0;
}

int delete_node(database *db, node_scheme *node_scheme) {

    unsigned char type = file_empty_entry;
    int after_deleted_node_offset;
    close_node(node_scheme);

    if (node_scheme->first_offset == 0 || node_scheme->last_offset == 0 || node_scheme->this_offset == 0) {
        return 0;
    }

    fseek_to_database(db, (int) (node_scheme->this_offset + sizeof(int)), SEEK_SET);
    fwrite_to_database(&type, sizeof(type), 1, db);
    fflush_to_database(db);
    fread_from_database(&after_deleted_node_offset, sizeof(after_deleted_node_offset), 1, db);

    if (node_scheme->first_offset == node_scheme->last_offset) {

        int empty_offset = 0;
        fseek_to_database(db, node_scheme->root_offset, SEEK_SET);
        fwrite_to_database(&empty_offset, sizeof(empty_offset), 1, db);
        node_scheme->first_offset = 0;
        fwrite_to_database(&empty_offset, sizeof(empty_offset), 1, db);
        node_scheme->last_offset = 0;
        node_scheme->prev_offset = 0;
        node_scheme->this_offset = 0;

    } else if (node_scheme->first_offset == node_scheme->this_offset) {

        fseek_to_database(db, node_scheme->root_offset, SEEK_SET);
        fwrite_to_database(&after_deleted_node_offset, sizeof(after_deleted_node_offset), 1, db);
        node_scheme->first_offset = after_deleted_node_offset;
        node_scheme->prev_offset = 0;
        node_scheme->this_offset = after_deleted_node_offset;

    } else if (node_scheme->last_offset == node_scheme->this_offset) {

        int empty_offset = 0;
        fseek_to_database(db, (int) (node_scheme->prev_offset + sizeof(int) + sizeof(unsigned char)), SEEK_SET);
        fwrite_to_database(&empty_offset, sizeof(empty_offset), 1, db);
        fseek_to_database(db, (int) (node_scheme->root_offset + sizeof(int)), SEEK_SET);
        fwrite_to_database(&node_scheme->prev_offset, sizeof(int), 1, db);
        node_scheme->last_offset = node_scheme->prev_offset;
        node_scheme->prev_offset = 0;
        node_scheme->this_offset = 0;

    } else {

        fseek_to_database(db, (int) (node_scheme->prev_offset + sizeof(int) + sizeof(unsigned char)), SEEK_SET);
        fwrite_to_database(&after_deleted_node_offset, sizeof(after_deleted_node_offset), 1, db);
        node_scheme->this_offset = after_deleted_node_offset;
    }

    fflush_to_database(db);
    return 1;
}

void commit_node(database *db, node_scheme *node_scheme) {

    if (node_scheme->buffer_count > 0) {

        if (node_scheme->new_node_flag) {

            int n = (int) (sizeof(int) + sizeof(unsigned char) + sizeof(int) + node_scheme->buffer_count);
            unsigned char type = file_node_data_entry;
            int offset_of_empty = 0;
            fseek_to_database(db, 0, SEEK_END);
            node_scheme->this_offset = ftell_of_database(db);

            fwrite_to_database(&n, sizeof(n), 1, db);
            fwrite_to_database(&type, sizeof(type), 1, db);
            fwrite_to_database(&offset_of_empty, sizeof(offset_of_empty), 1, db);
            fwrite_to_database(node_scheme->buffer, node_scheme->buffer_count, 1, db);
            node_scheme->prev_offset = node_scheme->last_offset;

            if (node_scheme->first_offset == 0 && node_scheme->last_offset == 0) {
                fseek_to_database(db, node_scheme->root_offset, SEEK_SET);
                fwrite_to_database(&node_scheme->this_offset, sizeof(int), 1, db);
                fwrite_to_database(&node_scheme->this_offset, sizeof(int), 1, db);
                node_scheme->first_offset = node_scheme->this_offset;
                node_scheme->last_offset = node_scheme->this_offset;
            } else {
                fseek_to_database(db, (int) (node_scheme->prev_offset + sizeof(int) + sizeof(unsigned char)), SEEK_SET);
                fwrite_to_database(&node_scheme->this_offset, sizeof(int), 1, db);
                fseek_to_database(db, (int) (node_scheme->root_offset + sizeof(int)), SEEK_SET);
                fwrite_to_database(&node_scheme->this_offset, sizeof(int), 1, db);
                node_scheme->last_offset = node_scheme->this_offset;
            }
            node_scheme->new_node_flag = 0;
        } else {
            fseek_to_database(db, (int) (node_scheme->this_offset + sizeof(int) + sizeof(unsigned char) + sizeof(int)),
                              SEEK_SET);
            fwrite_to_database(node_scheme->buffer, node_scheme->buffer_count, 1, db);
        }
        fflush_to_database(db);
    }
}

int create_link_from_node_to_node(node_scheme *from_node_scheme, node_scheme *to_node_scheme) {

    attribute_entry *attribute_list = from_node_scheme->attribute_first;
    node_link *directed_list = from_node_scheme->link_to_first;
    float *buf;
    int n, i;
    int offs = 0;

    while (directed_list != NULL && directed_list->node_scheme != to_node_scheme) {
        directed_list = directed_list->next;
    }

    if (directed_list == NULL) {
        return 0;
    }

    while (attribute_list != NULL) {
        offs += sizeof(float);
        attribute_list = attribute_list->next;
    }

    buf = (float *) (from_node_scheme->buffer + offs);
    n = (int) *buf;
    for (i = 0; i < n; i++) {
        if (buf[2 * i + 1] == (float) to_node_scheme->root_offset &&
            buf[2 * i + 2] == (float) to_node_scheme->this_offset) {
            return 1;
        }
    }
    if (n < reserved_for_links_in_node) {
        (*buf)++;
        buf[2 * n + 1] = (float) to_node_scheme->root_offset;
        buf[2 * n + 2] = (float) to_node_scheme->this_offset;
        return 1;
    } else {
        return 0;
    }
}


float *get_list_of_link(database *db, node_scheme *node_scheme, int *n) {

    attribute_entry *attribute_list = node_scheme->attribute_first;
    float *buffer;
    float *result = NULL;
    int offset = 0;

    while (attribute_list != NULL) {
        offset += sizeof(float);
        attribute_list = attribute_list->next;
    }

    buffer = (float *) (node_scheme->buffer + offset);
    *n = (int) *buffer;

    if (*n != 0) {

        int iter;
        int n_bytes = (int) (2 * (*n) * sizeof(float));
        result = (float *) malloc(n_bytes);
        busy_memory += n_bytes;

        for (iter = 0; iter < *n; iter++) {

            unsigned char Type;
            fseek_to_database(db, (int) (buffer[2 * iter + 2] + sizeof(int)), SEEK_SET);
            fread_from_database(&Type, sizeof(Type), 1, db);

            if (Type != file_node_data_entry) {
                result[2 * iter] = 0;
                result[2 * iter + 1] = 0;
            } else {
                result[2 * iter] = buffer[2 * iter + 1];
                result[2 * iter + 1] = buffer[2 * iter + 2];
            }
        }
    }
    return result;
}