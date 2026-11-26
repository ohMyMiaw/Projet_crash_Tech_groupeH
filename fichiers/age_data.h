#ifndef age_data_h
#define age_data_h
#define AGE_INCONNU (-1)


void age_data(char *fichier);



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

static int lire_champ_entier(const char *champ){
    if (champ[0] == '\0') {
        return AGE_INCONNU;
    }

    char *endptr;
    long valeur = strtol(champ, &endptr, 10);

    if (*endptr != '\0') {
        return AGE_INCONNU;
    }

    return (int)valeur;
}

static AgeNode *trouver_ou_creer_noeud(AgeNode **tete, int age){
    AgeNode *precedent = NULL;
    AgeNode *courant = *tete;

    while (courant != NULL && courant->age > age) {
        precedent = courant;
        courant = courant->next;
    }

    if (courant != NULL && courant->age == age) {
        return courant;
    }

    AgeNode *nouveau = malloc(sizeof(AgeNode));

    if (nouveau == NULL) {
        return NULL;
    }

    nouveau->age                  = age;
    nouveau->driver_count         = 0;
    nouveau->passenger_count      = 0;
    nouveau->pedestrian_count     = 0;
    nouveau->male_count           = 0;
    nouveau->female_count         = 0;
    nouveau->unknown_gender_count = 0;
    nouveau->next                 = courant;

    if (precedent == NULL) {
        *tete = nouveau;
    } else {
        precedent->next = nouveau;
    }

    return nouveau;
}

#endif



