#ifndef age_data_h
#define age_data_h

/*indices des tableaux de comptage */
#define NB_USER_TYPES 3 /* 0=conducteur, 1=passager, 2=piéton */
#define NB_GENDERS 3 /* 0=homme, 1=femme, 2=non relevé */

void age_data(char *fichier);

//Pyramide des ages avec filtres par genre et types d'usager
//un maillon par age, la liste ets triée par age décroissant
//count[NB_USER_TYPES][NB_GENDERS] permet de croiser les filtre 

typedef struct AgeNode {
    unsigned char age;
    unsigned int count[NB_USER_TYPES][NB_GENDERS];

    struct AgeNode *next;
} AgeNode;

#endif