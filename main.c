#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ASCII 256
#ifndef _MYBITOPS
#define _MYBITOPS
#define B_B_P "%c%c%c%c%c%c%c%c"
#define B2B(byte)  \
  (byte & 0x80 ? '1' : '0'),   (byte & 0x40 ? '1' : '0'),   (byte & 0x20 ? '1' : '0'),   (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'),   (byte & 0x04 ? '1' : '0'),   (byte & 0x02 ? '1' : '0'),   (byte & 0x01 ? '1' : '0')
typedef unsigned char uc;
typedef unsigned long ul;
void setBit(uc *byte, uc bit_number); // Set the i-th bit to 1
void clearBit(uc *byte, uc bit_number); // Set the i-th bit to 0
void toggleBit(uc *byte, uc bit_number); // Complement i-th bit
uc bitStatus(uc *byte, uc bit_number); // Get the status of i-th bit, returns 0 or 1
#endif

void setBit(uc *byte, uc bit_number) { // Set the i-th bit to 1
    (*byte) |= 1 << bit_number;
}

void clearBit(uc *byte, uc bit_number) { // Set the i-th bit to 0
    (*byte) &= ~(1 << bit_number);
}

void toggleBit(uc *byte, uc bit_number) { // Complement i-th bit
    (*byte) ^= 1 << bit_number;
}

uc bitStatus(uc *byte, uc bit_number) { // Get the status of i-th bit, returns 0 or 1
    return ((*byte & 1 << bit_number) > 0 ? 1 : 0);
}

typedef struct node {
    uc symbol;
    ul frequency;
    struct node *left;
    struct node *right;
} node;

typedef struct car {
    uc symbol;
    struct car *next;
} car;
typedef struct carCode {
    uc encoding;
    struct carCode *next;
} carCode;

typedef struct final_enc {
    uc symbol;
    ul frequency;
    uc encoding;
} final_enc;

typedef struct {
    int frequency;
    uc symbol;
} header;

node *table[ASCII];
final_enc can_list[ASCII];


int ch;
int countChars;
int countCodes;
int countCharsD;
int countCodesD;
clock_t start,end;
double tempo, tempoD;
car *listOfCars = NULL;


void deComp(char *c_out, char *c_outAfterDeComp);

void decoding();

node *createNode(uc symbol) {
    node *tmp = (node *) malloc(sizeof(node));
    tmp->symbol = symbol;
    tmp->frequency = 0;
    tmp->left = NULL;
    tmp->right = NULL;
    return tmp;
}

void createTreeTable() {
    for (int i = 0; i < 256; i++) {
        table[i] = createNode(i);
    }
}

void addCar(uc symbol) {
    if (listOfCars == NULL) {
        listOfCars = (car *) malloc(sizeof(car));
        listOfCars->symbol = symbol;
        listOfCars->next = NULL;
    } else {
        car *t1;
        t1 = listOfCars;
        while (t1->next != NULL) {
            t1 = t1->next;
        }
        car *tmp = (car *) malloc(sizeof(car));
        t1->next = tmp;
        tmp->symbol = symbol;
        tmp->next = NULL;
    }
    //printf(" %d ", symbol);
}

int readFileAndComputeFrequencies(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Problem opening file\n");
        return EXIT_FAILURE;
    }
    /*if (ferror(file) != 0) {
        printf("Problem opening file\n");
        return EXIT_FAILURE;
    }*/
    while (EOF != (ch = fgetc(file))) { // int ch
        uc c = (uc) ch;
        table[c]->frequency++;
        addCar(c);
        //printf("frequency  %d ", table[c]->frequency);
        countChars++;
    }
    fclose(file);
    //printf("\ncount of chars: %d\n", countChars);
    return EXIT_SUCCESS;
}

void sortTable() {
    //printf("Sorting frequencies, bubble sort!\n");
    for (int i = 0; i < 256; i++) {
        for (int j = i + 1; j < 256; j++) {
            if (table[i]->frequency < table[j]->frequency) {
                node *tmp = table[j];
                table[j] = table[i];
                table[i] = tmp;
            }
        }
    }
}

