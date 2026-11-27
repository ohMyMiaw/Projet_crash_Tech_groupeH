#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "age_data.h"
#include "main.h"
#include "constante.h"

#define INPUT_LEN 16                  /* taille du tampon de saisie du menu */

/* Dimensions du graphique */
#define MAX_BINS (AGE_MAX / 2 + 1)    /* la plus petite tranche couvre 2 ans */
#define CHART_HEIGHT 20               /* nombre de lignes de texte de la zone du graphique */
#define CHART_MAX_WIDTH 75            /* largeur cible de la zone du graphique (caractères) */
#define LABEL_ROWS 5                  /* une graduation de l’axe Y toutes les 5 lignes */
#define NB_NICE_STEPS 13

/* Tranches d’âge (en années) sélectionnables dans le menu */
#define NB_BIN_SIZES 3
#define DEFAULT_BIN_INDEX 1           /* 5 ans */

/* Résultat de la lecture d’un champ entier */
#define FIELD_OK      0
#define FIELD_EMPTY   1
#define FIELD_INVALID 2

char *AGE_TYPE_NAMES[NB_USER_TYPES]  = {"Conducteur", "Passager", "Pieton"};
char *AGE_GENDER_NAMES[NB_GENDERS]   = {"Homme", "Femme", "Non releve"};

/* Couleurs de fond : une barre est dessinée avec des espaces colorés (ASCII uniquement) */
char *AGE_TYPE_COLORS[NB_USER_TYPES]  = {ANSI_BG_GREEN, ANSI_BG_YELLOW, ANSI_BG_RED};
char *AGE_GENDER_COLORS[NB_GENDERS]   = {ANSI_BG_BLUE, ANSI_BG_MAGENTA, ANSI_BG_WHITE};

int   AGE_BIN_SIZES[NB_BIN_SIZES]    = {2, 5, 10};
char *AGE_BIN_NAMES[NB_BIN_SIZES]    = {"2 ans", "5 ans", "10 ans"};

/* Valeurs possibles pour un quart de l’axe Y (en %) */
double AGE_NICE_STEPS[NB_NICE_STEPS] = {0.1, 0.2, 0.25, 0.5, 1, 1.5, 2, 2.5, 3, 4, 5, 10, 25};

/* ------------------------------------------------------------------ */
/*  Lecture et stockage des données                                      */
/* ------------------------------------------------------------------ */

/* Lit un entier depuis un champ de texte. */
int age_read_int_field(char *field, int *value){
    int i = 0;
    int sign = 1;
    int result = 0;

    if (field[0] == '\0') {
        return FIELD_EMPTY;
    }

    if (field[0] == '-') {
        sign = -1;
        i = 1;
        if (field[1] == '\0') {
            return FIELD_INVALID;
        }
    }

    while (field[i] != '\0') {
        if (field[i] < '0' || field[i] > '9') {
            return FIELD_INVALID;
        }
        if (result > 100000000) {
            return FIELD_INVALID;
        }
        result = result * 10 + (field[i] - '0');
        i++;
    }

    *value = sign * result;
    return FIELD_OK;
}

/* Recherche le nœud correspondant à un âge dans la liste (triée par âge décroissant),
   et le crée à la bonne place s’il n’existe pas. */
AgeNode *age_find_or_create_node(AgeNode *head, int age){
    AgeNode *previous = NULL;
    AgeNode *current = head;

    while (current != NULL && current->age > age) {
        previous = current;
        current = current->next;
    }

    if (current != NULL && current->age == age) {
        return head;
    }

    AgeNode *new_node = malloc(sizeof(AgeNode));

    if (new_node == NULL) {
        fprintf(stderr, "Erreur : memoire insuffisante.\n");
        exit(1);
    }

    new_node->age = age;
    for (int t = 0; t < NB_USER_TYPES; t++) {
        for (int g = 0; g < NB_GENDERS; g++) {
            new_node->count[t][g] = 0;
        }
    }
    new_node->next = current;

    if (previous == NULL) {
        head = new_node;
    } else {
        previous->next = new_node;
    }

    return head;
}

/* Libère toute la liste. */
void age_free_list(AgeNode *head){
    while (head != NULL) {
        AgeNode *next = head->next;
        free(head);
        head = next;
    }
}

