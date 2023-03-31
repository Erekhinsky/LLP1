//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#include "include/query.h"
//#include "include/database_operations.h"
//
//int main () {
//
//    char *names[10] = {"Andrey", "Sonya", "Sasha", "Anvar", "Veronica", "Vitaliy",
//                       "Kirill", "Polina", "Danya", "Tanya"};
//    char *surnames[10] = {"Erekhinsky", "Inglikova", "Bogatov", "Gazizov", "Troynikova", "Kiyko",
//                          "Ievlev", "Morgunova", "Menkov", "Ivanova"};
//    char *email_addres[10] = {"dron12301230@yandex.ru", "IS@mail.ru", "BA@ya.ru", "GA@ya.ru", "TV@ya.ru",
//                              "KV@ya.ru", "IK@ya.ru", "MP@ya.ru", "MD@ya.ru", "IT@ya.ru"};
//
//    db_scheme *scheme;
//    database *db;
//    node_scheme *email_node;
//    node_scheme *message_node;
//    condition *condition1;
//    condition *condition2;
//    condition *condition3;
//    node_set_result *node_set_1;
//    node_set_result *node_set_2;
//
//    scheme = create_database_scheme();
//
//    email_node = add_node_type_to_scheme(scheme, "Email");
//    add_attribute_to_node_scheme(email_node, "Name", attribute_string);
//    add_attribute_to_node_scheme(email_node, "Surname", attribute_string);
//    add_attribute_to_node_scheme(email_node, "Email_addres", attribute_string);
//
//    message_node = add_node_type_to_scheme(scheme, "Message");
//    add_attribute_to_node_scheme(message_node, "Sender", attribute_string);
//    add_attribute_to_node_scheme(message_node, "Recipient", attribute_string);
//    add_attribute_to_node_scheme(message_node, "Length", attribute_int);
//
//    add_link_to_node_scheme(email_node, message_node);
//    add_link_to_node_scheme(message_node, email_node);
//
//    db = create_database_by_scheme(scheme, "../graph.txt");
////    db = open_database("../graph.txt");
//
//    for (int i = 0; i < 10; i++) {
//
//        create_node(email_node);
//        set_attribute(email_node, "Name", (float) create_string(db, names[i]));
//        set_attribute(email_node, "Surname", (float) create_string(db, surnames[i]));
//        set_attribute(email_node, "Email_addres", (float) create_string(db, email_addres[i]));
//        commit_node(db, email_node);
//
////        printf("Email: %s, Name: %s, Surname: %s\n",
////               get_string(db, get_attribute(db, email_node, "Email_addres")),
////               get_string(db, get_attribute(db, email_node, "Name")),
////               get_string(db, get_attribute(db, email_node, "Surname")));
//
//        create_node(message_node);
//        set_attribute(message_node, "Sender", (float) create_string(db, email_addres[i]));
//        if (i != 9) {
//            set_attribute(message_node, "Recipient", (float) create_string(db, email_addres[i + 1]));
//        } else {
//            set_attribute(message_node, "Recipient", (float) create_string(db, email_addres[0]));
//        }
//        set_attribute(message_node, "Length", (float) (50 + i));
//        commit_node(db, message_node);
//
////        printf("Sender: %s, Recipient: %s, Length: %f\n",
////               get_string(db, get_attribute(db, message_node, "Sender")),
////               get_string(db, get_attribute(db, message_node, "Recipient")),
////               get_attribute(db, message_node, "Length"));
//
//        open_node(db, email_node);
//        if (!create_link_from_node_to_node(email_node, message_node)) {
//            printf("Can not connect.\n\n");
//        }
//        commit_node(db, email_node);
//        open_node(db, message_node);
//        if (!create_link_from_node_to_node(message_node, email_node)) {
//            printf("Can not connect.\n\n");
//        }
//        commit_node(db, message_node);
//
//        close_node(email_node);
//        close_node(message_node);
//    }
//
//    go_to_first_node(message_node);
//    while (open_node(db, message_node)) {
//        int length;
//        char *Sender;
//        char *Recipient;
//        length = (int) get_attribute(message_node, "Length");
//        Sender = get_string(db, (int) get_attribute(message_node, "Sender"));
//        Recipient = get_string(db, (int) get_attribute(message_node, "Recipient"));
//        printf("Sender:%s Recipient:%s length:%i\n", Sender, Recipient, length);
//        free_busy_memory((int) (1 + strlen(Sender)));
//        free_busy_memory((int) (1 + strlen(Recipient)));
//        free(Sender);
//        free(Recipient);
//        next_node(db, message_node);
//    }
//
//    go_to_first_node(email_node);
//    while (open_node(db, email_node)) {
//        char *Name = get_string(db, (int) get_attribute(email_node, "Name"));
//        char *Surname = get_string(db, (int) get_attribute(email_node, "Surname"));
//        char *Email_addres = get_string(db, (int) get_attribute(email_node, "Email_addres"));
//        printf("Name:%s Surname:%s Email_addres:%s\n", Name, Surname, Email_addres);
//        free_busy_memory((int) (1 + strlen(Name)));
//        free_busy_memory((int) (1 + strlen(Surname)));
//        free_busy_memory((int) (1 + strlen(Email_addres)));
//        free(Name);
//        free(Surname);
//        free(Email_addres);
//        next_node(db, email_node);
//    }
//
//    printf("\nMATCH (e:Email)-[:SEND]->(m:Message) WHERE (e.Name != Sonya) AND (e.Surname != Bogatov) "
//           " AND (m.Length < 55) RETURN e;\n");
//    condition1 = create_condition_on_int_or_bool(operation_less, "Length", 55);
//    condition2 = create_condition_with_operation(operation_and,
//                                                 create_condition_on_string(operation_not_equal, "Name", "Sonya"),
//                                                 create_condition_on_string(operation_not_equal, "Surname", "Bogatov"));
//    node_set_1 = search(db, email_node, 2, message_node, condition1, email_node, condition2);
//    node_set_2 = node_set_1;
//    while (node_set_2 != NULL) {
//        go_to_node_set(node_set_2);
//        if (open_node(db, email_node)) {
//            char *Name = get_string(db, (int) get_attribute(email_node, "Name"));
//            char *Surname = get_string(db, (int) get_attribute(email_node, "Surname"));
//            char *Email_addres = get_string(db, (int) get_attribute(email_node, "Email_addres"));
//            printf("Name:%s Surname:%s Email_addres:%s\n", Name, Surname, Email_addres);
//            free_busy_memory(1 + (int) strlen(Name));
//            free_busy_memory(1 + (int) strlen(Surname));
//            free_busy_memory(1 + (int) strlen(Email_addres));
//            free(Name);
//            free(Surname);
//            free(Email_addres);
//            close_node(email_node);
////            next_node(db, email_node);
//        }
////        if (open_node(db, message_node)) {
////            char* Sender = get_string(db, get_attribute(db, message_node, "Sender"));
////            char* Recipient = get_string(db, get_attribute(db, message_node, "Recipient"));
////            float Length = get_attribute(db, message_node, "Length");
////            printf("Sender:%s Recipient:%s Length:%f\n", Sender, Recipient, Length);
////            free_busy_memory(1 + strlen(Sender));
////            free_busy_memory(1 + strlen(Recipient));
////            free(Sender);
////            free(Recipient);
//////            close_node(db, message_node);
////        }
//        node_set_2 = node_set_2->next;
//    }
//    free_node_set(node_set_1);
//
//    printf("\nMATCH (e:Email)-[:SEND]->(m:Message) WHERE (e.Name != Sonya) AND (e.Surname != Bogatov) "
//           "SET m.Surname=Erekhinsky RETURN e;\n\n");
//    update(db, email_node, "Surname", (float) create_string(db, "Erekhinsky"), 1, email_node, condition2);
//
//    printf("MATCH (e:Email)-[:SEND]->(m:Message) WHERE (e.Name != Sonya) AND (e.Surname != Bogatov) "
//           "RETURN e;\n");
//    node_set_1 = search(db, email_node, 1, email_node, condition2);
//    while (node_set_1 != NULL) {
//        go_to_node_set(node_set_1);
////        if (open_node(db, message_node)) {
////            char *Sender = get_string(db, get_attribute(message_node, "Sender"));
////            char *Recipient = get_string(db, get_attribute(message_node, "Recipient"));
////            float Length = get_attribute(message_node, "Length");
////            printf("Sender:%s Recipient:%s Length:%f\n", Sender, Recipient, Length);
////            free_busy_memory(1 + strlen(Sender));
////            free_busy_memory(1 + strlen(Recipient));
////            free(Sender);
////            free(Recipient);
////            close_node(message_node);
////        }
//        if (open_node(db, email_node)) {
//            char *Name = get_string(db, (int) get_attribute(email_node, "Name"));
//            char *Surname = get_string(db, (int) get_attribute(email_node, "Surname"));
//            char *Email_addres = get_string(db, (int) get_attribute(email_node, "Email_addres"));
//            printf("Name:%s Surname:%s Email_addres:%s\n", Name, Surname, Email_addres);
//            free_busy_memory(1 + (int) strlen(Name));
//            free_busy_memory(1 + (int) strlen(Surname));
//            free_busy_memory(1 + (int) strlen(Email_addres));
//            free(Name);
//            free(Surname);
//            free(Email_addres);
//            close_node(email_node);
//        }
//        node_set_1 = node_set_1->next;
//    }
//    free_node_set(node_set_1);
//
//    printf("\nMATCH (e:Email)-[:SEND]->(m:Message) WHERE (e.Surname == Erekhinsky) DELETE e;\n");
//    condition3 = create_condition_on_string(operation_equal, "Surname", "Erekhinsky");
//    delete(db, email_node, 1, email_node, condition3);
//
//    go_to_first_node(email_node);
//    while (open_node(db, email_node)) {
//        char *Name = get_string(db, (int) get_attribute(email_node, "Name"));
//        char *Surname = get_string(db, (int) get_attribute(email_node, "Surname"));
//        char *Email_addres = get_string(db, (int) get_attribute(email_node, "Email_addres"));
//        printf("Name:%s Surname:%s Email_addres:%s\n", Name, Surname, Email_addres);
//        free_busy_memory(1 + (int) strlen(Name));
//        free_busy_memory(1 + (int) strlen(Surname));
//        free_busy_memory(1 + (int) strlen(Email_addres));
//        free(Name);
//        free(Surname);
//        free(Email_addres);
//        next_node(db, email_node);
//    }
//
//    go_to_first_node(email_node);
//    int i = 0;
//    while (open_node(db, email_node)) {
//        delete_node(db, email_node);
//        i++;
//    }
//    printf("\n%i Email deleted\n", i);
//
//    go_to_first_node(message_node);
//    i = 0;
//    while (open_node(db, message_node)) {
//        delete_node(db, message_node);
//        i++;
//    }
//    printf("%i Message deleted\n", i);
//
//    close_database(db);
//    free_condition(condition1);
//    free_condition(condition2);
//    free_condition(condition3);
//
//    if (get_busy_memory() == 0)
//        printf("Memory freed OK.\n");
//    else
//        printf("%i bytes is not freed.\n", get_busy_memory());
//
//}