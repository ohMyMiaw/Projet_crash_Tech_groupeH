#ifndef crash_data_h
#define crash_data_h

void crash_data(char *fichier);

#endif

//STUCTURE CRASH DATA : rechercher rapidement un accident par année et ID,
//puis afficher les véhicules et leurs victimes

//Chaque véhicule possédera une liste chaînée de victimes
typedef struct VictimNode {
    int age;
    int gender;
    int user_type;
    int severity;

    struct VictimNode *next;
} VictimNode;


//Chaque accident possède une liste chaînée de véhicules
typedef struct VehicleNode {
    int vehicle_id;
    int purpose;

    VictimNode *victims;

    struct VehicleNode *next;
} VehicleNode;

//Chaque nœud de l'AVL représente un accident unique
typedef struct CrashNode {
    unsigned long long key;

    VehicleNode *vehicles;

    int height;

    struct CrashNode *left;
    struct CrashNode *right;
} CrashNode;

