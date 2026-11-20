#include <stdio.h>
#include <stdlib.h> 
#include "year_data.h"
#include "age_data.h"
#include "crash_data.h"



int main(){
    int choix;
    printf("1 : year_data \n ");
    printf("2 : age_data \n ");
    printf("3 : crash_data \n ");
    printf("5 : exit");
    scanf("%d",&choix);
    if (choix == 1){
        year_data();
    }
    if (choix == 2){
        age_data();
    }
    if (choix == 3){
        crash_data();
    }
    if (choix == (5)){
        return 0;
    }
}