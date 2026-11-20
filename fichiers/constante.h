#ifndef CONSTANTS_H
#define CONSTANTS_H


#define YEAR_MIN 2005
#define YEAR_MAX 2024
#define NB_YEARS (YEAR_MAX - YEAR_MIN + 1) 


#define MAX_LINE_LENGTH 256 // pour la lecture des fichiers CSV

#define VEHICLE_ID_LEN 15

#define NB_CSV_FIELDS 9

// Définition des indices des champs dans le fichier CSV
#define NB_SEVERITY 4
#define SEVERITY_INDEMNE 1
#define SEVERITY_TUE 2
#define SEVERITY_BLESSE_GRAVE 3
#define SEVERITY_BLESSE_LEGER 4



#define MODE_YEAR_DATA_NAME "YEAR_DATA"
#define MODE_AGE_DATA_NAME "AGE_DATA"
#define MODE_CRASH_DATA_NAME "CRASH_DATA"

#endif 
