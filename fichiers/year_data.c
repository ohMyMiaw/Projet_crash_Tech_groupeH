#include <stdio.h>
#include <stdlib.h>
#include "year_data.h"
#include "main.h"
#include "constante.h"

void year_data(char *fichier){
    char annee[8];
    printf("quelle année voulez-vous regarder   /x pour quitter");
    scanf("%s",annee);
    if (annee=="x"){
        return;
    }
    int value = (int)strtol(annee, NULL, 10);
    if (annee>YEAR_MIN && annee<YEAR_MAX){
        afficher_data_annee(value);
    }
}

void afficher_data_annee(annee){


    return;
}