/* Affiche un message d’erreur (avec le numéro de ligne si connu), puis quitte. */
void age_fatal_error(char *message, int line_number){
    if (line_number > 0) {
        fprintf(stderr, "Erreur (ligne %d) : %s\n", line_number, message);
    } else {
        fprintf(stderr, "Erreur : %s\n", message);
    }
    exit(1);
}

/* Découpe une ligne sur « ; » directement dans celle-ci. Renvoie le nombre de champs. */
int age_split_line(char *line, char *fields[NB_CSV_FIELDS]){
    int n = 1;
    int i = 0;

    fields[0] = line;
    while (line[i] != '\0' && n < NB_CSV_FIELDS) {
        if (line[i] == ';') {
            line[i] = '\0';
            fields[n] = &line[i + 1];
            n++;
        }
        i++;
    }
    /* on sort de la boucle après le 9e champ lorsqu’un « ; » le suit :
       il y a trop de colonnes */
    while (line[i] != '\0') {
        if (line[i] == ';') {
            return n + 1;
        }
        i++;
    }
    return n;
}

/* Traite une ligne de données et met à jour la liste. */
AgeNode *age_process_line(AgeNode *head, char *line, int line_number){
    char *fields[NB_CSV_FIELDS];
    int year = 0, birth_year = 0, user_type = 0, gender = 0;

    if (age_split_line(line, fields) != NB_CSV_FIELDS) {
        age_fatal_error("nombre de colonnes incorrect (9 attendues).", line_number);
    }

    /* Colonne 0 : année, 4 : type d’usager, 6 : genre, 7 : année de naissance */
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

    /* Données inutilisables pour le graphique : la ligne est ignorée */
    if (birth_result != FIELD_OK || user_type < 1 || user_type > NB_USER_TYPES) {
<<<<<<< HEAD
        return head;
=======
        return;
    }
    int age = year - birth_year;
    if (age < 0 || age > AGE_MAX) { // commentaire d’origine : demande de clarification sur cette condition
        return;
>>>>>>> e55cb33dac2e2ff3ce9c4f28bda94040b1e0ff76
    }

    /* Âge impossible : ignoré. Cela protège également le type unsigned char
       de AgeNode (un âge de 256 deviendrait silencieusement égal à 0). */
    int age = year - birth_year;
    if (age < 0 || age > AGE_MAX) {
        return head;
    }

    /* genre : 1 = homme, 2 = femme, toute autre valeur = inconnu */
    int gender_index = NB_GENDERS - 1;
    if (gender_result == FIELD_OK && (gender == 1 || gender == 2)) {
        gender_index = gender - 1;
    }

    head = age_find_or_create_node(head, age);

    AgeNode *node = head;
    while (node->age != age) {
        node = node->next;
    }
    node->count[user_type - 1][gender_index]++;

    return head;
}

/* Charge l’intégralité du fichier CSV dans la liste chaînée. */
AgeNode *age_load_data(char *file){
    FILE *f = fopen(file, "r");
    if (f == NULL) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier \"%s\".\n", file);
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    AgeNode *head = NULL;

    /* La ligne d’en-tête est obligatoire */
    if (fgets(line, MAX_LINE_LENGTH, f) == NULL) {
        age_fatal_error("fichier vide.", 0);
    }
    line_number++;
    if (strncmp(line, "year;", 5) != 0) {
        age_fatal_error("en-tete CSV invalide (\"year;collision_id;...\" attendu).", line_number);
    }

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        line_number++;

        int length = 0;
        while (line[length] != '\0' && line[length] != '\n' && line[length] != '\r') {
            length++;
        }
        if (line[length] == '\0' && !feof(f)) {
            age_fatal_error("ligne trop longue.", line_number);
        }
        line[length] = '\0';

        if (length == 0) {
            /* ligne vide */
        } else {
            head = age_process_line(head, line, line_number);
        }
    }

    fclose(f);
    return head;
}

/* ------------------------------------------------------------------ */
/*  Calculs                                                      */
/* ------------------------------------------------------------------ */

/* Additionne les effectifs d’un âge en fonction des filtres
   (-1 = aucun filtre, sinon indice du type / du genre). */
