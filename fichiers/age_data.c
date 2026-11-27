#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "age_data.h"
#include "main.h"
#include "constante.h"

#define BAR_WIDTH 30          /* taille maximum de la pyramide */
#define INPUT_LEN 16          /* taille du buffer d'entrée du menu */

/* Resultat de la lecture d'un champ entier */
#define FIELD_OK      0
#define FIELD_EMPTY   1
#define FIELD_INVALID 2

const char *AGE_TYPE_NAMES[NB_USER_TYPES]  = {"Conducteur", "Passager", "Pieton"};
const char *AGE_GENDER_NAMES[NB_GENDERS]   = {"Homme", "Femme", "Non releve"};
const char *AGE_TYPE_COLORS[NB_USER_TYPES] = {ANSI_GREEN, ANSI_YELLOW, ANSI_RED};

/* Lit un entier à partir d'un champ de texte. */
int age_read_int_field(const char *field, int *value){
    if (field[0] == '\0') {
        return FIELD_EMPTY;
    }

    char *endptr;
    long v = strtol(field, &endptr, 10);

    if (*endptr != '\0') {
        return FIELD_INVALID;
    }

    *value = (int)v;
    return FIELD_OK;
}

/* Recherche le nœud correspondant à un âge dans la liste (triée par âge décroissant),
   et le crée s'il n'existe pas. */
AgeNode *age_find_or_create_node(AgeNode **head, int age){
    AgeNode *previous = NULL;
    AgeNode *current = *head;

    while (current != NULL && current->age > age) {
        previous = current;
        current = current->next;
    }

    if (current != NULL && current->age == age) {
        return current;
    }

    AgeNode *new_node = calloc(1, sizeof(AgeNode));

    if (new_node == NULL) {
        return NULL;
    }

    new_node->age  = (unsigned char)age;
    new_node->next = current;

    if (previous == NULL) {
        *head = new_node;
    } else {
        previous->next = new_node;
    }

    return new_node;
}

/* Libère la mémoire utilisée par la liste. */
void age_free_list(AgeNode *head){
    while (head != NULL) {
        AgeNode *next = head->next;
        free(head);
        head = next;
    }
}

/* Ecris un message d'erreur (avec le numéro de ligne si connu) et quitte. */
void age_fatal_error(const char *message, long line_number){
    if (line_number > 0) {
        fprintf(stderr, "Erreur (ligne %ld) : %s\n", line_number, message);
    } else {
        fprintf(stderr, "Erreur : %s\n", message);
    }
    exit(EXIT_FAILURE);
}

/* Découpe une ligne en place sur ';'. Renvoie le nombre de champs. */
int age_split_line(char *line, char *fields[NB_CSV_FIELDS]){
    int n = 0;
    char *start = line;

    while (n < NB_CSV_FIELDS) {
        fields[n++] = start;
        char *separator = strchr(start, ';');
        if (separator == NULL) {
            return n;
        }
        *separator = '\0';
        start = separator + 1;
    }
    /* on sort de la boucle après le 9ème champ meme si un  ';' le suit:
       il y a trop de colonnes */
    return n + 1;
}

/* Traite une ligne de données et met à jour la liste. */
void age_process_line(AgeNode **head, char *line, long line_number){
    char *fields[NB_CSV_FIELDS];
    int year = 0, birth_year = 0, user_type = 0, gender = 0;

    if (age_split_line(line, fields) != NB_CSV_FIELDS) {
        age_fatal_error("nombre de colonnes incorrect (9 attendues).", line_number);
    }

    /* Colonne 0: year, 4: user_type, 6: gender, 7: birth_year */
    if (age_read_int_field(fields[0], &year) != FIELD_OK) {
        age_fatal_error("annee (year) invalide.", line_number);
    }
    if (age_read_int_field(fields[4], &user_type) == FIELD_INVALID) {
        age_fatal_error("type d'usager (user_type) invalide.", line_number);
    }
    int gender_result = age_read_int_field(fields[6], &gender);
    if (gender_result == FIELD_INVALID) {
        age_fatal_error("genre (gender) invalide.", line_number);
    }
    int birth_result = age_read_int_field(fields[7], &birth_year);
    if (birth_result == FIELD_INVALID) {
        age_fatal_error("annee de naissance (birth_year) invalide.", line_number);
    }

    /* Données inutilisables pour la pyramide : la ligne est ignorée */
    if (birth_result != FIELD_OK || user_type < 1 || user_type > NB_USER_TYPES) {
        return;
    }
    int age = year - birth_year;
    if (age < 0 || age > AGE_MAX) { // change ça alex stp jsp pourquoi ya ca
        return;
    }

    /* genre: 1 = Homme, 2 = Femme, tout les autres = non releve */
    int gender_index = NB_GENDERS - 1;
    if (gender_result == FIELD_OK && (gender == 1 || gender == 2)) {
        gender_index = gender - 1;
    }

    AgeNode *node = age_find_or_create_node(head, age);
    if (node == NULL) {
        age_fatal_error("memoire insuffisante.", 0);
    }
    node->count[user_type - 1][gender_index]++;
}

