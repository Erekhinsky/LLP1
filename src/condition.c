#include "../include/condition.h"

char *get_string(database *db, int offset) {

    unsigned char type;
    char *result;
    int L;
    int n;

    fseek_to_database(db, offset, SEEK_SET);
    fread_from_database(&n, sizeof(n), 1, db);
    fread_from_database(&type, sizeof(type), 1, db);

    if (type != file_string_entry) {
        return NULL;
    }

    L = (int) (n - sizeof(int) - sizeof(unsigned char));
    result = (char *) malloc(L);
    busy_memory += L;
    fread_from_database(result, L, 1, db);

    return result;
}

int create_string(database *db, char *value) {

    unsigned char type = file_string_entry;
    int L = (int) strlen(value);
    int n = (int) (sizeof(int) + sizeof(unsigned char) + 1 + L);
    int result;

    fseek_to_database(db, 0, SEEK_END);
    result = ftell_of_database(db);
    fwrite_to_database(&n, sizeof(n), 1, db);
    fwrite_to_database(&type, sizeof(type), 1, db);
    fwrite_to_database(value, L + 1, 1, db);
    fflush_to_database(db);

    return result;
}

int check_condition(database *db, node_scheme *node_scheme, condition *condition) {

    condition_operand first_operand;
    condition_operand second_operand;
    int result;

    if (node_scheme->buffer_count == 0) {
        return 0;
    }
    if (condition == NULL) {
        return 1;
    }

    first_operand = *condition->first_operand;
    if (condition->first_operand->operand_type == operand_attribute_name) {
        int i;
        attribute_entry *attribute = find_attribute_by_name(node_scheme,
                                                            condition->first_operand->attribute_name_operand, &i);
        float val = get_attribute(node_scheme, condition->first_operand->attribute_name_operand);

        switch (attribute->type) {
            case attribute_string:
                first_operand.operand_type = operand_string;
                first_operand.string_operand = get_string(db, (int) val);
                break;
            default:
                first_operand.operand_type = operand_number_or_bool;
                first_operand.number_or_bool_operand = val;
                break;
        }
    }

    second_operand = *condition->second_operand;
    if (condition->second_operand->operand_type == operand_attribute_name) {
        int i;
        attribute_entry *attribute = find_attribute_by_name(node_scheme,
                                                            condition->second_operand->attribute_name_operand, &i);
        float val = get_attribute(node_scheme, condition->second_operand->attribute_name_operand);

        switch (attribute->type) {
            case attribute_string:
                second_operand.operand_type = operand_string;
                second_operand.string_operand = get_string(db, (int) val);
                break;
            default:
                second_operand.operand_type = operand_number_or_bool;
                second_operand.number_or_bool_operand = val;
                break;
        }
    }

    switch (condition->operation_type) {
        case operation_equal: {
            if (first_operand.operand_type == second_operand.operand_type) {
                if (first_operand.operand_type == operand_string)
                    result = strcmp(first_operand.string_operand, second_operand.string_operand) == 0;
                else
                    result = first_operand.number_or_bool_operand == second_operand.number_or_bool_operand;
            } else
                result = 0;
            break;
        }
        case operation_not_equal: {
            if (first_operand.operand_type == second_operand.operand_type) {
                if (first_operand.operand_type == operand_string)
                    result = strcmp(first_operand.string_operand, second_operand.string_operand) != 0;
                else
                    result = first_operand.number_or_bool_operand != second_operand.number_or_bool_operand;
            } else
                result = 0;
            break;
        }
        case operation_less: {
            if (first_operand.operand_type == second_operand.operand_type) {
                if (first_operand.operand_type == operand_string)
                    result = strcmp(first_operand.string_operand, second_operand.string_operand) < 0;
                else
                    result = first_operand.number_or_bool_operand < second_operand.number_or_bool_operand;
            } else
                result = 0;
            break;
        }
        case operation_greater: {
            if (first_operand.operand_type == second_operand.operand_type) {
                if (first_operand.operand_type == operand_string)
                    result = strcmp(first_operand.string_operand, second_operand.string_operand) > 0;
                else
                    result = first_operand.number_or_bool_operand > second_operand.number_or_bool_operand;
            } else
                result = 0;
            break;
        }
        case operation_not: {
            result = !check_condition(db, node_scheme, first_operand.another_condition);
            break;
        }
        case operation_and: {
            result = check_condition(db, node_scheme, first_operand.another_condition) &&
                     check_condition(db, node_scheme, second_operand.another_condition);
            break;
        }
        case operation_or: {
            result = check_condition(db, node_scheme, first_operand.another_condition) ||
                     check_condition(db, node_scheme, second_operand.another_condition);
            break;
        }
    }

    if (condition->first_operand->operand_type == operand_attribute_name &&
        first_operand.operand_type == operand_string) {
        busy_memory -= (int) (1 + strlen(first_operand.string_operand));
        free(first_operand.string_operand);
    }
    if (condition->second_operand->operand_type == operand_attribute_name &&
        second_operand.operand_type == operand_string) {
        busy_memory -= (int) (1 + strlen(second_operand.string_operand));
        free(second_operand.string_operand);
    }

    return result;
}

