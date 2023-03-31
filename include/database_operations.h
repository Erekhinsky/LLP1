#ifndef DATABASE_OPERATIONS_H
#define DATABASE_OPERATIONS_H

#include "scheme_operations.h"

long int get_size_of_database(database *db);

database *open_database(char *filename);

database *create_database_by_scheme(db_scheme *scheme, char *filename);

void close_database(database *db);

void clear_database_file(database *db, char *filename);

#endif //DATABASE_OPERATIONS_H