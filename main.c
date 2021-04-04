#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASCII 256
#define TRUE 0
#define FALSE 1
#ifndef _MYBITOPS
#define _MYBITOPS
#define B_B_P "%c%c%c%c%c%c%c%c"
#define B2B(byte)  \
  (byte & 0x80 ? '1' : '0'),   (byte & 0x40 ? '1' : '0'),   (byte & 0x20 ? '1' : '0'),   (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'),   (byte & 0x04 ? '1' : '0'),   (byte & 0x02 ? '1' : '0'),   (byte & 0x01 ? '1' : '0')
typedef unsigned char uc;

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

typedef unsigned long ul;
typedef unsigned char uc;

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
typedef struct enc {
    uc symbol;
    uc codelen;
    ul frequency;
} enc;
typedef struct final_enc {
    uc symbol;
    uc length;
    uc encoding;
} final_enc;

node *table[ASCII];
enc code_table[ASCII];
enc *final_code_table[ASCII];
final_enc list[ASCII];
uc longest_code = 0;
unsigned long long total_bytes = 0;
int ch;
int countChars;
int countCodes;
int countCharsD;
int countCodesD;

void deComp(char *c_out, char *c_outAfterDeComp);
void encoding();
void decoding();

node *createNode(uc symbol) {
    node *tmp = (node *) malloc(sizeof(node));
    tmp->symbol = symbol;
    tmp->frequency = 0;
    tmp->left = NULL;
    tmp->right = NULL;
    return tmp;
}

car *listOfCars = NULL;

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
    //printf("car %d", symbol);
}

void createTreeTable() {
    for (int i = 0; i < 256; i++) {
        table[i] = createNode(i);
    }
}

void createCodeTable() {
    for (int i = 0; i < 256; i++) {
        code_table[i].codelen = 0;
        code_table[i].symbol = 0;
        code_table[i].frequency = 0;
        final_code_table[i] = NULL;
    }
}

int computeFrequencies(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Problem opening file\n");
        return EXIT_FAILURE;
    }
    int yy=0;
    //printf("Reading file and computing frequencies\n");
    while (EOF != (ch = fgetc(file))) {
        if (ferror(file) != 0) {
            printf("Problem opening file\n");
            return EXIT_FAILURE;
        }
        uc c = (uc) ch;
        table[c]->frequency++;
        total_bytes++;
        addCar(c);
        printf(" - %d added %d ",yy, c);
        countChars++;
        yy++;
    }
    fclose(file);
    printf("\ncount of chars: %d\n", countChars);
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
        printf("\n %d %d", tree->symbol, tree->frequency);
    }
    printTree(tree->left);
    printTree(tree->right);
}

void computeCodeLen(node *tree, uc codelen) {
    if (tree != NULL) {
        if (tree->left == NULL && tree->right == NULL) {
            code_table[tree->symbol].codelen = 255;
            if (tree->frequency > 0) {
                code_table[tree->symbol].symbol = tree->symbol;
                code_table[tree->symbol].frequency = tree->frequency;
                code_table[tree->symbol].codelen = codelen;
                if (codelen > longest_code)
                    longest_code = codelen;
            }
        }
        computeCodeLen(tree->left, codelen + 1);
        computeCodeLen(tree->right, codelen + 1);
    }
}

final_enc can_list[ASCII];

void create_final_list_encoding() {
    for (int i = 0; i < 256; i++) {
        list[i].symbol = 0;
        list[i].length = 0;
    }
    //printf("\nBefore sorting the final list of encoding");
    for (int i = 0; i < 256; i++) {
        if (code_table[i].frequency > 0) {
            list[i].symbol = code_table[i].symbol;
            list[i].length = code_table[i].codelen;
            //printf("\n%d %d", list[i].symbol, list[i].length);
        }
    }
    //printf("\nAfter sorting the final list of encoding");
    for (int i = 0; i < 256; i++) {
        for (int j = i + 1; j < 256; j++) {
            if (list[i].length > list[j].length) {
                final_enc tmp = list[j];
                list[j] = list[i];
                list[i] = tmp;
            }
        }
    }
    // print the list
    for (int i = 0; i < 256; i++) {
        if (list[i].length > 0) {
            //printf("\n%d %d", list[i].symbol, list[i].length);
        }
    }
    int y = 0;
    for (int i = 0; i < 256; i++) {
        if (list[i].length > 0) {
            can_list[y].symbol = list[i].symbol;
            can_list[y].length = list[i].length;
            y++;
        }
    }
    countCodes = y;
    printf("count of codes: %d \n",countCodes);
}

