#ifndef age_data_h
#define age_data_h

void age_data(char *fichier);
    
#endif

//PYRAMIDE DES GENRES AVEC FILTRES PAR GENRE ET TYPES (CONDUCTEURS PIETONS..)
typedef struct AgeNode {
    int age;

    int driver_count;
    int passenger_count;
    int pedestrian_count;

    int male_count;
    int female_count;
    int unknown_gender_count;

    struct AgeNode *next;
} AgeNode;

