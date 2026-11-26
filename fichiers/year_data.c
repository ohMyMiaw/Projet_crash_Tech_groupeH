#include <stdio.h>
#include <stdlib.h>
#include "year_data.h"
#include <string.h>
#include "main.h"
#include "constante.h"


static YearStats years[NB_YEARS];

void year_data(char *fichier){
    char annee[8];
    printf(ANSI_BOLD ANSI_BRIGHT_RED);
    printf("  +===========================================+\n");
    printf("  |                                           |\n");
    printf("  |    Quelle année voulez-vous regarder ?    |\n");
    printf("  |                                           |\n");
    printf("  |                                           |\n");
    printf("  |             X pour quitter                |\n");
    printf("  +===========================================+\n");
    printf(ANSI_RESET);
    scanf("%s",annee);
    if (strcmp(annee, "x") == 0){
        return;
    }
    int value = (int)strtol(annee, NULL, 10);
    if (value > YEAR_MIN && value < YEAR_MAX){
        afficher_data_annee(value);
    }
}

static void afficher_data_annee(int annee){
    int idx = annee - YEAR_MIN;
    YearStats *s = &years[idx];

    printf(ANSI_BOLD ANSI_BRIGHT_CYAN);
    printf("\n  --- Annee %d ---\n", annee);
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

static void charger_year_data(const char *fichier){
    FILE *f = fopen(fichier, "r");

    if (f == NULL) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier \"%s\".\n", fichier);
        return;
    }

    char ligne[MAX_LINE_LENGTH];

    while (fgets(ligne, sizeof(ligne), f) != NULL) {
        traiter_ligne(ligne);
    }

    fclose(f);
}