#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "age_data.h"
#include "main.h"
#include "constante.h"

#define INPUT_LEN 16                  /* size of the menu input buffer */

/* Chart geometry */
#define MAX_BINS (AGE_MAX / 2 + 1)    /* the smallest bin size is 2 years */
#define CHART_HEIGHT 20               /* number of text rows of the plot area */
#define CHART_MAX_WIDTH 75            /* target width of the plot area (chars) */
#define LABEL_ROWS 5                  /* a y-axis label every 5 rows */
#define NB_NICE_STEPS 13

/* Age bins (in years) selectable from the menu */
#define NB_BIN_SIZES 3
#define DEFAULT_BIN_INDEX 1           /* 5 years */

/* Result of reading an integer field */
#define FIELD_OK      0
#define FIELD_EMPTY   1
#define FIELD_INVALID 2

char *AGE_TYPE_NAMES[NB_USER_TYPES]  = {"Conducteur", "Passager", "Pieton"};
char *AGE_GENDER_NAMES[NB_GENDERS]   = {"Homme", "Femme", "Non releve"};

/* Background colors: a bar is drawn with colored spaces (pure ASCII) */
char *AGE_TYPE_COLORS[NB_USER_TYPES]  = {ANSI_BG_GREEN, ANSI_BG_YELLOW, ANSI_BG_RED};
char *AGE_GENDER_COLORS[NB_GENDERS]   = {ANSI_BG_BLUE, ANSI_BG_MAGENTA, ANSI_BG_WHITE};

int   AGE_BIN_SIZES[NB_BIN_SIZES]    = {2, 5, 10};
char *AGE_BIN_NAMES[NB_BIN_SIZES]    = {"2 ans", "5 ans", "10 ans"};

/* Candidate values for a quarter of the y axis (in %) */
double AGE_NICE_STEPS[NB_NICE_STEPS] = {0.1, 0.2, 0.25, 0.5, 1, 1.5, 2, 2.5, 3, 4, 5, 10, 25};

/* ------------------------------------------------------------------ */
/*  Reading and storing the data                                      */
/* ------------------------------------------------------------------ */

/* Reads an integer from a text field. */
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

/* Looks for the node of an age in the list (sorted by decreasing age),
   creates it at the right place if it does not exist. */
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

/* Frees the whole list. */
void age_free_list(AgeNode *head){
    while (head != NULL) {
        AgeNode *next = head->next;
        free(head);
        head = next;
    }
}

/* Prints an error message (with the line number if known) and exits. */
void age_fatal_error(char *message, int line_number){
    if (line_number > 0) {
        fprintf(stderr, "Erreur (ligne %d) : %s\n", line_number, message);
    } else {
        fprintf(stderr, "Erreur : %s\n", message);
    }
    exit(1);
}

/* Splits a line in place on ';'. Returns the number of fields. */
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
    /* we leave the loop after the 9th field while a ';' followed it:
       there are too many columns */
    while (line[i] != '\0') {
        if (line[i] == ';') {
            return n + 1;
        }
        i++;
    }
    return n;
}

/* Processes one data line and updates the list. */
AgeNode *age_process_line(AgeNode *head, char *line, int line_number){
    char *fields[NB_CSV_FIELDS];
    int year = 0, birth_year = 0, user_type = 0, gender = 0;

    if (age_split_line(line, fields) != NB_CSV_FIELDS) {
        age_fatal_error("nombre de colonnes incorrect (9 attendues).", line_number);
    }

    /* Column 0: year, 4: user_type, 6: gender, 7: birth_year */
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

    /* Data unusable for the chart: the line is ignored */
    if (birth_result != FIELD_OK || user_type < 1 || user_type > NB_USER_TYPES) {
        return head;
    }

    /* Impossible age: ignored. This also protects the unsigned char
       of AgeNode (an age of 256 would silently become 0). */
    int age = year - birth_year;
    if (age < 0 || age > AGE_MAX) {
        return head;
    }

    /* gender: 1 = male, 2 = female, anything else = unknown */
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

/* Loads the whole CSV file into the linked list. */
AgeNode *age_load_data(char *file){
    FILE *f = fopen(file, "r");
    if (f == NULL) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier \"%s\".\n", file);
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    AgeNode *head = NULL;

    /* The header line is mandatory */
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
            /* empty line */
        } else {
            head = age_process_line(head, line, line_number);
        }
    }

    fclose(f);
    return head;
}