/* Charge le fichier CSV complet dans la liste chaînée. */
AgeNode *age_load_data(const char *file){
    FILE *f = fopen(file, "r");
    if (f == NULL) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier \"%s\".\n", file);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    long line_number = 0;
    AgeNode *head = NULL;

    /* En tete obligatoire */
    if (fgets(line, sizeof(line), f) == NULL) {
        age_fatal_error("fichier vide.", 0);
    }
    line_number++;
    if (strncmp(line, "year;", 5) != 0) {
        age_fatal_error("en-tete CSV invalide (\"year;collision_id;...\" attendu).", line_number);
    }

    while (fgets(line, sizeof(line), f) != NULL) {
        line_number++;

        size_t length = strcspn(line, "\r\n");
        if (line[length] == '\0' && !feof(f)) {
            age_fatal_error("ligne trop longue.", line_number);
        }
        line[length] = '\0';

        if (length == 0) {
            continue; /* Ligne vide */
        }
        age_process_line(&head, line, line_number);
    }

    fclose(f);
    return head;
}

/* Additionne les comptages d'un âge selon les filtres
   (-1 = pas de filtre, sinon l'indice du type / du genre). */
void age_count(const AgeNode *node, int type_filter, int gender_filter,
               unsigned long by_type[NB_USER_TYPES],
               unsigned long by_gender[NB_GENDERS]){
    for (int t = 0; t < NB_USER_TYPES; t++) {
        by_type[t] = 0;
    }
    for (int g = 0; g < NB_GENDERS; g++) {
        by_gender[g] = 0;
    }
    for (int t = 0; t < NB_USER_TYPES; t++) {
        if (type_filter >= 0 && type_filter != t) {
            continue;
        }
        for (int g = 0; g < NB_GENDERS; g++) {
            if (gender_filter >= 0 && gender_filter != g) {
                continue;
            }
            by_type[t]   += node->count[t][g];
            by_gender[g] += node->count[t][g];
        }
    }
}

/* Longeur de la barre, mise à l'échelle sur la valeur maximale (au moins 1 si valeur > 0). */
int age_bar_width(unsigned long value, unsigned long max){
    if (value == 0 || max == 0) {
        return 0;
    }
    int width = (int)(value * BAR_WIDTH / max);
    return width < 1 ? 1 : width;
}

/* Affiche une barre colorée. */
void age_print_bar(const char *color, int length){
    printf("%s", color);
    for (int i = 0; i < length; i++) {
        putchar('#');
    }
    printf(ANSI_RESET);
}

/* Affiche les filtres actifs. */
void age_print_filters(int type_filter, int gender_filter){
    printf("  Filtres : usager = %s, genre = %s\n",
           type_filter   < 0 ? "tous" : AGE_TYPE_NAMES[type_filter],
           gender_filter < 0 ? "tous" : AGE_GENDER_NAMES[gender_filter]);
}

/* Pyramide par type d'usager: une barre par âge, divisée en
   conducteur / passager / piéton. Pourcentage = part de l'âge dans la
   population sélectionnée. */
void age_print_type_pyramid(const AgeNode *head, int type_filter, int gender_filter){
    unsigned long by_type[NB_USER_TYPES], by_gender[NB_GENDERS];
    unsigned long total = 0, max = 0;

    /* 1er passage : total et âge le plus représenté (pour l'échelle) */
    for (const AgeNode *n = head; n != NULL; n = n->next) {
        age_count(n, type_filter, gender_filter, by_type, by_gender);
        unsigned long sum = by_type[0] + by_type[1] + by_type[2];
        total += sum;
        if (sum > max) {
            max = sum;
        }
    }

    printf(ANSI_BOLD ANSI_BRIGHT_CYAN "\n  --- Pyramide des ages par type d'usager ---\n" ANSI_RESET);
    age_print_filters(type_filter, gender_filter);
    if (total == 0) {
        printf("  Aucune donnee pour cette selection.\n");
        return;
    }
    printf("  Legende : ");
    for (int t = 0; t < NB_USER_TYPES; t++) {
        printf("%s# %s" ANSI_RESET "  ", AGE_TYPE_COLORS[t], AGE_TYPE_NAMES[t]);
    }
    printf("\n  Effectif total : %lu\n\n", total);

    /* 2eme passage: affichage */
    for (const AgeNode *n = head; n != NULL; n = n->next) {
        age_count(n, type_filter, gender_filter, by_type, by_gender);
        unsigned long sum = by_type[0] + by_type[1] + by_type[2];
        if (sum == 0) {
            continue;
        }
        printf("  %3d ans | ", n->age);
        int used = 0;
        for (int t = 0; t < NB_USER_TYPES; t++) {
            int width = age_bar_width(by_type[t], max);
            age_print_bar(AGE_TYPE_COLORS[t], width);
            used += width;
        }
        printf("%*s %6.2f%%\n", BAR_WIDTH + 3 - used, "", 100.0 * (double)sum / (double)total);
    }
}

/* Pyramide par genre: hommes à gauche, femmes à droite. Les personnes dont
   le genre est inconnu sont exclues du calcul. */