void age_count(AgeNode *node, int type_filter, int gender_filter,
               int by_type[NB_USER_TYPES], int by_gender[NB_GENDERS]){
    for (int t = 0; t < NB_USER_TYPES; t++) {
        by_type[t] = 0;
    }
    for (int g = 0; g < NB_GENDERS; g++) {
        by_gender[g] = 0;
    }
    for (int t = 0; t < NB_USER_TYPES; t++) {
        if (type_filter < 0 || type_filter == t) {
            for (int g = 0; g < NB_GENDERS; g++) {
                if (gender_filter < 0 || gender_filter == g) {
                    by_type[t]   += node->count[t][g];
                    by_gender[g] += node->count[t][g];
                }
            }
        }
    }
}

/* Regroupe les âges dans des tranches de bin_size années (la tranche b correspond aux âges de
   b * bin_size à b * bin_size + bin_size - 1), en appliquant les filtres.
   Renvoie le nombre de tranches à afficher (dernière tranche non vide + 1). */
int age_aggregate(AgeNode *head, int type_filter, int gender_filter, int bin_size,
                  int by_type[][NB_USER_TYPES], int by_gender[][NB_GENDERS]){
    int node_type[NB_USER_TYPES], node_gender[NB_GENDERS];
    int nb_bins = 0;

    for (int b = 0; b < MAX_BINS; b++) {
        for (int t = 0; t < NB_USER_TYPES; t++) {
            by_type[b][t] = 0;
        }
        for (int g = 0; g < NB_GENDERS; g++) {
            by_gender[b][g] = 0;
        }
    }

    AgeNode *n = head;
    while (n != NULL) {
        int b = n->age / bin_size;
        if (b < MAX_BINS) {
            age_count(n, type_filter, gender_filter, node_type, node_gender);

            int sum = 0;
            for (int t = 0; t < NB_USER_TYPES; t++) {
                by_type[b][t] += node_type[t];
                sum += node_type[t];
            }
            for (int g = 0; g < NB_GENDERS; g++) {
                by_gender[b][g] += node_gender[g];
            }
            if (sum > 0 && b + 1 > nb_bins) {
                nb_bins = b + 1;
            }
        }
        n = n->next;
    }
    return nb_bins;
}

/* Valeur maximale de l’axe Y : une valeur arrondie (4 * un pas arrondi) supérieure ou égale à max_pct. */
double age_axis_top(double max_pct){
    for (int i = 0; i < NB_NICE_STEPS; i++) {
        if (4 * AGE_NICE_STEPS[i] >= max_pct) {
            return 4 * AGE_NICE_STEPS[i];
        }
    }
    return 100.0;
}

/* ------------------------------------------------------------------ */
/*  Affichage                                                           */
/* ------------------------------------------------------------------ */

/* Affiche n espaces avec la couleur de fond indiquée. */
void age_print_fill(char *color, int n){
    printf("%s", color);
    for (int i = 0; i < n; i++) {
        printf(" ");
    }
    printf(ANSI_RESET);
}

/* Affiche n caractères vides ( « . » en couleur atténuée sur une ligne de grille). */
void age_print_empty(int n, int grid){
    if (grid == 1) {
        printf(ANSI_DIM);
        for (int i = 0; i < n; i++) {
            printf(".");
        }
        printf(ANSI_RESET);
    } else {
        for (int i = 0; i < n; i++) {
            printf(" ");
        }
    }
}

/* Affiche les paramètres actifs. */
void age_print_settings(int type_filter, int gender_filter, int bin_size){
    char *type_name = "tous";
    char *gender_name = "tous";

    if (type_filter >= 0) {
        type_name = AGE_TYPE_NAMES[type_filter];
    }
    if (gender_filter >= 0) {
        gender_name = AGE_GENDER_NAMES[gender_filter];
    }
    printf("  Filtres : usager = %s, genre = %s | tranches de %d ans\n",
           type_name, gender_name, bin_size);
}

/* Affiche la légende : un carré coloré par série. */
void age_print_legend(char *colors[], char *names[], int series){
    printf("  Legende : ");
    for (int s = 0; s < series; s++) {
        age_print_fill(colors[s], 2);
        printf(" %s    ", names[s]);
    }
    printf("\n");
}

/* Dessine un diagramme en barres : les âges sur l’axe X et les pourcentages sur l’axe Y.
   pct[b][s] = pourcentage de la population sélectionnée pour la tranche b et la série s.
   stacked = 1 : les séries sont empilées dans une seule colonne (types d’usagers)
   stacked = 0 : les séries sont côte à côte (genres). */