/* ------------------------------------------------------------------ */
/*  Computations                                                      */
/* ------------------------------------------------------------------ */

/* Adds up the counts of one age according to the filters
   (-1 = no filter, otherwise index of the type / of the gender). */
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

/* Groups the ages into bins of bin_size years (bin b = ages from
   b * bin_size to b * bin_size + bin_size - 1), applying the filters.
   Returns the number of bins to display (last non-empty bin + 1). */
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

/* Top of the y axis: a "nice" value (4 * a nice step) that is >= max_pct. */
double age_axis_top(double max_pct){
    for (int i = 0; i < NB_NICE_STEPS; i++) {
        if (4 * AGE_NICE_STEPS[i] >= max_pct) {
            return 4 * AGE_NICE_STEPS[i];
        }
    }
    return 100.0;
}

/* ------------------------------------------------------------------ */
/*  Display                                                           */
/* ------------------------------------------------------------------ */

/* Prints n spaces with the given background color. */
void age_print_fill(char *color, int n){
    printf("%s", color);
    for (int i = 0; i < n; i++) {
        printf(" ");
    }
    printf(ANSI_RESET);
}

/* Prints n empty characters ('.' in dim color on a grid row). */
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

/* Prints the active settings. */
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

/* Prints the legend: one colored square per series. */
void age_print_legend(char *colors[], char *names[], int series){
    printf("  Legende : ");
    for (int s = 0; s < series; s++) {
        age_print_fill(colors[s], 2);
        printf(" %s    ", names[s]);
    }
    printf("\n");
}

/* Draws a bar chart: ages on the x axis, percentage on the y axis.
   pct[b][s] = percentage of the selected population for bin b, series s.
   stacked = 1 : the series are stacked in one column (types of users)
   stacked = 0 : the series are side by side (genders). */
void age_draw_chart(double pct[][NB_USER_TYPES], int nb_bins, int bin_size,
                    int series, int stacked, char *colors[]){
    int cols = 1;   /* number of columns per bin */
    int height[MAX_BINS][NB_USER_TYPES];
    double max = 0;

    if (stacked == 0) {
        cols = series;
    }

    /* y scale */
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
    double step = top / CHART_HEIGHT;   /* percentage represented by one row */

    /* height (in rows) of every bar; stacked bars store cumulated heights */
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
        /* a non-empty bar is always at least one row high */
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

    /* width of a bin: bars thickness (th) and gap between bins */
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

    /* plot area, from the top row to the bottom row */
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

    /* x axis */
    printf("    0.00%% +");
    for (int i = 0; i < nb_bins * cell; i++) {
        printf("-");
    }
    printf("\n");

    /* x labels: start age of the bin, every k bins (multiple of 10 years,
       and wide enough to avoid overlapping) */
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

/* Prints "20-24 ans" for bin b. */
void age_print_bin_range(int b, int bin_size){
    printf("%d-%d ans", b * bin_size, b * bin_size + bin_size - 1);
}

/* Chart by type of user: stacked bars (driver / passenger / pedestrian).
   Percentage = share of the age bin in the selected population. */
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

/* Chart by gender: men and women side by side for each age bin.
   People whose gender is unknown are excluded from the computation. */
void age_print_gender_chart(AgeNode *head, int type_filter, int gender_filter, int bin_size){
    int by_type[MAX_BINS][NB_USER_TYPES], by_gender[MAX_BINS][NB_GENDERS];
    double pct[MAX_BINS][NB_USER_TYPES];
    int total = 0, ignored = 0;

    int nb_bins = age_aggregate(head, type_filter, gender_filter, bin_size, by_type, by_gender);
    while (nb_bins > 0 && by_gender[nb_bins - 1][0] + by_gender[nb_bins - 1][1] == 0) {
        nb_bins--;   /* last bins only contain unknown genders */
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

/* Reads a line from the keyboard. Returns 0 at end of input (EOF). */
int age_read_input(char *buffer){
    if (scanf("%15s", buffer) != 1) {
        return 0;
    }
    int c = getchar();
    while (c != '\n' && c != EOF) {
        /* we discard the rest of the line */
        c = getchar();
    }
    return 1;
}

/* Prints the main menu. */
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

/* Asks the user to choose among nb options.
   allow_none = 1 : option 0 means "no filter" and the function returns -1.
   Otherwise the options are 1..nb. Returns the index of the chosen option,
   or current if the input is invalid. */
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

/* AGE_DATA mode entry point. */
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
            /* the 3rd gender (unknown) is not offered as a filter */
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
