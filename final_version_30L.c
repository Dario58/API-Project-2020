/* Versione Finale 8/9/2020 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 1026
char BUFFER[MAX];
int BUFFER_FULL = 0; // Serve per l'input.
int COMANDI = 0; // Conta i comandi svolti.
int UNDO_FATTI = 0; // Conta gli Undo fatti.
int WRITEONLY = 1;

//DATI: ARRAY//
char * * STATO_WO = NULL;
int TOP_FRASI = 0;
char * * STATO_CURR = NULL;

//DATI : STACK//
typedef struct comando_in_pila {
    char comando;
    int indice1;
    int indice2;
    int use;
    int frasi_stato;
    char * * FRASI;
} Record;
Record * STACK;
Record * CURR_CMD;
Record * WO_lo=NULL;
Record * WO_hi=NULL;
int ToS = 1;

//PUSH_STACK//
void push_stack(int ind1, int ind2, char cmd){
    STACK = realloc(STACK, (sizeof(Record) * (ToS+1)));
    STACK[ToS].comando=cmd;
    STACK[ToS].FRASI=NULL;
    STACK[ToS].indice1=ind1;
    STACK[ToS].indice2=ind2;
    STACK[ToS].use=1;
    STACK[ToS].frasi_stato=0;
    CURR_CMD=&(STACK[ToS]);
}

//CHANGE//
void change(int ind1, int ind2){
    int i, k=0;
    if(UNDO_FATTI>0){
        ToS=ToS-UNDO_FATTI;
        UNDO_FATTI=0;
    }
    push_stack(ind1, ind2, 'c');
    if(ind1 <= TOP_FRASI) WRITEONLY = 0;

    if(WRITEONLY){
        if(TOP_FRASI == 0) WO_lo = &(STACK[ToS]);

        STATO_WO = realloc(STATO_WO, sizeof(char *) * (ind2+1));
        for(i=ind1; i<ind2+1; i++){
            fgets(BUFFER, MAX, stdin);
            STATO_WO[i] = malloc(sizeof(char) * (strlen(BUFFER)+1));
            strcpy(STATO_WO[i], BUFFER);
        }
        TOP_FRASI = ind2;
        STACK[ToS].frasi_stato = TOP_FRASI;
        STACK[ToS].FRASI = NULL;
        WO_hi = &(STACK[ToS]);
        STATO_CURR = STATO_WO;
    }

    if(!WRITEONLY){
        if(ind2 <= TOP_FRASI){
            STACK[ToS].FRASI = malloc(sizeof(char *) * (TOP_FRASI+1));
            for(i=1; i<TOP_FRASI+1; i++){
                if(i>=ind1 && i<=ind2) {
                    fgets(BUFFER, MAX, stdin);
                    STACK[ToS].FRASI[i] = malloc(sizeof(char) * (strlen(BUFFER)+1));
                    strcpy(STACK[ToS].FRASI[i], BUFFER);
                }
                else{
                    STACK[ToS].FRASI[i] = STATO_CURR[i];
                }
            }
            STATO_CURR = STACK[ToS].FRASI;
        }
        else{
            STACK[ToS].FRASI = malloc(sizeof(char *) * (ind2+1));
            for(i=1; i<ind2+1; i++){
                if(i>=ind1 && i<=ind2) {
                    fgets(BUFFER, MAX, stdin);
                    STACK[ToS].FRASI[i] = malloc(sizeof(char) * (strlen(BUFFER)+1));
                    strcpy(STACK[ToS].FRASI[i], BUFFER);
                    k++;
                }
                else{
                    STACK[ToS].FRASI[i] = STATO_CURR[i+k];
                }
            }
            STATO_CURR = STACK[ToS].FRASI;
            TOP_FRASI = ind2;
        }
        STACK[ToS].frasi_stato = TOP_FRASI;
    }
    fgets(BUFFER, MAX, stdin); // Catturo il punto finale.
}

//DELETE//
void delete(int ind1, int ind2){
    int i;
    int limite;
    if(UNDO_FATTI > 0){
        ToS=ToS-UNDO_FATTI;
        UNDO_FATTI=0;
    }
    // Aggiustamento indice:
    if (ind1 == 0) ind1++; // Se ind1 della Delete iniziale è 0, aggiornalo semplicemente a 1.

    if(ind2 <= TOP_FRASI) limite=ind2; // Fin dove deve arrivare il ciclo?
    else limite=TOP_FRASI;

    push_stack(ind1, limite, 'd');

    if(ind2 == 0 || TOP_FRASI == 0){
        STACK[ToS].use = 0; // Diventa un comando "inutile" in Stack ed esco.
        STACK[ToS].FRASI = STATO_CURR;
        STACK[ToS].frasi_stato = TOP_FRASI;
        return;
    }

    STACK[ToS].FRASI = malloc(sizeof(char *) * (TOP_FRASI+1 - (limite-ind1+1))); // Aggiungo un char * perché voglio partire da 1

    for (i=1; i<(TOP_FRASI+1 - (limite-ind1+1)); i++) {
        if(i<ind1){
            STACK[ToS].FRASI[i] = STATO_CURR[i];
        }
        else{
            STACK[ToS].FRASI[i] = STATO_CURR[i + limite-ind1+1];
        }
    }
    TOP_FRASI = TOP_FRASI - (limite-ind1+1); // Ci saranno delle linee in meno.
    STACK[ToS].frasi_stato = TOP_FRASI;
    if(TOP_FRASI == 0) STACK[ToS].FRASI = NULL;
    STATO_CURR = STACK[ToS].FRASI;
}

//PRINT//
void print(int ind1, int ind2){
    int i;

    if(ind2 == 0 || CURR_CMD->frasi_stato == 0){
        for (i=0; i<ind2-ind1+1; i++){
            fputs(".\n", stdout);
        }
        return;
    }

    if(ind1==0){ // Punto iniziale aggiuntivo.
        fputs(".\n", stdout);
        ind1++;
    }
    for (i=ind1; i<ind2+1; i++){
        if (i < CURR_CMD->frasi_stato+1){
            if(STATO_CURR == NULL) STATO_CURR = STATO_WO;
            fputs(STATO_CURR[i], stdout);
        }
        else fputs(".\n", stdout);
    }
}

//UNDO//
void  undo(ind1){
    int i;
    Record * cima;
    if(ind1 > COMANDI) ind1=COMANDI;
    WRITEONLY=0;
    cima = CURR_CMD-ind1;

    if(cima>=WO_lo && cima<=WO_hi){
        cima->FRASI = malloc(sizeof(char *) * (cima->indice2+1));
        for(i=1; i<cima->indice2+1; i++){
            cima->FRASI[i] = STATO_WO[i];
        }
        STATO_CURR = cima->FRASI;
        TOP_FRASI = cima->frasi_stato;
    }
    else {
        STATO_CURR = cima->FRASI;
        TOP_FRASI = cima->frasi_stato;
    }
    CURR_CMD=cima;
    UNDO_FATTI = UNDO_FATTI + ind1;
    COMANDI = COMANDI - ind1;
}

//REDO//
void redo(ind1){
    int i;
    Record * cima;
    if(ind1 > UNDO_FATTI) ind1=UNDO_FATTI;
    WRITEONLY=0;
    cima = CURR_CMD+ind1;

    if(cima>=WO_lo && cima<=WO_hi){
        cima->FRASI = malloc(sizeof(char *) * (cima->indice2+1));
        for(i=1; i<cima->indice2+1; i++){
            cima->FRASI[i] = STATO_WO[i];
        }
        STATO_CURR = cima->FRASI;
        TOP_FRASI = cima->frasi_stato;
    }
    else {
        STATO_CURR = cima->FRASI;
        TOP_FRASI = cima->frasi_stato;
    }
    CURR_CMD=cima;
    UNDO_FATTI = UNDO_FATTI - ind1;
    COMANDI = COMANDI + ind1;
}

//MAIN//
int main() {
    STACK = malloc(sizeof(Record) * 1);
    STACK[0].comando='z';
    STACK[0].indice1=0;
    STACK[0].indice2=0;
    STACK[0].use=0;
    STACK[0].frasi_stato=0;
    STACK[0].FRASI = NULL;
    STATO_WO = malloc(sizeof(char *) * 1);
    char * temp, * newtemp;
    int ind1, ind2;
    int k;

    do{
        if(!BUFFER_FULL) fgets(BUFFER, MAX, stdin);
        if(*BUFFER=='q') return 0;
        temp=BUFFER;
        ind1=atoi(temp);
        while(*temp >= '0' && *temp<='9'){ // Finché punta un numero, scorro il comando.
            temp++;
        }
        if(*temp == 'u'){ //Entra in Undo.
            do{
                fgets(BUFFER, MAX, stdin);
                BUFFER_FULL = 1;
                newtemp = BUFFER;
                k = atoi(newtemp);
                while(*newtemp >= 48 && *newtemp<= 57){ // Finché ho numeri scorro.
                    newtemp++;
                }
                if(*newtemp == 'u'){ //Entra in Undo.
                    ind1 = ind1 + k;
                }
                else break;
            }while(1);
            if(COMANDI==0) continue;
            if(ind1 > 0) undo(ind1);
            continue;
        }
        else if(*temp == 'r'){ // Entra in Redo.
            do{
                fgets(BUFFER, MAX, stdin);
                BUFFER_FULL = 1;
                newtemp = BUFFER;
                k = atoi(newtemp);
                while(*newtemp >= 48 && *newtemp<= 57){ // Finché ho numeri scorro.
                    newtemp++;
                }
                if(*newtemp == 'r'){ // Entra in Redo.
                    ind1 = ind1 + k;
                }
                else break;
            }while(1);
            if(UNDO_FATTI == 0) continue;
            redo(ind1);
            continue;
        }
        temp++;
        ind2=atoi(temp);
        while(*temp >= 48 && *temp<= 57){ // Finché non arrivo alla virgola scorro.
            temp++;
        }
        if(*temp == 'c'){
            change(ind1, ind2);
            ToS++;
            COMANDI++;
            BUFFER_FULL = 0;
            continue;
        }
        else if(*temp == 'd'){
            delete(ind1, ind2);
            ToS++;
            COMANDI++;
            BUFFER_FULL = 0;
            continue;
        }
        else if(*temp == 'p'){
            print(ind1, ind2);
            BUFFER_FULL = 0;
            continue;
        }
    }while(1);
}
//Dario Del Gaizo