void age_draw_chart(double pct[][NB_USER_TYPES], int nb_bins, int bin_size,
                    int series, int stacked, char *colors[]){
    int cols = 1;   /* nombre de colonnes par tranche */
    int height[MAX_BINS][NB_USER_TYPES];
    double max = 0;

    if (stacked == 0) {
        cols = series;
    }

    /* Échelle de l’axe Y */
    for (int b = 0; b < nb_bins; b++) {
        double value = 0;
        for (int s = 0; s < series; s++) {
            if (stacked == 1) {
                value = value + pct[b][s];
            } else if (pct[b][s] > value) {
                value = pct[b][s];
            }
        }
        if (value > max) {
            max = value;
        }
    }
    double top  = age_axis_top(max);
    double step = top / CHART_HEIGHT;   /* pourcentage représenté par une ligne */

    /* hauteur (en lignes) de chaque barre ; les barres empilées stockent les hauteurs cumulées */
    for (int b = 0; b < nb_bins; b++) {
        double cumulated = 0;
        int first = -1;
        for (int s = 0; s < series; s++) {
            if (stacked == 1) {
                cumulated = cumulated + pct[b][s];
                height[b][s] = (int)(cumulated / step + 0.5);
            } else {
                height[b][s] = (int)(pct[b][s] / step + 0.5);
            }
            if (pct[b][s] > 0 && first < 0) {
                first = s;
            }
        }
        /* une barre non vide mesure toujours au moins une ligne de haut */
        if (stacked == 1) {
            if (first >= 0 && height[b][series - 1] == 0) {
                for (int s = first; s < series; s++) {
                    height[b][s] = 1;
                }
            }
        } else {
            for (int s = 0; s < series; s++) {
                if (pct[b][s] > 0 && height[b][s] == 0) {
                    height[b][s] = 1;
                }
            }
        }
    }

    /* largeur d’une tranche : épaisseur des barres (th) et espace entre les tranches */
    int room = CHART_MAX_WIDTH / nb_bins;
    int gap  = 0;
    if (room >= cols + 1) {
        gap = 1;
    }
    int th = (room - gap) / cols;
    if (th < 1) {
        th = 1;
    }
    if (th > 4) {
        th = 4;
    }
    int cell = cols * th + gap;

    /* zone du graphique, de la ligne du haut à celle du bas */
    for (int r = CHART_HEIGHT; r >= 1; r--) {
        int grid = 0;
        if (r % LABEL_ROWS == 0) {
            grid = 1;
            printf("  %6.2f%% |", top * r / CHART_HEIGHT);
        } else {
            printf("          |");
        }
        for (int b = 0; b < nb_bins; b++) {
            for (int c = 0; c < cols; c++) {
                char *color = NULL;
                if (stacked == 1) {
                    for (int s = 0; s < series && color == NULL; s++) {
                        if (r <= height[b][s]) {
                            color = colors[s];
                        }
                    }
                } else if (r <= height[b][c]) {
                    color = colors[c];
                }
                if (color != NULL) {
                    age_print_fill(color, th);
                } else {
                    age_print_empty(th, grid);
                }
            }
            age_print_empty(gap, grid);
        }
        printf("\n");
    }

    /* axe X */
    printf("    0.00%% +");
    for (int i = 0; i < nb_bins * cell; i++) {
        printf("-");
    }
    printf("\n");

    /* étiquettes de l’axe X : âge de début de tranche, toutes les k tranches (multiples de 10 ans,
       et espacées suffisamment pour éviter les chevauchements) */
    int k = (4 + cell - 1) / cell;
    while ((k * bin_size) % 10 != 0) {
        k++;
    }
    age_print_empty(11, 0);
    for (int b = 0; b < nb_bins; b = b + k) {
        int label = b * bin_size;
        int digits = 1;
        if (label >= 10) {
            digits = 2;
        }
        if (label >= 100) {
            digits = 3;
        }
        printf("%d", label);
        age_print_empty(k * cell - digits, 0);
    }
    printf("\n");

    int width = nb_bins * cell;
    int margin = 11;
    if (width > 9) {
        margin = 11 + (width - 9) / 2;
    }
    age_print_empty(margin, 0);
    printf(ANSI_BOLD "Age (ans)" ANSI_RESET "\n");
}

/* Affiche « 20-24 ans » pour la tranche b. */
void age_print_bin_range(int b, int bin_size){
    printf("%d-%d ans", b * bin_size, b * bin_size + bin_size - 1);
}

