#include "../include/node_internals.h"

int busy_memory = 0;
int buffer_size = 65536;

int get_busy_memory() {
    return busy_memory;
}

void free_busy_memory(int amount) {
    busy_memory -= amount;
}

void write_buffer(char *buffer, int *buffer_count, float data) {
    char *entry = (char *) &data;
    int i;
    buffer += *buffer_count;
    for (i = 0; i < sizeof(float); i++, buffer++, (*buffer_count)++)
        *buffer = entry[i];
}

attribute_entry *find_attribute_by_name(node_scheme *node_scheme, char *name, int *n) {

    attribute_entry *result = node_scheme->attribute_first;
    *n = 0;

    while (result != NULL)
        if (strcmp(name, result->string_name) == 0)
            return result;
        else {
            result = result->next;
            (*n)++;
        }

    *n = -1;
    return NULL;
}

void free_attribute(attribute_entry *attribute) {
    busy_memory -= (int) strlen(attribute->string_name) + 1;
    free(attribute->string_name);
    busy_memory -= sizeof(attribute_entry);
    free(attribute);
}

attribute_entry *add_attribute_to_node_scheme(node_scheme *node_scheme, char *name, unsigned char type) {

    attribute_entry *result;

    if (node_scheme->attribute_first == NULL || node_scheme->attribute_last == NULL) {

        result = (attribute_entry *) malloc(sizeof(attribute_entry));
        busy_memory += sizeof(attribute_entry);
        node_scheme->attribute_first = result;
        node_scheme->attribute_last = result;

    } else {

        int n;
        if (find_attribute_by_name(node_scheme, name, &n)) {
            return NULL;
        }

        result = (attribute_entry *) malloc(sizeof(attribute_entry));
        busy_memory += sizeof(attribute_entry);
        node_scheme->attribute_last->next = result;
        node_scheme->attribute_last = result;
    }

    result->string_name = (char *) malloc(1 + strlen(name) * sizeof(char));
    busy_memory += (int) (1 + strlen(name) * sizeof(char));
    strcpy(result->string_name, name);
    result->type = type;
    result->next = NULL;

    return result;
}

void delete_attribute_from_node_scheme(node_scheme *node_scheme, attribute_entry *to_delete) {

    if (node_scheme->attribute_first != NULL && node_scheme->attribute_last != NULL) {

        busy_memory -= (int) (1 + strlen(to_delete->string_name));
        free(to_delete->string_name);

        if (node_scheme->attribute_first == node_scheme->attribute_last) {

            if (node_scheme->attribute_first == to_delete) {
                busy_memory -= sizeof(attribute_entry);
                free(node_scheme->attribute_first);
                node_scheme->attribute_first = NULL;
                node_scheme->attribute_last = NULL;
            }

        } else if (node_scheme->attribute_first == to_delete) {

            attribute_entry *deleted = node_scheme->attribute_first;
            node_scheme->attribute_first = node_scheme->attribute_first->next;
            busy_memory -= sizeof(attribute_entry);
            free(deleted);

        } else {

            attribute_entry *prev = node_scheme->attribute_first;

            while (prev != NULL && prev->next != to_delete) {
                prev = prev->next;
            }

            if (prev != NULL) {
                attribute_entry *deleted = prev->next;
                if (node_scheme->attribute_last == to_delete) {
                    node_scheme->attribute_last = prev;
                    prev->next = NULL;
                } else {
                    prev->next = prev->next->next;
                }
                busy_memory -= sizeof(attribute_entry);
                free(deleted);
            }
        }
    }
}

void set_attribute(node_scheme *node_scheme, char *attribute_name, float value) {
    int n;
    if (node_scheme->buffer_count > 0 && find_attribute_by_name(node_scheme, attribute_name, &n)) {
        n *= sizeof(float);
        write_buffer(node_scheme->buffer, &n, value);
    }
}

float get_attribute(node_scheme *node_scheme, char *attribute_name) {
    int n;
    if (node_scheme->buffer_count > 0 && find_attribute_by_name(node_scheme, attribute_name, &n)) {
        float *buf = (float *) node_scheme->buffer;
        return buf[n];
    } else {
        return 0;
    }
}

void free_scheme_node(node_scheme *node_scheme) {

    node_link *link = node_scheme->link_to_first;
    attribute_entry *attribute = node_scheme->attribute_first;
    busy_memory -= (int) (1 + strlen(node_scheme->type_in_string));
    free(node_scheme->type_in_string);

    while (link != NULL) {
        node_link *to_delete = link;
        link = link->next;
        busy_memory -= sizeof(node_link);
        free(to_delete);
    }

    while (attribute != NULL) {
        attribute_entry *to_delete = attribute;
        attribute = attribute->next;
        free_attribute(to_delete);
    }

    busy_memory -= buffer_size;
    free(node_scheme->buffer);
    busy_memory -= sizeof(struct node_scheme);
    free(node_scheme);
}

node_scheme *find_node_scheme_by_type_name(db_scheme *scheme, char *type_name, int *counter) {

    node_scheme *result = scheme->first_node_scheme;
    *counter = 0;

    while (result != NULL) {
        if (strcmp(type_name, result->type_in_string) == 0) {
            return result;
        } else {
            result = result->next_node_scheme;
            (*counter)++;
        }
    }

    *counter = -1;
    return NULL;
}

