#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hash_file.h"
#include "hash_table.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HT_ERROR;        \
  }                         \
}

int open_files_counter = 0;
HT_info open_files[MAX_OPEN_FILES];
int *hash_table;

HT_ErrorCode HT_Init() {
  //insert code here

  // Αρχικά δεσμεύουμε χώρο για την εισαγωγή ενός item στο Hash Table
  hash_table = (int*)malloc(2*sizeof(int));
  hash_table[0] = -1;
  hash_table[1] = -1;
  
  return HT_OK;
}

HT_ErrorCode HT_CreateIndex(const char *filename) {
  //insert code here

  // Δημιουργία και άνοιγμα άδειου αρχείου fileName
  CALL_BF(BF_CreateFile(filename));
  int file_desc;
  CALL_BF(BF_OpenFile(filename, &file_desc));
  // Φτιάχνουμε και προσθέτουμε το header block με τα μεταδεδομένα του αρχείου
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_AllocateBlock(file_desc, block));
  void* data;
  data = BF_Block_GetData(block);
  HT_info* header = data;
  header->total_depth = 1;
  header->file_desc = file_desc;
  header->total_rec = 0;
  header->last_block = block;
  header->size_of_hash_table = 0;

  // Κλείσιμο αρχείου fileName και αποδέσμευση μνήμης
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  CALL_BF(BF_CloseFile(file_desc));
  BF_Close();
  return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  //insert code here


  // Ανοίγουμε το αρχείο
  BF_Init(LRU);
  int file_desc;
  CALL_BF(BF_OpenFile(fileName, &file_desc));

  // Παίρνουμε το πρώτο block και τα μεταδεδομένα του αρχείου
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(file_desc, 0, block));
  void* data;
  data = BF_Block_GetData(block); 
  HT_info* header = data;

  // Ενημερώνουμε το file descriptor
  header->file_desc = file_desc;

  // Βάζουμε τη κεφαλίδα του αρχείου στον πίνακα με τα ανοικτά αρχεία
  *indexDesc = open_files_counter;
  open_files_counter++;
  open_files[*indexDesc] = *header;

  // Έλεγχος για το αν έχει recs το αρχείο και τότε 
  // κατάλληλη αρχικοποίηση των μεταδεδομένων του αρχείου 
  int blocks_Num;
  BF_GetBlockCounter(file_desc, &blocks_Num);
  if (blocks_Num > 1) {
    CALL_BF(BF_GetBlock(file_desc, blocks_Num - 1, block));

    // Βάζουμε το τελευταίο block
    data = BF_Block_GetData(block);
    HT_block_info* block_header = data + BF_BLOCK_SIZE - sizeof(block_header);
    header->last_block = block;

    // Υπολογίζουμε τα total recs
    int total_blocks_with_recs = (blocks_Num - 1);
    int recs_per_block = ((BF_BLOCK_SIZE - sizeof(block_header))/sizeof(Record));
    int recs_of_last_block = block_header->num_of_rec;
    header->total_rec = (total_blocks_with_recs-1) * recs_per_block + recs_of_last_block;
  }

  // Αν δεν υπάρχουν εγγραφές μπορούμε να αποδεσμεύσουμε το block
  if (header->total_rec == 0) {
    BF_Block_Destroy(&block);
  }

  return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  //insert code here

  // Παίρνουμε τα μεταδεδομένα του αρχείου
  HT_info* header_info = &open_files[indexDesc];
  int file_desc = header_info->file_desc;

  // Κάνε unpin το πρώτο και το τελευταίο block
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(file_desc, 0, block));
  CALL_BF(BF_UnpinBlock(block));

  BF_Block *last_block = header_info->last_block;
  void* data = BF_Block_GetData(last_block);
  CALL_BF(BF_UnpinBlock(last_block));
  BF_Block_Destroy(&last_block);

  // Κλείσε το αρχείο
  BF_Block_Destroy(&block);
  CALL_BF(BF_CloseFile(file_desc));

  return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here

  // Παίρνουμε τα μεταδεδομένα του αρχείου
  HT_info* header_info = &open_files[indexDesc];
  int file_desc = header_info->file_desc;

  // Παίρνουμε το block με τα μεταδεδομένα του αρχείου
  // ώστε να γίνει dirty αργότερα
  BF_Block *header_block;
  BF_Block_Init(&header_block);
  CALL_BF(BF_GetBlock(file_desc, 0, header_block));
  void* data;

  // Start the insert!

  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}