void saveFromTableToCanList(){
    int r=0;
    for(int i=0; i<ASCII; i++){
        if(table[i]->frequency>0){
            can_list[r].symbol=table[i]->symbol;
            can_list[r].frequency=table[i]->frequency;
            can_list[r].encoding=0;
            //printf("\n codifica %d %d "B_B_P"", can_list[r].symbol, can_list[r].frequency, B2B(can_list[r].encoding));
            r++;
            countCodes=r;
        }
    }
}
void printCanList(){
    for(int i=0; i<ASCII; i++){
        if(can_list[i].frequency>0){
            if(i==0 || i==ASCII-2 || i==ASCII-1){
                printf("\n number %d -> codifica %d %d "B_B_P"", i, can_list[i].symbol, can_list[i].frequency, B2B(can_list[i].encoding));
            }

        }
    }
}

void encoding() {
    int i = 0;
    toggleBit(&can_list[i].encoding, 0);
    //printf("\n i= %d codifica %d %d "B_B_P"", i, can_list[i].symbol, can_list[i].frequency, B2B(can_list[i].encoding));
    i++;
    while (can_list[i].frequency > 0 && i<256) {
        int true = 1;
        can_list[i].encoding = can_list[i - 1].encoding;
        while (true == 1) {
            for (int e = 0; e < 8; e++) {
                if (bitStatus(&can_list[i].encoding, e) == 0) {
                    toggleBit(&can_list[i].encoding, e);
                    true = 0;
                    e = 8;
                } else {
                    toggleBit(&can_list[i].encoding, e);
                }
            }
        }
        if(i==255){
            toggleBit(&can_list[i].encoding, 0);
        }
        //printf("\n i= %d codifica %d %d "B_B_P"", i, can_list[i].symbol, can_list[i].frequency, B2B(can_list[i].encoding));
        i++;
    }
}

void buildTree() {
    //printf("Building tree\n");
    for (int i = 0; i < 255; i++) {
        int j = 255 - i;
        int k = 255 - i - 1;
        node *tmp = createNode(0);
        tmp->frequency = table[j]->frequency + table[k]->frequency;
        tmp->left = table[j];
        tmp->right = table[k];
        table[k] = table[j] = NULL;
        k--; // Exploiting Insertion Sort functioning mechanism
        while ((k >= 0) && (table[k]->frequency < tmp->frequency)) {
            table[k + 1] = table[k];
            k--;
        }
        table[k + 1] = tmp;
    }
}

void deleteTree(node *tree) {
    if (tree == NULL) return;
    deleteTree(tree->left);
    deleteTree(tree->right);
    free(tree);
}

void printTree(node *tree) {
    if (tree == NULL || tree->frequency == 0) {
        return;
    } else {
        printf("\n first node symbol %d - frequency %d", tree->symbol, tree->frequency);
    }
    printf("\n left node");
    printTree(tree->left);
    printf("\n right node");
    printTree(tree->right);
}

uc findEncoding(uc symbol){
    for(int i=0; i<countCodes; i++){
        if(symbol==can_list[i].symbol){
            return can_list[i].encoding;
        }
    }
}

void numberOfItems_header(header header_array[]) {
    int y=0;
    for(int u = 0; u < ASCII; u++){
        header_array[u].frequency=0;
        header_array[u].symbol=0;
    }
    for (int i = 0; i < ASCII; i++) {
        if (can_list[i].frequency > 0) {
            header_array[i].frequency = can_list[i].frequency;
            header_array[i].symbol = can_list[i].symbol;
            y=i;
        }
    }
}
void write_header(header a[], int items, const char * fileName) {
    FILE* ptr = fopen(fileName,"wb");
    if(! ptr) return;
    fwrite( a, sizeof(header), items, ptr);
    // save list of encodings to file
    int ee=0;
    uc encoding2;
    while (listOfCars != NULL) {
        uc encoding = findEncoding(listOfCars->symbol);
        fwrite(&encoding, sizeof(uc), 1, ptr);
        /*if(ee==0){
            printf(" %d-"B_B_P" ",ee, B2B(encoding));
        }*/
        ee++;
        encoding2=encoding;
        listOfCars = listOfCars->next;
    }
    //printf(" %d-"B_B_P" ",ee, B2B(encoding2));
    //printf("list of encodings wrote ok ");
    fclose(ptr);
    //printf("\nfile closed\n");
}

