#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hash_file.h"

#define RECORDS_NUM 1000 // you can change it if you want
#define GLOBAL_DEPT 2 // you can change it if you want
#define FILE_NAME "data.db"

const char* names[] = {
  "Yannis",
  "Christofos",
  "Sofia",
  "Marianna",
  "Vagelis",
  "Maria",
  "Iosif",
  "Dionisis",
  "Konstantina",
  "Theofilos",
  "Giorgos",
  "Dimitris"
};

const char* surnames[] = {
  "Ioannidis",
  "Svingos",
  "Karvounari",
  "Rezkalla",
  "Nikolopoulos",
  "Berreta",
  "Koronis",
  "Gaitanis",
  "Oikonomou",
  "Mailis",
  "Michas",
  "Halatsis"
};

const char* cities[] = {
  "Athens",
  "San Francisco",
  "Los Angeles",
  "Amsterdam",
  "London",
  "New York",
  "Tokyo",
  "Hong Kong",
  "Munich",
  "Miami"
};

#define CALL_OR_DIE(call)     \
  {                           \
    HT_ErrorCode code = call; \
    if (code != HT_OK) {      \
      printf("Error\n");      \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU);
  
  CALL_OR_DIE(HT_Init());

  int indexDesc;
  CALL_OR_DIE(HT_CreateIndex(FILE_NAME));
  CALL_OR_DIE(HT_OpenIndex(FILE_NAME, &indexDesc)); 

  Record record;
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    // create a record
    record.id = id;
    r = rand() % 12;
    memcpy(record.name, names[r], strlen(names[r]) + 1);
    r = rand() % 12;
    memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
    r = rand() % 10;
    memcpy(record.city, cities[r], strlen(cities[r]) + 1);

    CALL_OR_DIE(HT_InsertEntry(indexDesc, record));
  }

  printf("RUN PrintAllEntries\n");
  printf("*************************************************************\n");


  printf("\n\n\n\n");
  printf("TEST 1\n");
  printf("*************************************************************\n");
  int dec=521;
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, NULL));
  printf("\n");
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, &dec));
  printf("\n");
  CALL_OR_DIE(HashStatistics(indexDesc));
  CALL_OR_DIE(HT_CloseFile(indexDesc));
  printf("*************************************************************\n");


  printf("\n\n\n\n");
  printf("TEST 2\n");
  printf("*************************************************************\n");
  CALL_OR_DIE(HT_OpenIndex(FILE_NAME, &indexDesc));
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, NULL));
  printf("\n");
  dec=283;
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, &dec));
  printf("\n");
  CALL_OR_DIE(HashStatistics(indexDesc));
  CALL_OR_DIE(HT_CloseFile(indexDesc));
  printf("*************************************************************\n");


  printf("\n\n\n\n");
  printf("TEST 3\n");
  printf("*************************************************************\n");
  CALL_OR_DIE(HT_OpenIndex(FILE_NAME, &indexDesc));
  record.id = 683;
  r = rand() % 12;
  memcpy(record.name, names[r], strlen(names[r]) + 1);
  r = rand() % 12;
  memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
  r = rand() % 10;
  memcpy(record.city, cities[r], strlen(cities[r]) + 1);
  CALL_OR_DIE(HT_InsertEntry(indexDesc, record));
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, &record.id));
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, NULL));
  CALL_OR_DIE(HT_CloseFile(indexDesc));
  printf("*************************************************************\n");


  printf("\n\n\n\n");
  printf("TEST 4\n");
  printf("*************************************************************\n");
  CALL_OR_DIE(HT_OpenIndex(FILE_NAME, &indexDesc));
  

  for (int id = 1001; id < 2001; ++id) {
    // create a record
    record.id = id;
    r = rand() % 12;
    memcpy(record.name, names[r], strlen(names[r]) + 1);
    r = rand() % 12;
    memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
    r = rand() % 10;
    memcpy(record.city, cities[r], strlen(cities[r]) + 1);

    CALL_OR_DIE(HT_InsertEntry(indexDesc, record));
  }
  
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, NULL));
  printf("\n");
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, &dec));
  printf("\n");
  CALL_OR_DIE(HashStatistics(indexDesc));
  CALL_OR_DIE(HT_CloseFile(indexDesc));
  printf("*************************************************************\n");


  printf("\n\n\n\n");
  printf("TEST 5\n");
  printf("*************************************************************\n");
  CALL_OR_DIE(HT_OpenIndex(FILE_NAME, &indexDesc));
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, NULL));
  printf("\n\n");
  CALL_OR_DIE(HT_CreateIndex("data2.db"));
  int indexDesc2;
  CALL_OR_DIE(HT_OpenIndex("data2.db", &indexDesc2));
  for (int id = 0; id < 50; ++id) {
  // create a record
    record.id = id;
    r = rand() % 12;
    memcpy(record.name, names[r], strlen(names[r]) + 1);
    r = rand() % 12;
    memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
    r = rand() % 10;
    memcpy(record.city, cities[r], strlen(cities[r]) + 1);

    CALL_OR_DIE(HT_InsertEntry(indexDesc2, record));
  }
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc2, NULL));
  printf("\n");
  int p = 42;
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc2, &p));
  printf("\n");
  CALL_OR_DIE(HashStatistics(indexDesc2));
  printf("\n\n\n");

  CALL_OR_DIE(HT_CloseFile(indexDesc2));
  CALL_OR_DIE(HT_CloseFile(indexDesc));
  printf("*************************************************************\n");

  BF_Close(); 
}
