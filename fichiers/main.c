#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "year_data.h"
#include "age_data.h"
#include "crash_data.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        return 1;
    }

    char *mode    = argv[1];
    char fichier[100];
    strcpy(fichier,"dossier_csv/");
    strcat(fichier,argv[2]);

    if (strcmp(mode, "YEAR_DATA") == 0) {
        year_data(fichier);
    } else if (strcmp(mode, "AGE_DATA") == 0) {
        age_data(fichier);
    } else if (strcmp(mode, "CRASH_DATA") == 0) {
        crash_data(fichier);
    } else {
        return 1;
    }

    return 0;
}