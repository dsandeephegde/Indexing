#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

void writeRecord(FILE *fp, char *buf, long rec_off);

char* readRecord(FILE *fp, long rec_off);

int readSize(FILE *fp, long rec_off);

void deletePrimaryKey(int index);

typedef struct {
    int key; /* Record's key */
    long off; /* Record's offset in file */
} index_S;

typedef struct {
    int size; /* Hole's size */
    long off; /* Hole's offset in file */
} avail_S;

index_S  primaryKeys[ 10000 ];
avail_S availList[ 10000 ];
int pkLength;
int availLength;
char *availListOrder;
char *fileName;

void addIndex(index_S indexRecord) {
    int i;
    primaryKeys[pkLength++] = indexRecord;
    for(i = pkLength-2; i >= 0 && primaryKeys[i].key > indexRecord.key ; i--){
        index_S temp = primaryKeys[i];
        primaryKeys[i] = primaryKeys[i+1];
        primaryKeys[i+1] = temp;
    }
}

int binarySearch(int i, int j, int key) {
    int mid = (i+j)/2;
    if(i>j){
        return -1;
    }
    if(primaryKeys[mid].key == key){
        return mid;
    }
    else if(primaryKeys[mid].key > key){
        return binarySearch(i, mid-1, key);
    }else if(primaryKeys[mid].key < key){
        return binarySearch(mid+1, j, key);
    }
    return -1;
}

void sortPosition(int index) {
    int i;
    avail_S record = availList[index];
    if(strcmp(availListOrder, "--best-fit") == 0){
        for(i = index - 1; i >= 0 && availList[i].size > (record).size ; i--){
            avail_S temp = availList[i];
            availList[i] = availList[i+1];
            availList[i+1] = temp;
        }
    } else if(strcmp(availListOrder, "--worst-fit") == 0) {
        for(i = index - 1; i >= 0 && availList[i].size < (record).size ; i--){
            avail_S temp = availList[i];
            availList[i] = availList[i+1];
            availList[i+1] = temp;
        }
    } else{
        return;
    }
}

void addAvailable(int size, long off) {
    avail_S record;
    record.off = off;
    record.size = size;
    availList[availLength++] = record;
    sortPosition(availLength - 1);
}

void deleteRecord(int key) {
    int deleteKeyIndex = binarySearch(0, pkLength-1, key);
    if(deleteKeyIndex == -1){
        printf("No record with SID=%d exists\n", key);
    } else {
        index_S deletionPK = primaryKeys[deleteKeyIndex];
        FILE *fp = fopen(fileName, "r+b");
        int size = readSize(fp, deletionPK.off) + sizeof(int);
        addAvailable(size, deletionPK.off);
        deletePrimaryKey(deleteKeyIndex);
    }
}

void deletePrimaryKey(int index) {
    int i;
    for ( i = index ; i < pkLength - 1 ; i++ )
        primaryKeys[i] = primaryKeys[i+1];
    pkLength--;
}

void deleteAvailable(int index) {
    int i;
    for ( i = index ; i < availLength - 1 ; i++ )
        availList[i] = availList[i+1];
    availLength--;
}

long getEndOffset(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long rec_off = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return rec_off;
}

long getAvailableOffset(size_t sizeOfRecord, FILE *fp) {
    int i;
    for(i=0;i<availLength; i++){
        if(availList[i].size >= sizeOfRecord){
            long offset = availList[i].off;
            if(availList[i].size == sizeOfRecord){
                deleteAvailable(i);
            } else{
                long off = availList[i].off + sizeOfRecord;
                int size = availList[i].size - sizeOfRecord;
                deleteAvailable(i);
                addAvailable(size, off);
            }
            return offset;
        }
    }
    return getEndOffset(fp);
}

int getNumberOfElements(FILE *out)
{
    fseek( out, 0, SEEK_END );
    int length = ftell(out) / sizeof(index_S);
    fseek( out, 0, SEEK_SET );
    return length;
}

int getHoleSize() {
    int holeSize = 0,i;
    for (i = 0; i < availLength; ++i) {
        holeSize += availList[i].size;
    }
    return holeSize;
}

