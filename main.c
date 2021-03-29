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
void setBit(uc* byte, uc bit_number); // Set the i-th bit to 1
void clearBit(uc* byte, uc bit_number); // Set the i-th bit to 0
void toggleBit(uc* byte, uc bit_number); // Complement i-th bit
uc bitStatus(uc* byte, uc bit_number); // Get the status of i-th bit, returns 0 or 1
#endif
void setBit(uc* byte, uc bit_number){ // Set the i-th bit to 1
    (*byte) |= 1 << bit_number;
}
void clearBit(uc* byte, uc bit_number){ // Set the i-th bit to 0
    (*byte) &= ~(1 << bit_number);
}
void toggleBit(uc* byte, uc bit_number){ // Complement i-th bit
    (*byte) ^= 1 << bit_number;
}
uc bitStatus(uc* byte, uc bit_number){ // Get the status of i-th bit, returns 0 or 1
    return ((*byte & 1 << bit_number) > 0 ? 1 : 0);
}

typedef unsigned long ul; typedef unsigned char uc;

typedef struct node {
    uc symbol;
    ul frequency;
    struct node* left;
    struct node* right;
} node;

typedef struct car {
    uc symbol;
    struct car* next;
} car;
typedef struct carCode {
    uc encoding;
    struct carCode* next;
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

node* table[ASCII];
enc code_table[ASCII];
enc* final_code_table[ASCII];
final_enc list[ASCII];
uc longest_code=0;
unsigned long long total_bytes=0;
int ch;
int countChars;
int countCodes;
int countCharsD;
int countCodesD;

void deComp(char *c_out, char *c_outAfterDeComp);
void encoding();
void decode_arrayOfCodes();
void decoding();

node* createNode(uc symbol){
    node* tmp=(node*) malloc(sizeof(node));
    tmp->symbol=symbol;
    tmp->frequency=0;
    tmp->left=NULL;
    tmp->right=NULL;
    return tmp;
}
car* listOfCars=NULL;
void addCar(uc symbol){
    if(listOfCars==NULL){
        listOfCars=(car*) malloc(sizeof(car));
        listOfCars->symbol=symbol;
        listOfCars->next=NULL;
    } else{
        car* t1;
        t1=listOfCars;
        while (t1->next!=NULL){
            t1=t1->next;
        }
        car* tmp=(car*) malloc(sizeof(car));
        t1->next=tmp;
        tmp->symbol=symbol;
        tmp->next=NULL;
    }
}
void createTreeTable(){
    for(int i=0; i<256;i++){
        table[i]=createNode(i);
    }
}

void createCodeTable(){
    for(int i=0; i<256;i++){
        code_table[i].codelen=0;
        code_table[i].symbol=0;
        code_table[i].frequency=0;
        final_code_table[i]=NULL;
    }
}
int computeFrequencies(char * filename){
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Problem opening file\n");
        return EXIT_FAILURE;
    }
    printf("Reading file and computing frequencies\n");
    while (EOF != (ch = fgetc(file))){
        if (ferror(file) != 0){
            printf("Problem opening file\n");
            return EXIT_FAILURE;
        }
        uc c = (uc) ch;
        table[c]->frequency++;
        total_bytes++;
        addCar(c);
        countChars++;
    }
    fclose(file);
    printf("File closed\n");
    return EXIT_SUCCESS;
}

void sortTable(){
    printf("Sorting frequencies, bubble sort!\n");
    for (int i=0; i<256; i++){
        for (int j=i+1; j<256; j++){
            if (table[i]->frequency < table[j]->frequency){
                node* tmp = table[j];
                table[j] = table[i];
                table[i] = tmp;
            }
        }
    }
}

void buildTree(){
    printf("Building tree\n");
    for (int i=0; i<255; i++){
        int j = 255-i;
        int k = 255-i-1;
        node* tmp = createNode(0);
        tmp->frequency = table[j]->frequency + table[k]->frequency;
        tmp->left = table[j];
        tmp->right = table[k];
        table[k] = table[j] = NULL;
        k--; // Exploiting Insertion Sort functioning mechanism
        while ((k >= 0) && (table[k]->frequency < tmp->frequency)){
            table[k+1] = table[k];
            k--;
        }
        table[k+1]=tmp;
    }
}

void deleteTree(node* tree){
    if (tree == NULL) return;
    deleteTree(tree->left);
    deleteTree(tree->right);
    free(tree);
}
void printTree(node* tree){
    if (tree == NULL || tree->frequency==0) {
        return;
    } else {
        printf("\n %d %d", tree->symbol, tree->frequency);
    }
    printTree(tree->left);
    printTree(tree->right);
}

