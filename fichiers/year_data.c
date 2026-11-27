#include <stdio.h>
#include <stdlib.h>
#include "year_data.h"
#include <string.h>
#include "main.h"
#include "constante.h"

YearStats years[NB_YEARS];


//Affiche les statistiques pour une annee donnee, ou un message (claude ma dit c'est bien pour le compilateur)


void display_year_stats(int year);
void load_year_data(const char *file_path);
void strip_line_ending(char *line);
int split_csv_fields(char *line, char *fields[]);
long parse_field(const char *field, int *ok);
void update_year_stats(int year, long collision_id, long severity);
void process_line(char *line);





// Point d'entree du mode YEAR_DATA : charge les donnees du fichier,
//puis affiche un menu qui boucle tant que l'utilisateur ne quitte
// pas (saisie de "x"/"X")


void year_data(char *file_path){
    load_year_data(file_path);

    printf(ANSI_BOLD ANSI_BRIGHT_RED);
    printf("  +===========================================+\n");
    printf("  |                                           |\n");
    printf("  |    Quelle annee voulez-vous regarder ?    |\n");
    printf("  |                                           |\n");
    printf("  |                                           |\n");
    printf("  |             X pour quitter                |\n");
    printf("  +===========================================+\n");
    printf(ANSI_RESET);

    for (;;) {
        char input[8];

        printf("\nAnnee (%d-%d) ou X pour quitter : ", YEAR_MIN, YEAR_MAX);

        if (scanf("%7s", input) != 1) {
            return; // fin de saisie (Ctrl+D) ou erreur de lecture
        }

        if (strcmp(input, "x") == 0 || strcmp(input, "X") == 0) {
            return;
        }

        char *endptr;
        long year_value = strtol(input, &endptr, 10);

        if (*endptr != '\0') {
            printf("Saisie invalide : entrez une annee ou X.\n");
            continue;
        }

        if (year_value < YEAR_MIN || year_value > YEAR_MAX) {
            printf("Annee hors bornes (%d-%d).\n", YEAR_MIN, YEAR_MAX);
            continue;
        }

        display_year_stats((int)year_value);
    }
}

//Retire le '\r' et/ou le '\n' laisse par fgets en fin de ligne,
// pour qu'un champ lu depuis un fichier au format Windows (\r\n)
// ou Unix (\n) donne exactement le meme resultat dans les deux cas.


void strip_line_ending(char *line){
    line[strcspn(line, "\r\n")] = '\0';
}

//Decoupe une ligne CSV en champs, en utilisant le point-virgule comme separateur.

int split_csv_fields(char *line, char *fields[]){
    int count = 0;
    char *token = strtok(line, ";");

    while (token != NULL && count < NB_CSV_FIELDS) {
        fields[count] = token;
        count++;
        token = strtok(NULL, ";");
    }

    if (count != NB_CSV_FIELDS) {
        return -1;
    }

    return count;
}

// COnvertit un champ de texte en entier long, et indique si la conversion a reussi

long parse_field(const char *field, int *ok){
    char *endptr;
    long value = strtol(field, &endptr, 10);

    if (endptr == field || *endptr != '\0') {
        *ok = 0;
        return 0;
    }

    *ok = 1;
    return value;
}

// Met a jour les statistiques pour une annee donnee, en incrementant
// le nombre d'accidents et le nombre d'usagers selon la gravite de
void update_year_stats(int year, long collision_id, long severity){
    int index = year - YEAR_MIN;

    years[index].year = year;

    if (collision_id > years[index].accident_count) {
        years[index].accident_count = (int)collision_id;
    }

    switch (severity) {
        case SEVERITY_UNINJURED:
            years[index].uninjured_count++;
            break;
        case SEVERITY_FATALITY:
            years[index].fatality_count++;
            break;
        case SEVERITY_SERIOUS_INJURY:
            years[index].serious_injury_count++;
            break;
        case SEVERITY_MINOR_INJURY:
            years[index].minor_injury_count++;
            break;
        default:
            break; /* valeur de gravite non reconnue, on l'ignore */
    }
}

// Traite une ligne du fichier CSV : on parse les champs, on verifie
// qu'ils sont valides, et on met a jour les statistiques de l'annee

void process_line(char *line){
    strip_line_ending(line);

    if (line[0] == '\0') {
        return; /* ligne vide */
    }

    char *fields[NB_CSV_FIELDS];

    if (split_csv_fields(line, fields) < 0) {
        return; // ligne malformee, on ignore la ligne
    }

    int ok;

    long year = parse_field(fields[0], &ok);
    if (!ok || year < YEAR_MIN || year > YEAR_MAX) {
        return; //ligne ne contient pas une annee valide, on ignore la ligne
    }

    long collision_id = parse_field(fields[1], &ok);
    if (!ok) {
        return;
    }

    long severity = parse_field(fields[5], &ok);
    if (!ok) {
        return;
    }

    update_year_stats((int)year, collision_id, severity);
}

// Lit le fichier CSV ligne par ligne, et appelle process_line() pour
// chaque ligneS. Les statistiques sont mises a jour dans le tableau
void load_year_data(const char *file_path){
    FILE *f = fopen(file_path, "r");

    if (f == NULL) {
        // Probleme bloquant : sans fichier, YEAR_DATA ne peut rien faire.
        // On arrete le programme plutot que de continuer avec un tableau
        fprintf(stderr, "FAUX !!!! : impossible d'ouvrir le fichier \"%s\".\n", file_path);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), f) != NULL) {
        process_line(line);
    }

    fclose(f);
}

// Affiche les statistiques pour une annee donnee, ou un message
// indiquant qu'il n'y a pas de donnees pour cette annee.

void display_year_stats(int year){
    int index = year - YEAR_MIN;
    YearStats *s = &years[index];

    printf(ANSI_BOLD ANSI_BRIGHT_CYAN);
    printf("\n  --- Annee %d ---\n", year);
    printf(ANSI_RESET);

    if (s->accident_count == 0) {
        printf("  Aucune donnee pour cette annee dans ce fichier.\n");
        return;
    }

    printf("  Nombre d'accidents       : %d\n", s->accident_count);
    printf("  Usagers indemnes         : %d\n", s->uninjured_count);
    printf(ANSI_BRIGHT_RED "  Usagers tues             : %d\n" ANSI_RESET, s->fatality_count);
    printf("  Blesses graves           : %d\n", s->serious_injury_count);
    printf("  Blesses legers           : %d\n", s->minor_injury_count);
}