/* Graphique par type d’usager : barres empilées (conducteur / passager / piéton).
   Pourcentage = part de la tranche d’âge dans la population sélectionnée. */
void age_print_type_chart(AgeNode *head, int type_filter, int gender_filter, int bin_size){
    int by_type[MAX_BINS][NB_USER_TYPES], by_gender[MAX_BINS][NB_GENDERS];
    double pct[MAX_BINS][NB_USER_TYPES];
    int total = 0;

    int nb_bins = age_aggregate(head, type_filter, gender_filter, bin_size, by_type, by_gender);
    for (int b = 0; b < nb_bins; b++) {
        for (int t = 0; t < NB_USER_TYPES; t++) {
            total = total + by_type[b][t];
        }
    }

    printf(ANSI_BOLD ANSI_BRIGHT_CYAN "\n  --- Repartition des ages par type d'usager ---\n" ANSI_RESET);
    age_print_settings(type_filter, gender_filter, bin_size);
    if (total == 0) {
        printf("  Aucune donnee pour cette selection.\n");
        return;
    }
    age_print_legend(AGE_TYPE_COLORS, AGE_TYPE_NAMES, NB_USER_TYPES);
    printf("  Effectif total : %d\n", total);
    printf("  Chaque colonne regroupe %d ans (graduation = age de debut).\n\n", bin_size);

    int peak = 0;
    double peak_pct = 0;
    for (int b = 0; b < nb_bins; b++) {
        double sum = 0;
        for (int t = 0; t < NB_USER_TYPES; t++) {
            pct[b][t] = 100.0 * by_type[b][t] / total;
            sum = sum + pct[b][t];
        }
        if (sum > peak_pct) {
            peak_pct = sum;
            peak = b;
        }
    }

    age_draw_chart(pct, nb_bins, bin_size, NB_USER_TYPES, 1, AGE_TYPE_COLORS);

    printf("\n  Tranche la plus representee : ");
    age_print_bin_range(peak, bin_size);
    printf(" (%.2f%%)\n", peak_pct);
}

/* Graphique par genre : hommes et femmes côte à côte pour chaque tranche d’âge.
   Les personnes dont le genre est inconnu sont exclues du calcul. */
void age_print_gender_chart(AgeNode *head, int type_filter, int gender_filter, int bin_size){
    int by_type[MAX_BINS][NB_USER_TYPES], by_gender[MAX_BINS][NB_GENDERS];
    double pct[MAX_BINS][NB_USER_TYPES];
    int total = 0, ignored = 0;

    int nb_bins = age_aggregate(head, type_filter, gender_filter, bin_size, by_type, by_gender);
    while (nb_bins > 0 && by_gender[nb_bins - 1][0] + by_gender[nb_bins - 1][1] == 0) {
        nb_bins--;   /* les dernières tranches ne contiennent que des genres inconnus */
    }
    for (int b = 0; b < nb_bins; b++) {
        total   = total + by_gender[b][0] + by_gender[b][1];
        ignored = ignored + by_gender[b][2];
    }

    printf(ANSI_BOLD ANSI_BRIGHT_CYAN "\n  --- Repartition des ages par genre ---\n" ANSI_RESET);
    age_print_settings(type_filter, gender_filter, bin_size);
    if (total == 0) {
        printf("  Aucune donnee pour cette selection.\n");
        return;
    }
    age_print_legend(AGE_GENDER_COLORS, AGE_GENDER_NAMES, 2);
    printf("  Effectif total : %d (genre non releve ignore : %d)\n", total, ignored);
    printf("  Chaque colonne regroupe %d ans (graduation = age de debut).\n\n", bin_size);

    int peak[2] = {0, 0};
    double peak_pct[2] = {0, 0};
    for (int b = 0; b < nb_bins; b++) {
        for (int g = 0; g < 2; g++) {
            pct[b][g] = 100.0 * by_gender[b][g] / total;
            if (pct[b][g] > peak_pct[g]) {
                peak_pct[g] = pct[b][g];
                peak[g] = b;
            }
        }
    }

    age_draw_chart(pct, nb_bins, bin_size, 2, 0, AGE_GENDER_COLORS);

    for (int g = 0; g < 2; g++) {
        if (g == 0) {
            printf("\n  Pic des hommes : ");
        } else {
            printf("  Pic des femmes : ");
        }
        age_print_bin_range(peak[g], bin_size);
        printf(" (%.2f%%)\n", peak_pct[g]);
    }
}