condition *create_condition_with_operation(unsigned char operation, condition *operand1, condition *operand2) {

    condition *result = (condition *) malloc(sizeof(condition));
    condition_operand *_operand1 = (condition_operand *) malloc(sizeof(condition_operand));
    condition_operand *_operand2 = (condition_operand *) malloc(sizeof(condition_operand));
    busy_memory += sizeof(condition) + 2 * sizeof(condition_operand);

    _operand1->operand_type = operand_condition;
    _operand1->another_condition = operand1;

    _operand2->operand_type = operand_condition;
    _operand2->another_condition = operand2;

    result->operation_type = operation;
    result->first_operand = _operand1;
    result->second_operand = _operand2;

    return result;
}

condition *create_condition_on_string(unsigned char operation, char *attribute_name, char *value) {

    condition *result = (condition *) malloc(sizeof(condition));
    condition_operand *operand1 = (condition_operand *) malloc(sizeof(condition_operand));
    condition_operand *operand2 = (condition_operand *) malloc(sizeof(condition_operand));
    busy_memory += sizeof(condition) + 2 * sizeof(condition_operand);

    operand1->operand_type = operand_attribute_name;
    operand1->attribute_name_operand = (char *) malloc((strlen(attribute_name) + 1) * sizeof(char));
    busy_memory += (int) (1 + strlen(attribute_name));
    strcpy(operand1->attribute_name_operand, attribute_name);

    operand2->operand_type = operand_string;
    operand2->string_operand = (char *) malloc((strlen(value) + 1) * sizeof(char));
    busy_memory += (int) (1 + strlen(value));
    strcpy(operand2->string_operand, value);

    result->operation_type = operation;
    result->first_operand = operand1;
    result->second_operand = operand2;

    return result;
}

condition *create_condition_on_float(unsigned char operation, char *attribute_name, float value) {

    condition *result = (condition *) malloc(sizeof(condition));
    condition_operand *operand1 = (condition_operand *) malloc(sizeof(condition_operand));
    condition_operand *operand2 = (condition_operand *) malloc(sizeof(condition_operand));
    busy_memory += sizeof(condition) + 2 * sizeof(condition_operand);

    operand1->operand_type = operand_attribute_name;
    operand1->attribute_name_operand = (char *) malloc((strlen(attribute_name) + 1) * sizeof(char));
    busy_memory += (int) (1 + strlen(attribute_name));
    strcpy(operand1->attribute_name_operand, attribute_name);

    operand2->operand_type = operand_number_or_bool;
    operand2->number_or_bool_operand = value;

    result->operation_type = operation;
    result->first_operand = operand1;
    result->second_operand = operand2;

    return result;
}

condition *create_condition_on_int_or_bool(unsigned char operation, char *attribute_name, int value) {
    return create_condition_on_float(operation, attribute_name, (float) value);
}

void free_operand(condition_operand *operand) {

    switch (operand->operand_type) {

        case operand_condition:
            free_condition(operand->another_condition);
            break;

        case operand_string:
            busy_memory -= (int) (1 + strlen(operand->string_operand));
            free(operand->string_operand);
            break;

        case operand_attribute_name:
            busy_memory -= (int) (1 + strlen(operand->attribute_name_operand));
            free(operand->attribute_name_operand);
            break;
    }

    busy_memory -= sizeof(*operand);
    free(operand);
}

void free_condition(condition *condition) {
    free_operand(condition->first_operand);
    free_operand(condition->second_operand);
    busy_memory -= sizeof(*condition);
    free(condition);
}