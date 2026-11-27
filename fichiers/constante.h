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
#define SEVERITY_UNINJURED 1
#define SEVERITY_FATALITY 2
#define SEVERITY_SERIOUS_INJURY 3
#define SEVERITY_MINOR_INJURY 4

#define ANSI_RESET        "\033[0m"
#define ANSI_BOLD         "\033[1m"
#define ANSI_DIM          "\033[2m"

#define ANSI_BLACK        "\033[30m"
#define ANSI_RED          "\033[31m"
#define ANSI_GREEN        "\033[32m"
#define ANSI_YELLOW       "\033[33m"
#define ANSI_BLUE         "\033[34m"
#define ANSI_MAGENTA      "\033[35m"
#define ANSI_CYAN         "\033[36m"
#define ANSI_WHITE        "\033[37m"

#define ANSI_BRIGHT_RED     "\033[91m"
#define ANSI_BRIGHT_GREEN   "\033[92m"
#define ANSI_BRIGHT_YELLOW  "\033[93m"
#define ANSI_BRIGHT_BLUE    "\033[94m"
#define ANSI_BRIGHT_MAGENTA "\033[95m"
#define ANSI_BRIGHT_CYAN    "\033[96m"
#define ANSI_BRIGHT_WHITE   "\033[97m"

#define ANSI_BG_BLACK    "\033[40m"
#define ANSI_BG_RED      "\033[41m"
#define ANSI_BG_GREEN    "\033[42m"
#define ANSI_BG_YELLOW   "\033[43m"
#define ANSI_BG_BLUE     "\033[44m"
#define ANSI_BG_MAGENTA  "\033[45m"
#define ANSI_BG_CYAN     "\033[46m"
#define ANSI_BG_WHITE    "\033[47m"


#define MODE_YEAR_DATA_NAME "YEAR_DATA"
#define MODE_AGE_DATA_NAME "AGE_DATA"
#define MODE_CRASH_DATA_NAME "CRASH_DATA"

#endif 