void encoding() {
    //printf("\n%3d %2x ("B_B_P")", can_list[0].symbol, can_list[0].length, B2B(can_list[0].encoding));
    int i = 0;
    while (can_list[i].length > 0) {
        int true = 1;
        can_list[i].encoding = can_list[i - 1].encoding;
        //printf("\n%3d %2x ("B_B_P")", can_list[i].symbol, can_list[i].length, B2B(can_list[i].encoding));
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
        i++;
    }
}

carCode *listOfCodes = NULL;

void addCarCodeAndSaveToFile(char *filename) {
    FILE *file = fopen(filename, "wb+");
    int y=0;
    for (int i = 0; i < ASCII; i++) {
        if (can_list[i].length > 0) {
            printf(" %d ", can_list[i].length);
            fwrite(&can_list[i].length, sizeof(uc), 1, file);
            y=i;
        }
    }
    can_list[y+1].length=0;
    fwrite(&can_list[y+1].length, sizeof(uc), 1, file);
    printf(" %d ", can_list[y+1].length);
    for (int i = 0; i < ASCII; i++) {
        if (can_list[i].length > 0) {
            printf(" %d ", can_list[i].symbol);
            fwrite(&can_list[i].symbol, sizeof(uc), 1, file);
        }
    }
    uc t=0;
    fwrite(&t, sizeof(uc), 1, file);
    printf(" %d ", t);

    carCode *t3;
    listOfCodes = t3;
    printf("\nlist of codes: ");
    while (listOfCars != NULL) {
        printf(" car %d - ", listOfCars->symbol);
        for (int i = 0; i < countCodes; i++) {
            //printf(" %d ", can_list[i].symbol);
            if (listOfCars->symbol == can_list[i].symbol) {
                t3= (carCode *) malloc(sizeof(carCode));
                t3->encoding = can_list[i].encoding;
                fwrite(&t3->encoding, sizeof(uc), 1, file);
                printf("("B_B_P") ", B2B(t3->encoding));
                t3->next = NULL;
            }
        }
        listOfCars = listOfCars->next;
    }
    printf("\nFile e' stato compresso");
    fclose(file);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void compress(char *file_name_in, char *file_name_out) {

    createTreeTable();
    createCodeTable();
    computeFrequencies(file_name_in);
    sortTable();
    buildTree();
    computeCodeLen(table[0], 0);
    //computeCanonicalEncoding
    //1. Sort code_table & save in list[total_bytes]
    create_final_list_encoding();
    //2. Compute canonical encoding for each item in the list
    encoding();
    //3. Encode the message and save to the output file
    addCarCodeAndSaveToFile(file_name_out);
    deleteTree(table[0]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
carCode *listOfCodesD;
final_enc can_list2[ASCII];

void read_file_binario(char *filename) {
    countCodesD=0;
    countCharsD=0;
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("\nIl file non esiste");
        exit(1);
    }
    while ((ch = fgetc(file)) != 0) {
        if (ferror(file) != 0) {
            printf("Problem opening file\n");
            EXIT_FAILURE;
        }
        int w = 0;
        uc c = (uc) ch;
        printf("\n reading first c = %d ", c);
        can_list2[w].length = c;
        //printf("length = %d ", can_list2[w].length);
        w++;
        countCodesD++;
        while (1) {
            fread(&c, sizeof(uc), 1, file);
            if (c == 0) {
                break;
            }
            printf(" - reading c = %d ", c);
            can_list2[w].length = c;
            //printf("length = %d ", can_list2[w].length);
            w++;
            countCodesD++;
            //printf(" countOfCodes = %d ", countCodesD);
        }
        uc c2 = (uc) ch;
        int w2 = 0;
        while (1) {
            fread(&c2, sizeof(uc), 1, file);
            printf(" - reading c2 = %d ", c2);
            if (c2 == 0) {
                break;
            }
            can_list2[w2].symbol = c2;
            //printf("symbol = %d ", can_list2[w2].symbol);
            w2++;
        }
        break;
    }
    for(int e=0; e<countCodesD; e++){
        printf("\n %d - %d %d "B_B_P" ",e, can_list2[e].symbol, can_list2[e].length, B2B(can_list2[e].encoding));
    }
    printf("\n");

    listOfCodesD = (carCode *) malloc(sizeof(carCode));
    carCode *t3;
    t3 = listOfCodesD;
    int c_end;
    int ww=0;
    ch = fgetc(file);
    while (EOF != ch) {
        uc c3 = (uc) ch;
        printf("\n %d reading c3 "B_B_P" ",ww, B2B(c3));
        t3->encoding=c3;
        t3->next=(carCode *) malloc(sizeof(carCode));
        countCharsD++;
        t3=t3->next;
        ch = fgetc(file);
        c_end = ch;
        ww++;
    }
    if(c_end==EOF){
        t3=NULL;
    }
    fclose(file);
    /*carCode *t9;
    t9 = listOfCodesD;
    printf("\n printing list of codes: ");
    int yyy=0;
    do{
        printf(" %d "B_B_P" ",yyy, B2B(t9->encoding));
        t9=t9->next;
        yyy++;
    }while(t9->next!=NULL);*/
}

void decoding() {
    printf("\nerrore da qui %3d %2x ("B_B_P")", can_list2[0].symbol, can_list2[0].length, B2B(can_list2[0].encoding));
    for (int i = 1; i < countCodesD; i++) {
        int true = 1;
        can_list2[i].encoding = can_list2[i - 1].encoding;

        while (true == 1) {
            for (int e = 0; e < 8; e++) {
                if (bitStatus(&can_list2[i].encoding, e) == 0) {
                    toggleBit(&can_list2[i].encoding, e);
                    true = 0;
                    e = 8;
                } else {
                    toggleBit(&can_list2[i].encoding, e);
                }
            }
        }
        printf("\n%3d %2x ("B_B_P")", can_list2[i].symbol, can_list2[i].length, B2B(can_list2[i].encoding));
    }
}

car *listOfCarsD = NULL;
car *t6 = NULL;

void decode_listOfCodesAndSaveToFile(char *c_outAfterDeComp) {
    printf("\n count of chars %d ", countCharsD);
    printf("count of codes %d\n ", countCodesD);
    FILE *file2 = fopen(c_outAfterDeComp, "w");
    listOfCarsD = NULL;
    t6=(car *) malloc(sizeof(car));
    listOfCarsD=t6;
    carCode *r2;
    r2= listOfCodesD;
    //printf("list of codes: ");
    while (r2->next != NULL) {
        printf("\n "B_B_P" ", B2B(r2->encoding));
        for (int t = 0; t < countCodesD; t++) {
            int l1, l2, l3, l4, l5, l6, l7, l8;
            int k1, k2, k3, k4, k5, k6, k7, k8;
            l1 = bitStatus(&r2->encoding, 0);
            k1 = bitStatus(&can_list2[t].encoding, 0);
            l2 = bitStatus(&r2->encoding, 1);
            k2 = bitStatus(&can_list2[t].encoding, 1);
            l3 = bitStatus(&r2->encoding, 2);
            k3 = bitStatus(&can_list2[t].encoding, 2);
            l4 = bitStatus(&r2->encoding, 3);
            k4 = bitStatus(&can_list2[t].encoding, 3);
            l5 = bitStatus(&r2->encoding, 4);
            k5 = bitStatus(&can_list2[t].encoding, 4);
            l6 = bitStatus(&r2->encoding, 5);
            k6 = bitStatus(&can_list2[t].encoding, 5);
            l7 = bitStatus(&r2->encoding, 6);
            k7 = bitStatus(&can_list2[t].encoding, 6);
            l8 = bitStatus(&r2->encoding, 7);
            k8 = bitStatus(&can_list2[t].encoding, 7);

            // converts codes to chars
            if (l1 == k1 && l2 == k2 && l3 == k3 && l4 == k4 && l5 == k5 && l6 == k6 && l7 == k7 && l8 == k8) {
                while (t6->next != NULL) {
                    t6 = t6->next;
                }
                t6->symbol = can_list2[t].symbol;
                printf(" %d ", t6->symbol);
                t6->next = NULL;
            }
        }
        r2 = r2->next;
        fwrite(&t6->symbol, sizeof(uc), 1, file2);
        //printf(" | %d", t6->symbol);
    }
    printf("\nFile e' stato decompresso");
    fclose(file2);
}

void deComp(char *c_in2, char *c_outAfterDeComp) {

    //open the file and read 1) arrayOfCodes[Capacity] + countCharsD number and 2) ff map[ASCII]
    read_file_binario(c_in2);
    //convert the map to canonic Codes
    decoding();
    //convert the codes to the listOfChars using listOfCodes and save to file
    decode_listOfCodesAndSaveToFile(c_outAfterDeComp);
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

        } else {
            fprintf(stderr,
                    "\nLe opzioni sono: -c per comprimere e -d per decomprimere. Si prega di specificare l'opzione da eseguire, il file di input ed il file di destinazione.\n");
            exit(1);
        }
    }
    return 0;
}
