#include <stdio.h>
#include <stdlib.h>
#include "year_data.h"
#include <string.h>
#include "main.h"
#include "constante.h"

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

void afficher_data_annee(int annee){

    return;
}