/* ------------------------------------------------------------------ */
/*  Menu                                                              */
/* ------------------------------------------------------------------ */

/* Lit une ligne au clavier. Renvoie 0 lorsque l’entrée est terminée (EOF). */
int age_read_input(char *buffer){
    if (scanf("%15s", buffer) != 1) {
        return 0;
    }
    int c = getchar();
    while (c != '\n' && c != EOF) {
        /* on ignore le reste de la ligne */
        c = getchar();
    }
    return 1;
}

/* Affiche le menu principal. */
void age_print_menu(int type_filter, int gender_filter, int bin_size){
    printf(ANSI_BOLD ANSI_BRIGHT_RED);
    printf("\n  +===========================================+\n");
    printf("  |          GRAPHIQUE DES AGES               |\n");
    printf("  +===========================================+\n");
    printf("  |  1 : graphique par type d'usager          |\n");
    printf("  |  2 : graphique par genre                  |\n");
    printf("  |  3 : filtre type d'usager                 |\n");
    printf("  |  4 : filtre genre                         |\n");
    printf("  |  5 : taille des tranches d'age            |\n");
    printf("  |  X : quitter                              |\n");
    printf("  +===========================================+\n");
    printf(ANSI_RESET);
    age_print_settings(type_filter, gender_filter, bin_size);
    printf("  Choix : ");
    fflush(stdout);
}

/* Demande à l’utilisateur de choisir parmi nb options.
   allow_none = 1 : l’option 0 signifie « aucun filtre » et la fonction renvoie -1.
   Sinon, les options vont de 1 à nb. Renvoie l’indice de l’option choisie,
   ou current si la saisie est invalide. */
int age_ask_choice(char *title, char *names[], int nb, int current, int allow_none){
    char input[INPUT_LEN];
    int choice = 0;
    int min = 1;

    printf("\n  %s\n", title);
    if (allow_none == 1) {
        min = 0;
        printf("  0 : aucun filtre\n");
    }
    for (int i = 0; i < nb; i++) {
        printf("  %d : %s\n", i + 1, names[i]);
    }
    printf("  Choix : ");
    fflush(stdout);

    if (age_read_input(input) == 0) {
        return current;
    }
    if (age_read_int_field(input, &choice) != FIELD_OK || choice < min || choice > nb) {
        printf("  Choix invalide, rien n'a change.\n");
        return current;
    }
    return choice - 1;
}

/* Point d’entrée du mode AGE_DATA. */
void age_data(char *file){
    AgeNode *head = age_load_data(file);
    int type_filter = -1, gender_filter = -1;
    int bin_index = DEFAULT_BIN_INDEX;
    char input[INPUT_LEN];
    int running = 1;

    while (running == 1) {
        age_print_menu(type_filter, gender_filter, AGE_BIN_SIZES[bin_index]);
        if (age_read_input(input) == 0) {
            running = 0;
        } else if (strcmp(input, "x") == 0 || strcmp(input, "X") == 0) {
            running = 0;
        } else if (strcmp(input, "1") == 0) {
            age_print_type_chart(head, type_filter, gender_filter, AGE_BIN_SIZES[bin_index]);
        } else if (strcmp(input, "2") == 0) {
            age_print_gender_chart(head, type_filter, gender_filter, AGE_BIN_SIZES[bin_index]);
        } else if (strcmp(input, "3") == 0) {
            type_filter = age_ask_choice("Filtre sur le type d'usager :",
                                         AGE_TYPE_NAMES, NB_USER_TYPES, type_filter, 1);
        } else if (strcmp(input, "4") == 0) {
            /* le 3e genre (inconnu) n’est pas proposé comme filtre */
            gender_filter = age_ask_choice("Filtre sur le genre :",
                                           AGE_GENDER_NAMES, NB_GENDERS - 1, gender_filter, 1);
        } else if (strcmp(input, "5") == 0) {
            bin_index = age_ask_choice("Taille des tranches d'age :",
                                       AGE_BIN_NAMES, NB_BIN_SIZES, bin_index, 0);
        } else {
            printf("  Choix invalide.\n");
        }
    }

    age_free_list(head);
}
