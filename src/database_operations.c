#include "../include/database_operations.h"

long int get_size_of_database(database *db) {
    fseek_to_database(db, 0, SEEK_END);
    return ftell_of_database(db);
}

database *open_database(char *filename) {

    database *db = (database *) malloc(sizeof(database));
    db->file_db = fopen(filename, "r+b");
    busy_memory += sizeof(database);

    if (db->file_db) {
        db->scheme = create_database_scheme();
        db->write_buffer = (char *) malloc(buffer_size);
        busy_memory += buffer_size;
        db->write_buffer_count = 0;
        db->read_buffer = (char *) malloc(buffer_size);
        busy_memory += buffer_size;
        db->read_buffer_count = 0;
        db->read_buffer_iter = 0;
        fseek_to_database(db, sizeof(long int), SEEK_SET); // Пропускаем место указания размера схемы
        load_scheme(db, db->scheme);
        return db;
    } else {
        busy_memory -= sizeof(*db);
        free(db);
        return NULL;
    }
}

database *create_database_by_scheme(db_scheme *scheme, char *filename) {

    database *result = (database *) malloc(sizeof(database));
    busy_memory += sizeof(database);
    result->file_db = fopen(filename, "w+b");

    if (result->file_db) {
        long int length_of_scheme;
        result->scheme = scheme;
        result->write_buffer = (char *) malloc(buffer_size);
        busy_memory += buffer_size;

        result->write_buffer_count = 0;
        result->read_buffer = (char *) malloc(buffer_size);
        busy_memory += buffer_size;

        result->read_buffer_count = 0;
        result->read_buffer_iter = 0;
        fseek_to_database(result, sizeof(long int), SEEK_SET);
        save_scheme(result, scheme);

        length_of_scheme = ftell_of_database(result);
        fseek_to_database(result, 0, SEEK_SET);
        fwrite_to_database(&length_of_scheme, sizeof(length_of_scheme), 1, result);
        fflush_to_database(result);

        return result;
    } else {
        busy_memory -= sizeof(*result);
        free(result);
        return NULL;
    }
}

void close_database(database *db) {
    free_database_scheme(db->scheme);
    fclose_database(db);
    free(db->write_buffer);
    free(db->read_buffer);
    busy_memory -= 2 * buffer_size;
    free(db);
    busy_memory -= sizeof(*db);
}

void clear_database_file(database *db, char *filename) {
    fclose_database(db);
    fopen(filename, "w+b");
}