int main(int argc, char *argv[]) {
    FILE *fp; /* Input/output stream */
    long rec_off; /* Record offset */
    int rec_size; /* Record size, in characters */
    FILE *availFile, *indexFile; /* Output file stream */
    int i;

    if(argc != 3){
        printf("Enter Correct number of arguments, Usage: assn_2 --best-fit student.db");
        exit(0);
    }

    availListOrder = argv[1];
    fileName = argv[2];

    char *indexFileName = "index.bin";
    char *availFileName = "avail.bin";
    if ((fp = fopen(fileName, "r+b")) == NULL ) {
        fp = fopen(fileName, "w+b");
        availFile = fopen(availFileName, "w+b" );
        indexFile = fopen(indexFileName, "w+b" );
    } else{
        if ((availFile = fopen(indexFileName, "r+b" )) == NULL ) {
            availFile = fopen(indexFileName, "w+b" );
        }
        pkLength = getNumberOfElements(availFile);
        fread(primaryKeys, sizeof( index_S ), pkLength, availFile);
        fclose(availFile);

        if ((indexFile = fopen(availFileName, "r+b" )) == NULL ) {
            indexFile = fopen(availFileName, "w+b" );
        }
        availLength = getNumberOfElements(indexFile);
        fread(availList, sizeof( avail_S ), availLength, indexFile);
        fclose(indexFile);
    }
    fclose(fp);

    char command[4096];
    char *key;
    char *value;
    gets(command);
    while(strcmp(command, "end")){
        if(strncmp("add", command, 3) == 0){
            strtok(command, " ");
            key = strtok(NULL, " ");
            value = strtok(NULL, " ");

            if(binarySearch(0, pkLength-1, atol(key)) != -1){
                printf("Record with SID=%s exists\n", key);
            }else{
                fp = fopen(fileName, "r+b");
                size_t sizeOfRecord = strlen(value) + sizeof(int);
                rec_off = getAvailableOffset(sizeOfRecord, fp);
                writeRecord(fp, value, rec_off);

                index_S indexRecord;
                indexRecord.key = atoi(key);
                indexRecord.off = rec_off;
                addIndex(indexRecord);
                fclose(fp);
            }
        } else if(strncmp("del", command, 3) == 0){
            strtok(command, " ");
            key = strtok(NULL, " ");
            deleteRecord(atoi(key));
        } else if(strncmp("find", command, 4) == 0){
            strtok(command, " ");
            key = strtok(NULL, " ");
            int keyIndex = binarySearch(0, pkLength-1, atoi(key));
            if(keyIndex == -1){
                printf("No record with SID=%s exists\n", key);
            }else{
                index_S foundPK = primaryKeys[keyIndex];
                fp = fopen(fileName, "r+b");
                char *res = readRecord(fp, foundPK.off);
                printf("%s\n", res);
                fclose(fp);
            }
        }
        gets(command);
    }

    printf("Index:\n");
    for(i=0;i<pkLength;i++){
        printf( "key=%d: offset=%ld\n", primaryKeys[i].key, primaryKeys[i].off );
    }
    printf("Availability:\n");
    for(i=0;i<availLength;i++){
        printf( "size=%d: offset=%ld\n", availList[i].size, availList[i].off );
    }
    printf( "Number of holes: %d\n", availLength );
    printf( "Hole space: %d\n", getHoleSize() );

    availFile = fopen(indexFileName, "wb" );
    fwrite( primaryKeys, sizeof( index_S ), pkLength, availFile );
    fclose( availFile );
    indexFile = fopen(availFileName, "wb" );
    fwrite( availList, sizeof( avail_S ), availLength, indexFile );
    fclose( indexFile );
}

char* readRecord(FILE *fp, long rec_off) {
    char *buf;
    int rec_size;
    fseek( fp, rec_off, SEEK_SET );
    fread( &rec_size, sizeof( int ), 1, fp );
    buf = malloc(rec_size+1);
    fread( buf, 1, rec_size, fp );
    buf[rec_size] = '\0';
    return buf;
}

int readSize(FILE *fp, long rec_off) {
    int rec_size;
    fseek( fp, rec_off, SEEK_SET );
    fread( &rec_size, sizeof( int ), 1, fp );
    return rec_size;
}

void writeRecord(FILE *fp, char *buf, long rec_off) {
    int rec_size; /* Record size, in characters */

    rec_size = (int) strlen(buf);
    fseek( fp, rec_off, SEEK_SET );

    fwrite(&rec_size, sizeof( int ), 1, fp);
    fwrite(buf, sizeof( char ), rec_size, fp);
}