void computeCodeLen(node* tree, uc codelen){
    if (tree != NULL){
        if (tree->left == NULL && tree->right==NULL){
            code_table[tree->symbol].codelen = 255;
            if (tree->frequency > 0){
                code_table[tree->symbol].symbol = tree->symbol;
                code_table[tree->symbol].frequency = tree->frequency;
                code_table[tree->symbol].codelen = codelen;
                if (codelen > longest_code)
                    longest_code = codelen;
            }
        }
        computeCodeLen(tree->left, codelen+1);
        computeCodeLen(tree->right, codelen+1);
    }
}
final_enc can_list[ASCII];
void create_final_list_encoding (){
    for(int i=0; i<256;i++){
        list[i].symbol=0;
        list[i].length=0;
    }
    //printf("\nBefore sorting the final list of encoding");
    for (int i=0; i<256; i++){
        if (code_table[i].frequency > 0){
            list[i].symbol=code_table[i].symbol;
            list[i].length=code_table[i].codelen;
            //printf("\n%d %d", list[i].symbol, list[i].length);
        }
    }
    //printf("\nAfter sorting the final list of encoding");
    for (int i=0; i<256; i++){
        for (int j=i+1; j<256; j++){
            if (list[i].length > list[j].length){
                final_enc tmp = list[j];
                list[j] = list[i];
                list[i] = tmp;
            }
        }
    }
    // print the list
    for (int i=0; i<256; i++){
        if (list[i].length > 0){
            //printf("\n%d %d", list[i].symbol, list[i].length);
        }
    }
    int y=0;
    for (int i=0; i<256; i++){
        if (list[i].length > 0){
            can_list[y].symbol=list[i].symbol;
            can_list[y].length=list[i].length;
            y++;
        }
    }
    countCodes=y;
}
void encoding() {
    //printf("\n%3d %2x ("B_B_P")", can_list[0].symbol, can_list[0].length, B2B(can_list[0].encoding));
    int i=0;
    while(can_list[i].length>0){
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
        } i++;
    }
    printf("ok\n");
}
carCode* listOfCodes=NULL;
void addCarCode(){
    carCode* t3;
    t3=listOfCodes;
    while(listOfCars->next!=NULL){
        //printf("\n %d \n", listOfCars->symbol);
        for(int i=0; i<countCodes;i++){
            //printf(" %d ", can_list[i].symbol);
            if(listOfCars->symbol==can_list[i].symbol){
                t3=(carCode*) malloc(sizeof(carCode));
                t3->encoding=can_list[i].encoding;
                printf("("B_B_P") ", B2B(t3->encoding));
                t3->next=NULL;
            }
        } listOfCars=listOfCars->next;
    }
}

void save_to_file_binario(char *filename){
    printf("\n lunghezze: ");
    FILE* file = fopen(filename, "wb+");
    for(int i=0; i<ASCII;i++){
        if(can_list[i].length>0){
            printf(" %d ", can_list[i].length);
            fwrite(&can_list[i].length, sizeof(uc), 1, file);
        }
    }
    fwrite(0, sizeof(uc), 1, file);
    fclose(file);
    printf("\n codici: ");
    FILE* file2 = fopen(filename, "ab+");
    carCode* t7;
    t7=listOfCodes;
    printf(" esempio: ("B_B_P") ", B2B(t7->encoding));
    while (t7->next!=NULL){
        printf(" ("B_B_P") ", B2B(t7->encoding));
        fwrite(&t7->encoding, sizeof(ul), 1, file2);
        t7=t7->next;
    }
    printf("\nFile e' stato compresso");
    fclose(file2);
}