void age_print_gender_pyramid(const AgeNode *head, int type_filter, int gender_filter){
    unsigned long by_type[NB_USER_TYPES], by_gender[NB_GENDERS];
    unsigned long total = 0, max = 0, ignored = 0;

    for (const AgeNode *n = head; n != NULL; n = n->next) {
        age_count(n, type_filter, gender_filter, by_type, by_gender);
        total   += by_gender[0] + by_gender[1];
        ignored += by_gender[2];
        if (by_gender[0] > max) {
            max = by_gender[0];
        }
        if (by_gender[1] > max) {
            max = by_gender[1];
        }
    }

    printf(ANSI_BOLD ANSI_BRIGHT_CYAN "\n  --- Pyramide des ages par genre ---\n" ANSI_RESET);
    age_print_filters(type_filter, gender_filter);
    if (total == 0) {
        printf("  Aucune donnee pour cette selection.\n");
        return;
    }
    printf("  Legende : " ANSI_BLUE "# Hommes" ANSI_RESET " (gauche)  "
           ANSI_MAGENTA "# Femmes" ANSI_RESET " (droite)\n");
    printf("  Effectif total : %lu (genre non releve ignore : %lu)\n\n", total, ignored);

    for (const AgeNode *n = head; n != NULL; n = n->next) {
        age_count(n, type_filter, gender_filter, by_type, by_gender);
        if (by_gender[0] + by_gender[1] == 0) {
            continue;
        }
        int male_width   = age_bar_width(by_gender[0], max);
        int female_width = age_bar_width(by_gender[1], max);

        printf("  %3d ans %6.2f%% ", n->age, 100.0 * (double)by_gender[0] / (double)total);
        printf("%*s", BAR_WIDTH - male_width, "");
        age_print_bar(ANSI_BLUE, male_width);
        printf("|");
        age_print_bar(ANSI_MAGENTA, female_width);
        printf("%*s %6.2f%%\n", BAR_WIDTH - female_width, "", 100.0 * (double)by_gender[1] / (double)total);
    }
}

/* Lis une ligne depuis le clavier. Retourne 0 en cas de fin de saisie (EOF). */
int age_read_input(char *buffer, size_t size){
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    if (strchr(buffer, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            /* we discard the rest of the line */
        }
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    return 1;
}

/* Affiche le menu principal. */
void age_print_menu(int type_filter, int gender_filter){
    printf(ANSI_BOLD ANSI_BRIGHT_RED);
    printf("\n  +===========================================+\n");
    printf("  |          PYRAMIDE DES AGES                |\n");
    printf("  +===========================================+\n");
    printf("  |  1 : pyramide par type d'usager           |\n");
    printf("  |  2 : pyramide par genre                   |\n");
    printf("  |  3 : filtre type d'usager                 |\n");
    printf("  |  4 : filtre genre                         |\n");
    printf("  |  X : quitter                              |\n");
    printf("  +===========================================+\n");
    printf(ANSI_RESET);
    age_print_filters(type_filter, gender_filter);
    printf("  Choix : ");
    fflush(stdout);
}

/* Demande un filtre (0 = aucun, 1..nb). Retourne l'index, -1 pour "tous",
   ou le filtre actuel si l'entrée est invalide. */
int age_ask_filter(const char *title, const char *names[], int nb, int current){
    char input[INPUT_LEN];

    printf("\n  %s\n  0 : aucun filtre\n", title);
    for (int i = 0; i < nb; i++) {
        printf("  %d : %s\n", i + 1, names[i]);
    }
    printf("  Choix : ");
    fflush(stdout);

    if (!age_read_input(input, sizeof(input))) {
        return current;
    }
    char *endptr;
    long v = strtol(input, &endptr, 10);
    if (input[0] == '\0' || *endptr != '\0' || v < 0 || v > nb) {
        printf("  Choix invalide, filtre inchange.\n");
        return current;
    }
    return (int)v - 1;
}

/* Point d’entrée de type AGE_DATA. */
void age_data(char *file){
    AgeNode *head = age_load_data(file);
    int type_filter = -1, gender_filter = -1;
    char input[INPUT_LEN];

    for (;;) {
        age_print_menu(type_filter, gender_filter);
        if (!age_read_input(input, sizeof(input))) {
            break;
        }
        if (strcmp(input, "x") == 0 || strcmp(input, "X") == 0) {
            break;
        }
        if (strcmp(input, "1") == 0) {
            age_print_type_pyramid(head, type_filter, gender_filter);
        } else if (strcmp(input, "2") == 0) {
            age_print_gender_pyramid(head, type_filter, gender_filter);
        } else if (strcmp(input, "3") == 0) {
            type_filter = age_ask_filter("Filtre sur le type d'usager :",
                                         AGE_TYPE_NAMES, NB_USER_TYPES, type_filter);
        } else if (strcmp(input, "4") == 0) {
            /* Le 3d genre (inconnu) n'est pas proposé comme filtre */
            gender_filter = age_ask_filter("Filtre sur le genre :",
                                           AGE_GENDER_NAMES, NB_GENDERS - 1, gender_filter);
        } else {
            printf("  Choix invalide.\n");
        }
    }

    age_free_list(head);
}
