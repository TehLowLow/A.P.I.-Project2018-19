#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define DEBUG 0


typedef enum Bool {
    false, true
} bool;

struct EntTable {

    unsigned int entNumber;  //Numero di entità hashate sotto questo indice

    struct PlainEnt *entEntries; //Array che contiene tutte le entità hashate sotto un indice
};


struct PlainEnt {

    char *entName; //Nome dell' entità

    unsigned int destCounter; //Int per il conteggio di sorgenti nel report.

    struct PlainRel **relKeys; // Array per il backtracking, contiene tutte le chiavi di relazioni in cui la PlainEnt è coinvolta
};


struct RelTable {

    unsigned int relNumber; //Numero di entità hashate sotto questo indice

    struct PlainRel *relEntries; //Array che contiene tutte le relazioni hashate sotto un indice
};


struct PlainRel {

    char *relName; //Nome della relazione hashata sotto questo indice

    unsigned int cplNumber; //Numero di elementi(a coppie) presenti nell' array binded

    struct Couples *binded;  //Array di coppie di entità collegate dalla relazione
};


struct Couples {  //coppia sorgente destinazione collegata da una relazione

    struct PlainEnt *source;

    struct PlainEnt *destination;
};

//-----PROTOTYPES-------------------------------------------------------------------------------------------------------

int hash64(char input);

struct PlainEnt *EntityLookup(char *inputName, unsigned int tableHash, struct EntTable *entHash);

void PrintArray(struct PlainEnt **array, unsigned int arrayCounter);


//-----MEMORY INIT------------------------------------------------------------------------------------------------------

//Questa funzione crea la tabella di hashing per le entità. Gestisce 64*64 indici (hashing numerico per i primi due
// caratteri) più una colonna speciale in caso di entità con nome composto da 1 carattere.


struct EntTable *initEntHash() {


    struct EntTable *hash = NULL;

    hash = calloc(4097, sizeof(struct EntTable));

    return hash;

}


//----------------------------------------------------------------------------------------------------------------------

//Questa funzione crea la tabella di hashing per le relazioni. Gestisce 64*64 indici (hashing numerico per i primi due
// caratteri) più una colonna speciale in caso di relazioni con nome composto da 1 carattere.

struct RelTable *initRelHash() {

    struct RelTable *hash = NULL;

    hash = calloc(4097, sizeof(struct RelTable));

    return hash;

}

//-----COMMANDS---------------------------------------------------------------------------------------------------------

//Questa funzione viene chiamata quando in input leggo il comando addent, ne calcolo l hash, e aggiungo in coda
//l' entità passata dal comando, sse questa non è gia presente nella tabella.