void computeCanonicalEncoding(){
    //1. Sort code_table & save in list[total_bytes]
    create_final_list_encoding();
    //2. Compute canonical encoding for each item in the list
    encoding();
    //3. Encode the message that will be saved to the output file
    addCarCode();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void compress(char * file_name_in, char * file_name_out) {

    createTreeTable();
    createCodeTable();
    computeFrequencies(file_name_in);
    sortTable();
    buildTree();
    computeCodeLen(table[0], 0);
    computeCanonicalEncoding();
    save_to_file_binario(file_name_out);
    deleteTree(table[0]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
carCode* listOfCodesD=NULL;
void addCarCodeD(ul encoding){
    carCode* t3;
    t3=listOfCodesD;
    if(t3==NULL){
        t3=(carCode*)malloc(sizeof(carCode));
        t3->encoding=encoding;
        t3->next=NULL;
    } else{
        t3->next=(carCode*)malloc(sizeof(carCode));
        t3=t3->next;
        t3->encoding=encoding;
        t3->next=NULL;
    }
}

final_enc can_list2[ASCII];
void read_file_binario(char *filename){
    FILE* file = fopen(filename, "rb");
    if(file==NULL){
        printf("\nIl file non esiste");
        exit(1);
    }
    while ((ch = fgetc(file)) !=0){
        if (ferror(file) != 0){
            printf("Problem opening file\n");
            EXIT_FAILURE;
        }
        uc c = (uc) ch;
        while(c!=0){
            for(int w=0;w<ASCII;w++){
                fread(&can_list2[w].length, sizeof(uc),1,file);
                //printf("\n read %d char ", w);
                //printf("%d %d "B_B_P" ", can_list2[w].symbol, can_list2[w].length, B2B(can_list2[w].encoding));
            }
            countCodesD++;
        }
        uc c4;
        fread(&c4, sizeof(uc),1,file);
        printf("\n control 0 = %d \n ", c4);
    }
    while (EOF != (ch = fgetc(file))){
        ul c1=(ul) ch;
        fread(&c1, sizeof(ul),1,file);
        addCarCodeD(c1);
        countCharsD++;
    }
    fclose(file);
}
void decoding() {
    //printf("\n%3d %2x ("B_B_P")", can_list[0].symbol, can_list[0].length, B2B(can_list[0].encoding));
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
        //printf("\n%3d %2x ("B_B_P")", can_list2[i].symbol, can_list2[i].length, B2B(can_list2[i].encoding));
    }

}
car* listOfCarsD=NULL;
car* t6=NULL;

void decode_arrayOfCodes(){
    //printf("count of chars %d", countCharsD);
    //printf("count of codes %d", countCodesD);
    t6=listOfCarsD;
    t6=(car*)malloc(sizeof(car));
    for(int i=0; i<countCharsD; i++){
        for(int t=0; t<countCodesD; t++){
            int l1,l2,l3,l4,l5,l6,l7,l8;
            int k1,k2,k3,k4,k5,k6,k7,k8;
            l1=bitStatus(&listOfCodesD->encoding,0);
            k1=bitStatus(&can_list2[t].encoding,0);
            l2=bitStatus(&listOfCodesD->encoding,1);
            k2=bitStatus(&can_list2[t].encoding,1);
            l3=bitStatus(&listOfCodesD->encoding,2);
            k3=bitStatus(&can_list2[t].encoding,2);
            l4=bitStatus(&listOfCodesD->encoding,3);
            k4=bitStatus(&can_list2[t].encoding,3);
            l5=bitStatus(&listOfCodesD->encoding,4);
            k5=bitStatus(&can_list2[t].encoding,4);
            l6=bitStatus(&listOfCodesD->encoding,5);
            k6=bitStatus(&can_list2[t].encoding,5);
            l7=bitStatus(&listOfCodesD->encoding,6);
            k7=bitStatus(&can_list2[t].encoding,6);
            l8=bitStatus(&listOfCodesD->encoding,7);
            k8=bitStatus(&can_list2[t].encoding,7);

            // converts codes to chars
            if(l1==k1 && l2==k2 && l3==k3 && l4==k4 && l5==k5 && l6==k6 && l7==k7 && l8==k8 ){
                t6->symbol=can_list2[t].symbol;
                //printf("\n"B_B_P" - "B_B_P" ", B2B(arrayOfCodesD[i]), B2B(can_list2[t].encoding));
                //printf(" %d - %c ", arrayOfCharsD[i], arrayOfCharsD[i]);
                t6=t6->next;
            }
        }
    }
}

void save_to_file_txt(char *c_outAfterDeComp){
    FILE* file2 = fopen(c_outAfterDeComp, "w");
    for(int p=0; p<countCharsD; p++){
        fwrite(&listOfCarsD->symbol, sizeof(uc), 1, file2);
    }
    printf("\nFile e' stato decompresso");
    fclose(file2);
}

void deComp(char *c_in2, char *c_outAfterDeComp) {

    //open the file and read 1) arrayOfCodes[Capacity] + countCharsD number and 2) ff map[ASCII]
    read_file_binario(c_in2);
    //convert the map to canonic Codes
    decoding();
    //convert the codes to the arrayOfChars using arrayOfCodes
    decode_arrayOfCodes();
    //save to the output txt file the arrayOfChars
    save_to_file_txt(c_outAfterDeComp);
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