node_scheme *add_node_type_to_scheme(db_scheme *scheme, char *type_name) {

    node_scheme *result;

    if (scheme->first_node_scheme == NULL || scheme->last_node_scheme == NULL) {
        result = (node_scheme *) malloc(sizeof(node_scheme));
        busy_memory += sizeof(node_scheme);
        scheme->first_node_scheme = result;
        scheme->last_node_scheme = result;
    } else {
        int n;
        if (find_node_scheme_by_type_name(scheme, type_name, &n)) {
            return NULL;
        }
        result = (node_scheme *) malloc(sizeof(node_scheme));
        busy_memory += sizeof(node_scheme);
        scheme->last_node_scheme->next_node_scheme = result;
        scheme->last_node_scheme = result;
    }

    result->type_in_string = (char *) malloc(1 + strlen(type_name) * sizeof(char));
    busy_memory += (int) (1 + strlen(type_name) * sizeof(char));
    strcpy(result->type_in_string, type_name);

    result->root_offset = 0;
    result->first_offset = 0;
    result->last_offset = 0;

    result->buffer = (char *) malloc(sizeof(char) * buffer_size);
    busy_memory += (int) (buffer_size * sizeof(char));

    result->buffer_count = 0;
    result->new_node_flag = 0;
    result->prev_offset = 0;
    result->this_offset = 0;
    result->link_to_first = NULL;
    result->link_to_last = NULL;
    result->attribute_first = NULL;
    result->attribute_last = NULL;
    result->next_node_scheme = NULL;

    return result;
}

void delete_node_type_from_scheme(db_scheme *scheme, node_scheme *node_scheme) {

    if (scheme->first_node_scheme != NULL && scheme->last_node_scheme != NULL) {

        if (scheme->first_node_scheme == scheme->last_node_scheme) {

            if (scheme->first_node_scheme == node_scheme) {
                free_scheme_node(node_scheme);
                scheme->first_node_scheme = NULL;
                scheme->last_node_scheme = NULL;
            }

        } else if (scheme->first_node_scheme == node_scheme) {
            scheme->first_node_scheme = node_scheme->next_node_scheme;
            free_scheme_node(node_scheme);

        } else {

            struct node_scheme *prev = scheme->first_node_scheme;
            while (prev != NULL && prev->next_node_scheme != node_scheme) {
                prev = prev->next_node_scheme;
            }

            if (prev != NULL) {
                if (scheme->last_node_scheme == node_scheme) {
                    scheme->last_node_scheme = prev;
                    prev->next_node_scheme = NULL;
                } else {
                    prev->next_node_scheme = node_scheme->next_node_scheme;
                }
                free_scheme_node(node_scheme);
            }
        }
    }
}

node_link *check_can_linked_to(node_scheme *node_scheme, struct node_scheme *to_node_scheme) {

    node_link *result = node_scheme->link_to_first;

    while (result != NULL) {
        if (to_node_scheme == result->node_scheme) {
            return result;
        } else {
            result = result->next;
        }
    }

    return NULL;
}

node_link *add_link_to_node_scheme(node_scheme *node_scheme, struct node_scheme *to_node_scheme) {

    node_link *link;

    if (check_can_linked_to(node_scheme, to_node_scheme)) {
        return NULL;
    }

    link = (node_link *) malloc(sizeof(node_link));
    busy_memory += sizeof(node_link);
    link->node_scheme = to_node_scheme;
    link->next = NULL;

    if (node_scheme->link_to_first == NULL || node_scheme->link_to_last == NULL) {
        node_scheme->link_to_first = link;
        node_scheme->link_to_last = link;
    } else {
        node_scheme->link_to_last->next = link;
        node_scheme->link_to_last = link;
    }

    return link;
}

void delete_link_from_node_scheme(node_scheme *node_scheme, struct node_scheme *to_delete) {

    if (node_scheme->link_to_first != NULL && node_scheme->link_to_last != NULL) {

        if (node_scheme->link_to_first == node_scheme->link_to_last) {

            if (node_scheme->link_to_first->node_scheme == to_delete) {
                busy_memory -= sizeof(node_link);
                free(node_scheme->link_to_first);
                node_scheme->link_to_first = NULL;
                node_scheme->link_to_last = NULL;
            }

        } else if (node_scheme->link_to_first->node_scheme == to_delete) {
            node_link *deleted = node_scheme->link_to_first;
            node_scheme->link_to_first = node_scheme->link_to_first->next;
            busy_memory -= sizeof(node_link);
            free(deleted);

        } else {

            node_link *prev = node_scheme->link_to_first;
            while (prev != NULL && prev->next->node_scheme != to_delete) {
                prev = prev->next;
            }
            if (prev != NULL) {
                node_link *deleted = prev->next;
                if (node_scheme->link_to_last->node_scheme == to_delete) {
                    node_scheme->link_to_last = prev;
                    prev->next = NULL;
                } else {
                    prev->next = prev->next->next;
                }
                busy_memory -= sizeof(node_link);
                free(deleted);
            }
        }
    }
}