#ifndef LLP1_CONDITION_H
#define LLP1_CONDITION_H

#include "node_operations.h"

enum {
    operand_number_or_bool,
    operand_string,
    operand_attribute_name,
    operand_condition
};

enum {
    attribute_int,
    attribute_float,
    attribute_string,
    attribute_boolean
};

enum {
    operation_equal,
    operation_not_equal,
    operation_less,
    operation_greater,
    operation_not,
    operation_and,
    operation_or
};

typedef struct condition_operand {
    unsigned char operand_type;
    union {
        struct condition *another_condition;
        char *string_operand;
        float number_or_bool_operand;
        char *attribute_name_operand;
    };
} condition_operand;

typedef struct condition {
    unsigned char operation_type;
    condition_operand *first_operand;
    condition_operand *second_operand;
} condition;

char *get_string(database *db, int offset);

int create_string(database *db, char *value);

int check_condition(database *db, node_scheme *node_scheme, condition *condition);

condition *create_condition_with_operation(unsigned char operation, condition *operand1, condition *operand2);

condition *create_condition_on_string(unsigned char, char *, char *);

condition *create_condition_on_int_or_bool(unsigned char, char *, int);

condition *create_condition_on_float(unsigned char, char *, float);

void free_operand(condition_operand *operand);

void free_condition(condition *condition);

#endif //LLP1_CONDITION_H
