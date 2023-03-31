#include "../include/query.h"

void go_to_node_set(node_set_result *node_set) {
    node_set->node_scheme->prev_offset = node_set->prev_offset;
    node_set->node_scheme->this_offset = node_set->curr_offset;
}

void free_node_set(node_set_result *node_set) {
    while (node_set != NULL) {
        node_set_result *to_delete = node_set;
        node_set = node_set->next;
        busy_memory -= sizeof(*to_delete);
        free(to_delete);
    }
}

node_set_result *query_search_by_many_args(database *db, node_scheme *result_type, int parameter, va_list args) {

    node_set_result *set;
    node_set_result *set_by_result_type;
    node_scheme *node_schemes;
    condition *conditions;

    if (parameter == 0) {
        return NULL;
    }

    node_schemes = va_arg(args, node_scheme *);
    conditions = va_arg(args, condition *);

    node_set_result *result = NULL;
    node_set_result *prev = NULL;

    go_to_first_node(node_schemes);
    while (open_node(db, node_schemes)) {

        if (check_condition(db, node_schemes, conditions)) {

            node_set_result *item = (node_set_result *) malloc(sizeof(node_set_result));
            busy_memory += sizeof(node_set_result);
            item->node_scheme = node_schemes;
            item->prev_offset = node_schemes->prev_offset;
            item->curr_offset = node_schemes->this_offset;
            item->next = NULL;
            item->prev = prev;
            if (prev != NULL) {
                prev->next = item;
            }
            prev = item;
            if (result == NULL) {
                result = item;
            }
        }
        next_node(db, node_schemes);
    }
    set = result;

    if (result_type != NULL) {
        if (result_type->type_in_string == set->node_scheme->type_in_string) {
            set_by_result_type = set;
        }
    }

    parameter--;

    while (parameter > 0) {

        node_scheme *node_scheme_next;
        condition *condition_next;
        node_set_result *set_first_iter = NULL;
        node_set_result *set_next;
        node_set_result *set_pointer;
        prev = NULL;

        if (set == NULL) {
            return NULL;
        }

        node_scheme_next = va_arg(args, node_scheme *);
        condition_next = va_arg(args, condition *);
        set_pointer = set;

        while (set_pointer != NULL) {

            node_schemes->prev_offset = set_pointer->prev_offset;
            node_schemes->this_offset = set_pointer->curr_offset;

            if (open_node(db, node_schemes)) {

                int i, n;
                float *links = get_list_of_link(db, node_schemes, &n);

                for (i = 0; i < n; i++) {

                    if (links[2 * i] == (float) node_scheme_next->root_offset) {

                        node_set_result *item = (node_set_result *) malloc(sizeof(node_set_result));
                        busy_memory += sizeof(node_set_result);
                        item->node_scheme = node_scheme_next;
                        item->prev_offset = 0;
                        item->curr_offset = (int) links[2 * i + 1];
                        item->next = NULL;
                        item->prev = prev;
                        if (prev != NULL) {
                            prev->next = item;
                        }
                        prev = item;
                        if (set_first_iter == NULL) {
                            set_first_iter = item;
                        }
                    }
                }
                close_node(node_schemes);
                busy_memory -= sizeof(links);
                free(links);
            }
            set_pointer = set_pointer->next;
        }

        if (set_first_iter == NULL) {
            set_next = NULL;
        } else {
            result = NULL;
            prev = NULL;
            if (set_first_iter == NULL) {
                return NULL;
            }
            go_to_first_node(set_first_iter->node_scheme);

            while (set_first_iter != NULL && open_node(db, set_first_iter->node_scheme)) {

                if (set_first_iter->node_scheme->this_offset == set_first_iter->curr_offset) {

                    set_first_iter->prev_offset = set_first_iter->node_scheme->prev_offset;

                    if (check_condition(db, set_first_iter->node_scheme, condition_next)) {
                        node_set_result *item = (node_set_result *) malloc(sizeof(node_set_result));
                        busy_memory += sizeof(node_set_result);
                        item->node_scheme = set_first_iter->node_scheme;
                        item->prev_offset = set_first_iter->node_scheme->prev_offset;
                        item->curr_offset = set_first_iter->node_scheme->this_offset;
                        item->next = NULL;
                        item->prev = prev;
                        if (prev != NULL) {
                            prev->next = item;
                        }
                        prev = item;
                        if (result == NULL) {
                            result = item;
                        }
                    }
                    close_node(set_first_iter->node_scheme);
                    set_first_iter = set_first_iter->next;
                } else {
                    close_node(set_first_iter->node_scheme);
                    next_node(db, set_first_iter->node_scheme);
                }
            }
            set_next = result;
        }

        free_node_set(set);
        free_node_set(set_first_iter);

        if (set_next != NULL) {
            set = set_next;
        }

        if (result_type != NULL) {
            if (result_type->type_in_string == set->node_scheme->type_in_string) {
                set_by_result_type = set;
            }
        }

        node_schemes = node_scheme_next;
        parameter--;
    }

    if (result_type == NULL || set_by_result_type == NULL) {
        return set;
    } else {
        return set_by_result_type;
    }
}

node_set_result *search(database *db, node_scheme *result_type, int parameter, ...) {
    node_set_result *result;
    va_list args;
    va_start(args, parameter);
    result = query_search_by_many_args(db, result_type, parameter, args);
    va_end(args);
    return result;
}


void delete(database *db, node_scheme *result_type, int parameter, ...) {
    node_set_result *set;
    node_set_result *set1;
    va_list args;
    va_start(args, parameter);
    set = query_search_by_many_args(db, result_type, parameter, args);
    set1 = set;
    va_end(args);
    while (set != NULL && set->next != NULL) {
        set = set->next;
    }
    while (set != NULL) {
        go_to_node_set(set);
        delete_node(db, set->node_scheme);
        set = set->prev;
    }
    free_node_set(set1);
}

void update(database *db, node_scheme *result_type, char *attribute_name, float attribute_value, int parameter, ...) {
    node_set_result *set;
    node_set_result *set1;
    va_list args;
    va_start(args, parameter);
    set = query_search_by_many_args(db, result_type, parameter, args);
    set1 = set;
    va_end(args);
    while (set != NULL) {
        go_to_node_set(set);
        if (open_node(db, set->node_scheme)) {
            set_attribute(set->node_scheme, attribute_name, attribute_value);
            commit_node(db, set->node_scheme);
        }
        set = set->next;
    }
    free_node_set(set1);
}