void HashInputEnt(struct EntTable *hashTable) {

    char *inputEnt = NULL;
    int tableIndex;
    struct PlainEnt *result = NULL;


    scanf("%ms", &inputEnt);      //Leggo l' entità

    if (strlen(inputEnt) > 3) {
        tableIndex = hash64(inputEnt[1]) * 64 + hash64(inputEnt[2]);   //Se ha più di 1 carattere, hashing
    } else {
        tableIndex = 4096;  //Altrimenti array di singletons
    }

    result = EntityLookup(inputEnt, tableIndex, hashTable);

    if (result == NULL) { //Se la tabella hash non ha ancora entità hashate con quella chiave, alloco

        if (hashTable[tableIndex].entEntries == NULL) {  //TODO 1

            hashTable[tableIndex].entNumber = 1;

            hashTable[tableIndex].entEntries = calloc(1, sizeof(struct PlainEnt));

            hashTable[tableIndex].entEntries[0].entName = malloc(1);

            strcpy(hashTable[tableIndex].entEntries[0].entName, inputEnt);


        } else {

            hashTable[tableIndex].entNumber++;

            unsigned int temp = hashTable[tableIndex].entNumber;

            //Aggiungo una casella in cui salvare l' entità e la aggiungo

            hashTable[tableIndex].entEntries = realloc(hashTable[tableIndex].entEntries,
                                                       temp * sizeof(struct PlainEnt));


            hashTable[tableIndex].entEntries[temp - 1].entName = malloc(
                    strlen(inputEnt)); //cerco un indirizzo per la stringa. Segfaulta altrimenti

            strcpy(hashTable[tableIndex].entEntries[temp - 1].entName, inputEnt);

        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

//Questa funzione viene chiamata quando in input leggo il comando addrel, ne calcolo l hash, e aggiungo la realzione
//passata dal comando, sse questa non è gia presente nella tabella.

void HashInputRel(struct RelTable *relHashTable, struct EntTable *entHashTable) {

    char *src;
    char *dest;
    char *inputRel;
    int hashedSrc = 0;
    int hashedDest = 0;
    int tableIndex = 0;
    struct PlainEnt *srcFound = NULL;
    struct PlainEnt *destFound = NULL;
    struct PlainEnt *relFound = NULL;
    unsigned int ordered;
    unsigned int bufferCounter;

    //Leggo i parametri del comando

    scanf("%ms", &src);
    scanf("%ms", &dest);
    scanf("%ms", &inputRel);

    //Per prima cosa verifico che le due entità esistano nella EntTable

    if (strlen(src) > 3) {  //Calcolo l' hash
        hashedSrc = hash64(src[1]) * 64 + hash64(src[2]);
    } else {
        hashedSrc = 4096;
    }

    srcFound = EntityLookup(src, hashedSrc, entHashTable); //Cerco la sorgente

    if (srcFound != NULL) { //Se non trovo la sorgente è inutile cercare la dest, tanto poi scarterei il comando

        if (strlen(dest) > 3) {  //Cerco la destinazione
            hashedDest = hash64(dest[1]) * 64 + hash64(dest[2]);
        } else {
            hashedDest = 4096;
        }

        destFound = EntityLookup(dest, hashedDest, entHashTable);

    }


    //Ho trovato entrambe, proseguo. Se non trovo entrambe non faccio nulla.

    if (srcFound != NULL && destFound != NULL) {

        if (strlen(inputRel) > 3) {

            tableIndex = hash64(inputRel[1]) * 64 + hash64(inputRel[2]);

        } else {

            tableIndex = 4096;
        }

        //Con l' hash di dove dovrebbe collocarsi questa relazione, verifico se effettivamente è gia stata caricata.

        for (unsigned int a = 0;
             a < relHashTable[tableIndex].relNumber; a++) { // a è l' indice di relazione nell' array della hash

            if (strcmp(relHashTable[tableIndex].relEntries[a].relName, inputRel) == 0) {
                //L'ho trovata, verifico se ha gia delle coppie oppure se questa è la prima

                if (relHashTable[tableIndex].relEntries[a].binded ==
                    NULL) {//Se questa coppia è la prima, alloco e aggiungo in testa.         //TODO 1

                    relHashTable[tableIndex].relEntries[a].cplNumber = 1;

                    unsigned int cplIndex = relHashTable[tableIndex].relEntries[a].cplNumber;

                    relHashTable[tableIndex].relEntries[a].binded = calloc(1, sizeof(struct Couples));
                    relHashTable[tableIndex].relEntries[a].binded[0].source = srcFound;
                    relHashTable[tableIndex].relEntries[a].binded[0].destination = destFound;

                    //TODO 5

                    return;

                } else {

                    //C'è gia gente allocata, devo verificare che questa coppia non esista ancora per quella relazione,
                    // altrimenti non faccio nulla.                       TODO 2

                    for (unsigned int b = 0; b < relHashTable[tableIndex].relEntries[a].cplNumber; b++) {
                        // b è l' indice di coppia per verificare se la relazione gia esiste

                        if (strcmp(relHashTable[tableIndex].relEntries[a].binded[b].source->entName, src) == 0 &&
                            strcmp(relHashTable[tableIndex].relEntries[a].binded[b].destination->entName, dest) == 0) {

                            //Esiste gia, ritorno
                            return;

                        }
                    }

                    //Non esiste, devo aggiungere in coda la coppia di entità. Incremento il numero di couples, rialloco l' array e assegno i puntatori source e dest alla nuova couple.

                    relHashTable[tableIndex].relEntries[a].cplNumber++;

                    unsigned int cplNumbTemp = relHashTable[tableIndex].relEntries[a].cplNumber;

                    struct Couples *tempBind = realloc(relHashTable[tableIndex].relEntries[a].binded,
                                                       relHashTable[tableIndex].relEntries[a].cplNumber *
                                                       sizeof(struct PlainRel));

                    relHashTable[tableIndex].relEntries[a].binded = tempBind;

                    relHashTable[tableIndex].relEntries[a].binded[cplNumbTemp - 1].source = srcFound;

                    relHashTable[tableIndex].relEntries[a].binded[cplNumbTemp - 1].destination = destFound;

                    //TODO 5

                    return;

                }


            }
        }

        //Qua devo invece aggiungere la nuova relazione che non ho mai incontrato, e la aggiungo gia in ordine



        if (relHashTable[tableIndex].relNumber == 0) {

            // Se la table è vuota inserisco subito in testa, e aggiungo le due entità

            relHashTable[tableIndex].relNumber++;

            relHashTable[tableIndex].relEntries = malloc(1);

            relHashTable[tableIndex].relEntries = calloc(relHashTable[tableIndex].relNumber,
                                                         sizeof(struct PlainRel));  //Alloco la prima cella di entries e la nomino

            relHashTable[tableIndex].relEntries[0].relName = malloc(1);

            strcpy(relHashTable[tableIndex].relEntries[0].relName, inputRel);

            relHashTable[tableIndex].relEntries[0].cplNumber++;

            relHashTable[tableIndex].relEntries[0].binded = calloc(1,
                                                                   sizeof(struct Couples)); // Alloco la prima cella di Coppie e le assegno

            relHashTable[tableIndex].relEntries[0].binded[0].source = srcFound;

            relHashTable[tableIndex].relEntries[0].binded[0].destination = destFound;

            //TODO 5

            return;

        }

        //Se la table non è vuota, devo cercare l' indice

        ordered = 0;

        for (unsigned int a = 0; a < relHashTable[tableIndex].relNumber; a++) {

            ordered = a;

            if (strcmp(inputRel, relHashTable[tableIndex].relEntries[a].relName) < 0) {

                break;

            }

        }


        //ordered ora contiene l' indice di dove devo piazzare la nuova relazione, devo riallocare

        relHashTable[tableIndex].relNumber++;

        struct PlainRel *buffer = calloc(relHashTable[tableIndex].relNumber, sizeof(struct PlainRel));

        bufferCounter = 0;

        while (bufferCounter < ordered) { //Copio tutti quelli prima del posto in cui inserire la nuova struct

            strcpy(buffer[bufferCounter].relName,
                   relHashTable[tableIndex].relEntries[bufferCounter].relName);  //Copio il nome
            buffer[bufferCounter].cplNumber = relHashTable[tableIndex].relEntries[bufferCounter].cplNumber; //Copio il numero di coppie
            buffer[bufferCounter].binded = relHashTable[tableIndex].relEntries[bufferCounter].binded; //Copio il ptr all' array di coppie

            bufferCounter++; //Incremento e ripeto

        }

        bufferCounter = ordered; //Inserisco la nuova struct. La copia è identica a quella scritta sopra e a quella successiva.

        strcpy(buffer[bufferCounter].relName, inputRel);
        buffer[bufferCounter].cplNumber = 1;
        buffer[bufferCounter].binded = calloc(1, sizeof(struct Couples));


        bufferCounter++;

        while (bufferCounter < relHashTable[tableIndex].relNumber) { //Copio tutti i successivi

            strcpy(buffer[bufferCounter].relName, relHashTable[tableIndex].relEntries[bufferCounter].relName);
            buffer[bufferCounter].cplNumber = relHashTable[tableIndex].relEntries[bufferCounter].cplNumber;
            buffer[bufferCounter].binded = relHashTable[tableIndex].relEntries[bufferCounter].binded;

            bufferCounter++;


        }

        free(relHashTable[tableIndex].relEntries); //Libero il vecchio array precedente all' inserimento

        relHashTable[tableIndex].relEntries = buffer;  //Gli assegno il ptr del nuovo array ordinato

        //Ci aggiungo la nuova relazione

        relHashTable[tableIndex].relEntries[ordered].binded[0].source = srcFound;
        relHashTable[tableIndex].relEntries[ordered].binded[0].destination = destFound;

        return;
    }
}



/*Ordinamento Lessicografico
 *
 * Uso strcmp
 *
 * Entro nell' array e faccio strcmp della stringa in input con tutte le stringhe in memoria,al primo indice in cui
 * ho minore scorrendo dall' inizio esco dal ciclo e so che devo infilarla li.
 *
 *
 * Funzionamento generale:
 *
 *
 * leggo e salvo;
 * entro nella tabella sotto l' hash che calcolo;
 * scorro l' array dall' inizio facendo
 *
 *
 * (for every_relation){
 * if(strcmp(input, hashtable)<0){la parola va in quell' indice, return indice}}
 *
 *
 * Non uso realloc, perchè mi copia anche i dati, ma devo pensare a una mia realloc, che copi i dati prima dell' indice
 * uguali, all' indice inserisca la nuova relazione, e poi copi i dati successivi sfasati di un indice.
 *
 */


//TODO 3
//----------------------------------------------------------------------------------------------------------------------

void DeleteEnt() {};

//----------------------------------------------------------------------------------------------------------------------

void DeleteRel() {};

//----------------------------------------------------------------------------------------------------------------------

//Il report deve scorrere tutta la tabella hash, e stampare tutte le relazioni che hanno almeno una coppia con il loro max ricevente
//Il conteggio dei max riceventi lo faccio a tempo di report.
//L' idea è, scorro per ogni relazione l' array di coppie, entro negli indirizzi e nella tabella di entità incremento
//un contatore. Una volta incrementato tutti i contatori, rileggo l' array e trovo l' int più grande. Scorro una terza volta
// e salvo tutti coloro che come indice han quel valore e nel mentre resetto tutti gli indici a zero

//Complessità spaziale: un unsigned int per ogni entità
//Complessità temporale = Theta(3n)

//Si potrebbe fare meglio in tempo, ma in spazio è praticamente minimo, perche si tratta di un byte per ogni diverso destinatario, e un array con
// k elementi = numero di destinatari con valore max di sorgenti.




void Report(struct RelTable *relHash, struct EntTable *entHash) {

    struct PlainEnt **entBuffer = malloc(1 * sizeof(struct PlainEnt *));
    unsigned int bufferCounter = 0; //Dimensione dell'array


    for (int index = 0; index < 4097; index++) { //Per ogni chiave della hash

        if (relHash[index].relNumber != 0) { //Se la chiave corrisponde a delle relazioni

            for (unsigned int a = 0; a < relHash[index].relNumber; a++) {//Per ogni relazione di quella chiave

                for (unsigned int b = 0; b <
                                         relHash[index].relEntries[a].cplNumber; b++) {//Per ogni coppia, entro in dest e incremento il suo destCounter

                    relHash[index].relEntries[a].binded[b].destination->destCounter++;

                }

                bufferCounter = 1;

                entBuffer[0] = relHash[index].relEntries[a].binded[0].destination;  //Salvo di default la prima entità dell' array binded, che avrà un destinatario con valore x

                for (unsigned int c = 1; c < relHash[index].relEntries[a].cplNumber; c++) {

                    //Se trovo un valore maggiore di quello che consideravo il max allora flush dell' array, e quel dest diventa il nuovo max

                    if (relHash[index].relEntries[a].binded[c].destination->destCounter > entBuffer[0]->destCounter) {

                        free(entBuffer);

                        bufferCounter = 1;

                        entBuffer = calloc(1, sizeof(struct PlainEnt *));

                        entBuffer[0] = relHash[index].relEntries[a].binded[c].destination; //Salvo il nuovo max

                    } else if (relHash[index].relEntries[a].binded[c].destination->destCounter ==
                               entBuffer[0]->destCounter) {

                        //Se è uguale va aggiunto all'array di max, perchè sono equipotenti

                        bufferCounter++;

                        entBuffer = realloc(entBuffer, bufferCounter * sizeof(struct PlainEnt *));

                        entBuffer[bufferCounter -
                                  1] = relHash[index].relEntries[a].binded[c].destination; //Aggiunta in coda


                    }

                    //Se il valore è minore lo skippo, perchè ho gia il max
                }

                //Finito questi cicli, devo stampare i risultati secondo specifiche, e fare un ultimo scorrimento per
                //resettare a zero tutti i counter nella tabella.

                printf("%s", relHash[index].relEntries[a].relName); //Stampa nome relazione
                printf(" ");

                /*for (unsigned int d = 0; d < bufferCounter; d++) {

                    //Stampa i risultati, occhio che in caso di situazione di parità ( più entità con stesso numero di riceventi)
                    //van stampate in ordine alfabetico, creare una funzione di sort del buffer

                    //Prima implementazione per debug

                    printf("%s", entBuffer[d]->entName);


                }*/

                PrintArray(entBuffer, bufferCounter);

                printf("%d", entBuffer[0]->destCounter);  //Stampo il max ricevente

                printf("\n");

                for (unsigned int f = 0; f < relHash[index].relEntries[a].cplNumber; f++) {

                    relHash[index].relEntries[a].binded[f].destination->destCounter = 0;  //Resetto i counter per altri report.

                }


            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
//Questa funzione legge il main txt e si occupa di richiamare le diverse funzioni di parsing nel caso di comando su entità, comando su
//relazione o comando di flusso.

static inline bool ParseTxt(struct EntTable *entTable, struct RelTable *relTable) {

    char *inCommand = NULL;

    scanf("%ms", &inCommand);

    if (inCommand[0] == 'a') {

        if (strcmp(inCommand, "addent") == 0) {//Chiama la funzione che aggiunge un elemento all hash

            HashInputEnt(entTable);


            if (DEBUG) { printf("%s", inCommand); }
            return true;

        } else {//Chiama la funzione che aggiunge una relazione alla hash

            HashInputRel(relTable, entTable);


            if (DEBUG) { printf("%s", inCommand); }
            return true;

        }

    } else if (inCommand[0] == 'd') {

        if (strcmp(inCommand, "delent") == 0) { //chiama la funzione di elimina elemento


            if (DEBUG) { printf("%s", inCommand); }
            return true;


        } else {  //chiama la funzione di elimina relazione


            if (DEBUG) { printf("%s", inCommand); }
            return true;

        }


    } else if (inCommand[0] == 'r') {/*chiama il report*/

        Report(relTable, entTable);

        if (DEBUG) { printf("%s", inCommand); }
        return true;

    } else if (inCommand[0] == 'e') {/*termino*/ return false; }


}

//-----HELPERS----------------------------------------------------------------------------------------------------------

//Questa funzione calcola l indice dell' array in cui inserire l' entità o la relazione. La chiave ritornata è calcolata
//in base a una regola di riduzione dei caratteri validi in input da un indice base 64 a un indice decimale.

//La compressione dei valori ascii di un carattere a valori compresi fra 0 e 63 avviene come:

/* Se incontro il carattere "-", gli assegno il valore 0;
 * Se incontro una cifra sottraggo al suo valore ascii 47;
 * Se incontro una lettera maiuscola sottraggo 54
 * Se incontro il carattere "_" gli assegno il valore 37
 * Se incontro una lettera minuscola sottraggo 59
 */

//Una volta compressa la lettera, calcolo l hash come:

//Val_prima * 64 + val_seconda.

//Questo mi ritorna un indice compreso fra 0 e 4095, e lascia il valore indice 4096 per le entità che son più corte di
//2 caratteri.


inline int hash64(char input) {    //Gioele     71  105


    int hashed = 0;

    hashed = input;

    if (input == 45) {  //Carattere '-'

        hashed = 0;


    } else if (48 <= input && input <= 57) {  //Cifre 0...9

        hashed = input - 47;

    } else if (65 <= input && input <= 90) { //Lettere Maiuscole A..Z

        hashed = input - 54;


    } else if (input == 95) { //Carattere '_'

        hashed = 37;

    } else if (97 <= input && input <= 122) {  //Lettere minuscole a...z

        hashed = input - 59;

    }

    return hashed;

}
//----------------------------------------------------------------------------------------------------------------------

//Questa funzione scansiona la tabella di entità per verificare se un' entità esiste già in memoria.

struct PlainEnt *EntityLookup(char *inputName, unsigned int tableHash, struct EntTable *entHash) {

    for (unsigned int i = 0; i < entHash[tableHash].entNumber; i++) {

        if (strcmp(entHash[tableHash].entEntries[i].entName, inputName) == 0) {

            return &(entHash[tableHash].entEntries[i]);

        }
    }

    return NULL;
}



//----------------------------------------------------------------------------------------------------------------------

struct PlainRel *RelationLookup(char *inputName, unsigned int tableHash, struct RelTable *relHash) {

    for (unsigned int i = 0; i < relHash[tableHash].relNumber; i++) {

        if (strcmp(relHash[tableHash].relEntries[i].relName, inputName) == 0) {

            return &(relHash[tableHash].relEntries[i]);

        }
    }

    return NULL;
}
//----------------------------------------------------------------------------------------------------------------------

void PrintArray(struct PlainEnt **array, unsigned int arrayCounter) {

    struct PlainEnt **buffer = calloc(1, sizeof(struct PlainEnt *));

    unsigned int bufferCounter = 1;

    //Questa funzione riordina l' array e lo stampa su stdout

    buffer[0] = array[0];  //Di base il primo el lo copio, poi li riordino.

    if (arrayCounter == 2) { //Se l' array è lungo 2 per evitare overflow killer alloco manualmente in testa o in coda

        bufferCounter = 2;

        buffer = realloc(buffer, bufferCounter * sizeof(struct PlainEnt *));

        if (strcmp(array[1]->entName, buffer[0]->entName) < 0) {

            buffer[1] = buffer[0];
            buffer[0] = array[1];

        } else if (strcmp(array[1]->entName, buffer[0]->entName) == 0) {

            bufferCounter = 1;

        } else { //Caso specifico, se l' array è lungo due ma sono uguali skippa

            buffer[1] = array[1];
        }
    } else {

        for (unsigned int a = 1; a <
                                 arrayCounter; a++) {//Per tutti gli elementi in input (array di max con doppioni)  //TODO rotto questo ramo else

            for (unsigned int i = 0; i < bufferCounter; i++) { //Ciclo su ogni elemento ordinato

                unsigned int order = i;

                if (strcmp(array[a]->entName, buffer[i]->entName) < 0) { //Se array[a] viene prima di buffer

                    //Ho trovato l' indice, inserisco l entità

                    bufferCounter++;

                    buffer = realloc(buffer, bufferCounter * sizeof(struct PlainEnt *));

                    for (unsigned int j = bufferCounter - 2; j >
                                                             order; j--) { //Rialloco, sposto tutti di una casella in avanti e nel posto "order" ci metto il nuovo elemento


                        buffer[j + 1] = buffer[j];


                    }

                    buffer[order] = array[a];
                    break;


                } else if (strcmp(array[a]->entName, buffer[i]->entName) ==
                           0) { //Se sono uguali skippo, rimuovo i doppioni

                    //è un doppione, skippo l' esecuzione

                    break;


                }

                //L' aggiunta in coda si fa solo se order == buffercounter-1

                if (order == bufferCounter -
                             1) { //Aggiunta in coda se non trovo un indice, ovvero l' elemento in ordine alfabetico viene dopo tutti

                    //Aggiunta in coda

                    bufferCounter++;

                    buffer = realloc(buffer, bufferCounter * sizeof(struct PlainEnt *));

                    buffer[bufferCounter - 1] = array[a];

                }

            }


        }

    }

    for (unsigned int k = 0; k < bufferCounter; k++) {  //Stampa tutti i nomi

        printf("%s", buffer[k]->entName);
        printf(" ");

    }


}


//-----MAIN-------------------------------------------------------------------------------------------------------------


int main() {


    struct EntTable *entitiesHash = initEntHash();
    struct RelTable *relationHash = initRelHash();

    //Memoria usata per inizializzare le strutture dati: 128kB

    while (ParseTxt(entitiesHash, relationHash)) {

        //continue;

    }

    printf("FINITO");

}

/* TODO
 *        cercare come mai usavo static inline nelle funzioni.
 *        Cercare in tutto il programma dove eseguo delle strcpy senza prima inizializzare la stringa.
 *        Problemi con gli indci dei binded e degli array, rileggere il codice tutto in generale.
 *        Minor Improvement: Rimuovere in memoria i " " che comunque occupano due byte per ogni entry (poca roba ma magari...)
 *      1 ricorda nelle delete di impostare questo array a null se finiscono le entità, altrimenti si fotte
 *      2 L array di keys è inutile tenerlo di int, è più comodo risparmiare tempo a discapito dello spazio e salvare ptr alla relation in cui
 *        è contenuta l' entità monitorata. Diventa pericoloso in termini di spazio, ma sicuramente non dover fare continue scansioni della
 *        hash è conveniente in termini di tempo.
 *      3 Devo controllare di aver messo bene i return, perchè devo poter uscire dai for appena una condizione non è soddisfatta.
 *      4 Qua scansiono l'array di entità per vedere se trovo l' entità, ma dovrei controllare prima che ci sia, perchè se ancora non è inizializzato sarà null
 *        La condizione del for potrebbe salvarmi, perchè se entNumber è zero, esco subito dal for e sto a posto.
 *      5 Aggiungere il backtracking
 *
 *
 *
 */