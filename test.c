#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "include/query.h"
#include "include/database_operations.h"

#define ITER_NUM 800
#define CHECK_NUM 30

#define max_rand_str_length 99
#define BILLION  1000000000L

char * generate_random_string() {

    int n = rand() % (max_rand_str_length+1);
    char * result = (char *) malloc(n+1);
    int i;

    for (i = 0; i < n; i++) {
        result[i] = 'A' + (rand() % ('Z' - 'A' + 1));
    }
    result[n] = 0;

    return result;
}

int main() {

    char* filename = "graph.txt";
    db_scheme* scheme;
    database* db;
    node_scheme* email_node;
    node_scheme* message_node;
    condition* condition;
    node_set_result* node_set;

    double time;
    FILE* times;
    char* times_name = "C://Users//dron1//CLionProjects//llp1//times.txt";
    struct timespec start, stop;

    scheme = create_database_scheme();

    email_node = add_node_type_to_scheme(scheme, "Email");
    add_attribute_to_node_scheme(email_node, "Name", attribute_string);
    add_attribute_to_node_scheme(email_node, "Surname", attribute_string);
    add_attribute_to_node_scheme(email_node, "Email_addres", attribute_string);

    message_node = add_node_type_to_scheme(scheme, "Message");
    add_attribute_to_node_scheme(message_node, "Sender", attribute_string);
    add_attribute_to_node_scheme(message_node, "Recipient", attribute_string);
    add_attribute_to_node_scheme(message_node, "Length", attribute_int);
    add_attribute_to_node_scheme(message_node, "Body", attribute_string);

    add_link_to_node_scheme(email_node, message_node);
    add_link_to_node_scheme(message_node, email_node);

    db = create_database_by_scheme(scheme, filename);
    condition = create_condition_on_int_or_bool(operation_not_equal, "Length", 3);

    times = fopen(times_name, "w");

    //insert
    printf("INSERT 1 START\n");
    fprintf(times, "INSERT 1\n");
    fprintf(times, "Num Time Size\n");
    clock_gettime (CLOCK_REALTIME, &start);
    for (int i = 0; i < CHECK_NUM; i++) {

        char *Name = generate_random_string();
        char *Surname = generate_random_string();
        char *Email_addres = generate_random_string();

        create_node(email_node);
        set_attribute(email_node, "Name", (float) create_string(db, Name));
        set_attribute(email_node, "Surname", (float) create_string(db, Surname));
        set_attribute(email_node, "Email_addres", (float) create_string(db, Email_addres));
        commit_node(db, email_node);

        free(Name);
        free(Surname);
        free(Email_addres);

        clock_gettime (CLOCK_REALTIME, &stop);
        time = (double) ( stop.tv_sec - start.tv_sec ) + (double)( stop.tv_nsec - start.tv_nsec ) / (double)BILLION;
        fprintf(times, "%i %lf %li\n", i, time, get_size_of_database(db));
        clock_gettime (CLOCK_REALTIME, &start);

        close_node(email_node);
    }

    //insert #2
    printf("INSERT 2 START\n");
    fprintf(times, "INSERT 2\n");
    fprintf(times, "Num Time Size\n");
    clock_gettime (CLOCK_REALTIME, &start);
    for (int i = 0; i < CHECK_NUM; i++) {

        char* Sender = generate_random_string();
        char* Recipient = generate_random_string();
        char* Body = generate_random_string();

        create_node(message_node);
        set_attribute(message_node, "Sender", (float) create_string(db,Sender));
        set_attribute(message_node, "Recipient", (float) create_string(db, Recipient));
        set_attribute(message_node, "Length", (float) i);
        set_attribute(message_node, "Body", (float) create_string(db, Body));
        commit_node(db, message_node);

        free(Sender);
        free(Recipient);
        free(Body);

        clock_gettime (CLOCK_REALTIME, &stop);
        time = (float) (stop.tv_sec - start.tv_sec) + (double)( stop.tv_nsec - start.tv_nsec ) / (double)BILLION;
        fprintf(times, "%i %lf %li\n", i, time, get_size_of_database(db));
        clock_gettime (CLOCK_REALTIME, &start);

        close_node(message_node);
    }

    //create link
    go_to_first_node(message_node);
    go_to_first_node(email_node);
    for (int i = 0; i < CHECK_NUM; i++) {

        open_node(db, email_node);
        create_link_from_node_to_node(email_node, message_node);
        commit_node(db, email_node);

        open_node(db, message_node);
        create_link_from_node_to_node(message_node, email_node);
        commit_node(db, message_node);

        close_node(email_node);
        close_node(message_node);
    }

    printf("CLEAR START\n");
    go_to_first_node(email_node);
    while (open_node(db, email_node)) {
        delete_node(db, email_node);
    }
    go_to_first_node(message_node);
    while (open_node(db, message_node)) {
        delete_node(db, message_node);
    }

    //select
    printf("SELECT START\n");
    fprintf(times, "SELECT\n");
    fprintf(times, "Num Time\n");
    go_to_first_node(message_node);
    go_to_first_node(email_node);
    clock_gettime (CLOCK_REALTIME, &start);
    for (int i = 0; i < CHECK_NUM; i++) {
        for (int j = 0; j < CHECK_NUM; j++) {
            char* Sender = generate_random_string();
            char* Recipient = generate_random_string();
            char* Body = generate_random_string();

            create_node(message_node);
            set_attribute(message_node, "Sender", (float) create_string(db,Sender));
            set_attribute(message_node, "Recipient", (float) create_string(db, Recipient));
            set_attribute(message_node, "Length", (float) j);
            set_attribute(message_node, "Body", (float) create_string(db, Body));
            commit_node(db, message_node);

            free(Sender);
            free(Recipient);
            free(Body);
            close_node(message_node);
        }

        node_set = search(db, message_node, 1, message_node, condition);
        clock_gettime (CLOCK_REALTIME, &stop);
        time = (float) (stop.tv_sec - start.tv_sec ) + (double)( stop.tv_nsec - start.tv_nsec ) / (double)BILLION;
        fprintf(times, "%i %lf\n", i, time);
        clock_gettime (CLOCK_REALTIME, &start);
        free_node_set(node_set);

    }

//    printf("SOUT\n");
//    go_to_first_node(message_node);
//    while (open_node(db, message_node)) {
//        char* Sender = get_string(db, get_attribute(message_node, "Sender"));
//        char* Recipient = get_string(db, get_attribute(message_node, "Recipient"));
//        float Length = get_attribute(message_node, "Length");
//        printf("Sender:%s Recipient:%s Length:%f\n", Sender, Recipient, Length);
//        free_busy_memory(1 + strlen(Sender));
//        free_busy_memory(1 + strlen(Recipient));
//        free(Sender);
//        free(Recipient);
//        next_node(db, message_node);
//    }

    printf("CLEAR START\n");
    go_to_first_node(email_node);
    while (open_node(db, email_node)) {
        delete_node(db, email_node);
    }
    go_to_first_node(message_node);
    while (open_node(db, message_node)) {
        delete_node(db, message_node);
    }

    clear_database_file(db, filename);

    //update
    printf("UPDATE START\n");
    fprintf(times, "UPDATE\n");
    fprintf(times, "Num Time\n");
    go_to_first_node(message_node);
    go_to_first_node(email_node);
    clock_gettime (CLOCK_REALTIME, &start);

    for (int i = 0; i < CHECK_NUM; i++) {

        for (int j = -1; j < CHECK_NUM; j++) {
            char* Sender = generate_random_string();
            char* Recipient = generate_random_string();
            char* Body = generate_random_string();

            create_node(message_node);
            set_attribute(message_node, "Sender", (float) create_string(db,Sender));
            set_attribute(message_node, "Recipient", (float) create_string(db, Recipient));
            set_attribute(message_node, "Length", (float) j);
            set_attribute(message_node, "Body", (float) create_string(db, Body));
            commit_node(db, message_node);

            free(Sender);
            free(Recipient);
            free(Body);
            close_node(message_node);
        }

        update(db, message_node, "Length", (float)(i+5),1, message_node, condition);
        clock_gettime (CLOCK_REALTIME, &stop);
        time = (double)(stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
        fprintf(times, "%i %lf\n", i, time);
        clock_gettime (CLOCK_REALTIME, &start);
    }

    //clear
    printf("CLEAR START\n");
    go_to_first_node(email_node);
    while (open_node(db, email_node)) {
        delete_node(db, email_node);
    }
    go_to_first_node(message_node);
    while (open_node(db, message_node)) {
        delete_node(db, message_node);
    }

    clear_database_file(db, filename);

    //delete
    printf("DELETE START\n");
    fprintf(times, "DELETE\n");
    fprintf(times, "Num Time\n");
    go_to_first_node(message_node);
    go_to_first_node(email_node);
    struct timespec start_help, stop_help;

    clock_gettime (CLOCK_REALTIME, &start);
    for (int i = 0; i < CHECK_NUM; i++) {
        clock_gettime (CLOCK_REALTIME, &start_help);
        for (int j = -1; j < CHECK_NUM * i; j++) {
            char* Sender = generate_random_string();
            char* Recipient = generate_random_string();
            char* Body = generate_random_string();

            create_node(message_node);
            set_attribute(message_node, "Sender", (float) create_string(db,Sender));
            set_attribute(message_node, "Recipient", (float) create_string(db, Recipient));
            set_attribute(message_node, "Length", (float) j);
            set_attribute(message_node, "Body", (float) create_string(db, Body));
            commit_node(db, message_node);

            free(Sender);
            free(Recipient);
            free(Body);
            close_node(message_node);
        }
        clock_gettime (CLOCK_REALTIME, &stop_help);

        delete(db, message_node, 1, message_node, condition);

        clock_gettime(CLOCK_REALTIME, &stop);
        time = (double) (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / (double) BILLION
                - (double) (stop_help.tv_sec - start_help.tv_sec) + (double) (stop_help.tv_nsec - start_help.tv_nsec) / (double) BILLION;
        fprintf(times, "%i %lf %li\n", i * 500, time, get_size_of_database(db));
        go_to_first_node(message_node);
        while (open_node(db, message_node)) {
            delete_node(db, message_node);
        }
        clock_gettime (CLOCK_REALTIME, &start);

        clear_database_file(db, filename);
        printf("%i ITER COMPLETE\n", i);
    }
    fclose(times);

    close_database(db);
    free_condition(condition);

    if (get_busy_memory() == 0)
        printf("Memory is freed correctly!\n");
    else
        printf("Not freed: %i bytes!\n", get_busy_memory());

    return 0;
}