void saveToFile(char *filename) {
    header header_array[ASCII];
    numberOfItems_header(header_array);
    write_header(header_array, ASCII, filename);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void compress(char *file_name_in, char *file_name_out) {
    start=clock();
    printf("Please wait...");
    createTreeTable();
    printf("\ncompleted 10%%");
    readFileAndComputeFrequencies(file_name_in);
    printf("\ncompleted 20%%");
    sortTable();
    saveFromTableToCanList(); // from table[ASCII] to can_list[ASCII]
    printf("\ncompleted 40%%");
    encoding();
    printf("\ncompleted 60%%");
    //printCanList();
    //buildTree();
    printf("\ncompleted 80%%");
    saveToFile(file_name_out);
    printf("\ncompleted 100%%");
    end=clock();
    tempo=((double)(end-start))/CLOCKS_PER_SEC;
    printf("\nFile has been compressed successfully in %.3lf seconds!", tempo);
    // delete data
    deleteTree(table[0]);
    listOfCars=NULL;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
carCode *listOfCodesD;
final_enc can_list2[ASCII];
typedef struct savetf *Header2;

int read_header(header a[], const char * fileName) {
    FILE* ptr = fopen(fileName,"rb");
    if( !ptr ) return 0;
    int n = 0;
    //printf(" fopen ok \n");
    for (n=0; n<ASCII; n++) {
        fread(&a[n],sizeof(header),1,ptr);
        /*if(n==0 || n==ASCII-2 || n==ASCII-1){
            printf("%d - %d : %d\n",n, a[n].symbol, a[n].frequency);
        }*/
    }
    //printf(" header finished ");

    countCodesD=0;
    countCharsD=0;
    //printf("\nStep 1 - lettura codes: \n");

    listOfCodesD = (carCode *) malloc(sizeof(carCode));
    carCode *t3;
    t3 = listOfCodesD;
    int ww=0;
    uc c4;
    ch = fgetc(ptr);
    while (EOF != ch) {
        uc c3 = (uc) ch;
        /*if(ww==0){
            printf(" %d - reading FFirst "B_B_P" ",ww, B2B(c3));
        }*/
        t3->encoding=c3;
        t3->next=NULL;
        countCharsD++;
        ch = fgetc(ptr);
        if(EOF != ch){
            t3->next=(carCode *) malloc(sizeof(carCode));
            t3=t3->next;
        }
        ww++;
        c4=c3;
    }
    //printf(" %d - reading LLast "B_B_P" ",ww-1, B2B(c4));
    //printf("\n \nlist of codes finished ");
    fclose(ptr);

    return n;
}
void decoding(header header_array[]) {
    for(int u = 0; u < ASCII; u++){
        can_list[u].frequency=0;
        can_list[u].symbol=0;
    }
    for (int i = 0; i < ASCII; i++) {
        can_list2[i].frequency=header_array[i].frequency;
        can_list2[i].symbol=header_array[i].symbol;
    }
    int p = 0;
    while (can_list2[p].frequency > 0) {
        countCodesD++;
        int true = 1;
        can_list2[p].encoding = can_list2[p - 1].encoding;
        while (true == 1) {
            for (int e = 0; e < 8; e++) {
                if (bitStatus(&can_list2[p].encoding, e) == 0) {
                    toggleBit(&can_list2[p].encoding, e);
                    true = 0;
                    e = 8;
                } else {
                    toggleBit(&can_list2[p].encoding, e);
                }
            }
        }
        if(p==255){
            toggleBit(&can_list2[p].encoding, 0);
        }
        //printf("\n p = %d decodifica %d %d "B_B_P"", p, can_list2[p].symbol, can_list2[p].frequency, B2B(can_list2[p].encoding));
        p++;
    }
}
void decode_listOfCodesAndSaveToFile(char *c_outAfterDeComp) {
    //printf("\n count of chars %d ", countCharsD);     printf("count of codes %d\n ", countCodesD);
    FILE *file2 = fopen(c_outAfterDeComp, "wb");
    //car *listOfCarsD = NULL;
    carCode *r2;
    r2= listOfCodesD;
    //printf("list of codes: ");
    //printf("\nStep 4 - decodifica e salvataggio nel file di output (file_3): \n");
    while (r2 != NULL) {
        //printf("\n "B_B_P" ", B2B(r2->encoding));
        for (int t = 0; t < countCodesD; t++) {
            //printf("\n %d "B_B_P" ", can_list2[t].symbol, B2B(can_list2[t].encoding));
            int l1, l2, l3, l4, l5, l6, l7, l8;                   int k1, k2, k3, k4, k5, k6, k7, k8;
            l1 = bitStatus(&r2->encoding, 0);            k1 = bitStatus(&can_list2[t].encoding, 0);
            l2 = bitStatus(&r2->encoding, 1);            k2 = bitStatus(&can_list2[t].encoding, 1);
            l3 = bitStatus(&r2->encoding, 2);            k3 = bitStatus(&can_list2[t].encoding, 2);
            l4 = bitStatus(&r2->encoding, 3);            k4 = bitStatus(&can_list2[t].encoding, 3);
            l5 = bitStatus(&r2->encoding, 4);            k5 = bitStatus(&can_list2[t].encoding, 4);
            l6 = bitStatus(&r2->encoding, 5);            k6 = bitStatus(&can_list2[t].encoding, 5);
            l7 = bitStatus(&r2->encoding, 6);            k7 = bitStatus(&can_list2[t].encoding, 6);
            l8 = bitStatus(&r2->encoding, 7);            k8 = bitStatus(&can_list2[t].encoding, 7);
            // converts codes to chars
            //printf("\n %d "B_B_P" "B_B_P" ",t, B2B(can_list2[t].encoding), r2->encoding);
            if (l1 == k1 && l2 == k2 && l3 == k3 && l4 == k4 && l5 == k5 && l6 == k6 && l7 == k7 && l8 == k8) {
               fwrite(&can_list2[t].symbol, sizeof(uc), 1, file2);
               //printf(" %d ", can_list2[t].symbol);
            }
        }
        r2 = r2->next;
    }
    fclose(file2);
}

void deComp(char *c_in2, char *c_outAfterDeComp) {
    start=clock();
    printf("Please wait... ");
    //read_file_binario(c_in2);
    header * a = malloc(sizeof(header) * ASCII);
    read_header( a, c_in2);
    printf("\ncompleted 30%%");
    decoding(a);
    printf("\ncompleted 60%%");
    decode_listOfCodesAndSaveToFile(c_outAfterDeComp);
    printf("\nompleted 100%%");
    end=clock();
    tempoD=((double)(end-start))/CLOCKS_PER_SEC;
    printf("\nFile has been decompressed successfully in %.3lf seconds!",tempoD);
    listOfCodesD=NULL;
    free (a);
}

int readFiles(char *filename1, char *filename2) {
    FILE *file1 = fopen(filename1, "rb");
    int yy=0;
    printf("\nReading file1\n");
    while (EOF != (ch = fgetc(file1))) {
        uc c = (uc) ch;
        printf(" %d-%d ",yy, c);
        yy++;
    }
    fclose(file1);
    /*FILE *file2 = fopen(filename2, "rb");
    int tt=0;
    printf("\nReading file2\n");
    while (EOF != (ch = fgetc(file2))) {
        uc c = (uc) ch;
        printf(" %d-%d ",tt, c);
        tt++;
    }
    fclose(file2);*/
    return EXIT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    if (argc - 1 != 3) {
        fprintf(stderr,
                "\nSi prega di specificare: 1) il comando da eseguire (-c; -d);\n2) il file di input;\n3) il file di destinazione.\n");
        exit(1);
    } else {

        if (strcmp(argv[1], "-c") == 0) {
            compress(argv[2], argv[3]);

        } else if (strcmp(argv[1], "-d") == 0) {
            deComp(argv[2], argv[3]); // decomprimere (c_out, c_in2); // diff cin.txt cin2.txt

        } else if(strcmp(argv[1], "-f") == 0){
            readFiles(argv[2], argv[3]); // stampa contenuto binario di due file
        } else {
            fprintf(stderr,
                    "\nLe opzioni sono: -c per comprimere e -d per decomprimere. Si prega di specificare l'opzione da eseguire, il file di input ed il file di destinazione.\n");
            exit(1);
        }
    }
    return 0;
}
