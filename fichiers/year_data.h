#ifndef year_data_h
#define year_data_h

void year_data(char *fichier);

#endif

//STATISTIQUE PAR ANNEE:
typedef struct YearStats {
    int year;
    int accident_count;

    int uninjured_count;
    int fatality_count;
    int serious_injury_count;
    int minor_injury_count;
} YearStats;
