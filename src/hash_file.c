#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
  header->global_depth = 1;
  header->file_desc = file_desc;
  header->total_rec = 0;
  header->last_block = block;
  header->size_of_hash_table = 2;

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

  // Αρχίζει η διαδικασία του insert
  if (header_info->total_rec == 0) {
    
    // Αρχικοποίσε ένα block και βάλε το ως last_block
    BF_Block *block;
    BF_Block_Init(&block);
    header_info->last_block = block;
    CALL_BF(BF_AllocateBlock(file_desc, block));

    // Αρχικοποίησε και βάλε το header που αποθηκεύει το info του 
    // block στο τέλος του block
    data = BF_Block_GetData(block); 
    HT_block_info* block_header = data;
    int offset = BF_BLOCK_SIZE - sizeof(block_header);
    memcpy(data + offset, block_header, sizeof(block_header));
    block_header = data + offset;
    block_header->num_of_rec = 1;
    block_header->local_depth = 1;
    block_header->capacity = BF_BLOCK_SIZE - sizeof(record) - sizeof(block_header);

    // Βάλε το record στην αρχή του block
    Record* rec = data;
    rec[0] = record;
    header_info->total_rec += 1;

    // Κάνε update το Hash Table
    int hash_value = hash_function(record.id, header_info->global_depth);
    int block_num;
    CALL_BF(BF_GetBlockCounter(file_desc, &block_num));
    block_num--;
    hash_table[hash_value] = block_num;
    printf("record.id = %d | global_depth = %d\n", record.id, header_info->global_depth);
    printf("hash_value = %d | block_num = %d\n",hash_value, block_num);

    // Κάνε το block Dirty
    BF_Block_SetDirty(block);

    // Κάνε το header block του αρχείου Dirty και Destroy
    BF_Block_SetDirty(header_block);
    BF_Block_Destroy(&header_block);
  }
  else {

    int hash_value = hash_function(record.id, header_info->global_depth);
    int block_number = hash_table[hash_value];
    if (block_number == -1) {
      // Αρχικοποίσε ένα block και βάλε το ως last_block
      BF_Block *block;
      BF_Block_Init(&block);
      header_info->last_block = block;
      CALL_BF(BF_AllocateBlock(file_desc, block));

      // Αρχικοποίησε και βάλε το header που αποθηκεύει το info του 
      // block στο τέλος του block
      data = BF_Block_GetData(block);
      HT_block_info* block_header = data;
      int offset = BF_BLOCK_SIZE - sizeof(block_header);
      memcpy(data + offset, block_header, sizeof(block_header));
      block_header = data + offset;
      block_header->num_of_rec = 1;
      block_header->local_depth = 1;
      block_header->capacity = BF_BLOCK_SIZE - sizeof(record) - sizeof(block_header);

      // Βάλε το record στην αρχή του block
      Record* rec = data;
      rec[0] = record;
      header_info->total_rec += 1;

      // Κάνε update το Hash Table
      char bin_string[33];
      dec2bin_string(record.id, bin_string);
      char first_bit = bin_string[0];
      int pos_amount = pow(2, header_info->global_depth) / 2;
      char* first_hash_value_string = (char*)malloc((header_info->global_depth)*sizeof(char));
      first_hash_value_string[0] = first_bit;
      for (int i = 1 ; i < header_info->global_depth ; i++) {
          first_hash_value_string[i] = '0';
      }
      int first_hash_value = bin_string2dec(first_hash_value_string);
      free(first_hash_value_string);
      int block_num;
      CALL_BF(BF_GetBlockCounter(file_desc, &block_num));
      block_num--;
      while (pos_amount > 0) {
        pos_amount--;
        hash_table[first_hash_value] = block_num;
        first_hash_value++;
      }


      // Κάνε το block Dirty
      BF_Block_SetDirty(block);

      // Κάνε το header block του αρχείου Dirty και Destroy
      BF_Block_SetDirty(header_block);
      BF_Block_Destroy(&header_block);
    }
    else {

      // Φόρτωσε το block με το block_number και τα μεταδεδομένα του
      BF_Block *block = header_info->last_block;
      BF_GetBlock(file_desc, block_number, block);
      data = BF_Block_GetData(block);
      HT_block_info* block_header = data + BF_BLOCK_SIZE - sizeof(block_header);
      int record_size = sizeof(record);

      // Τσέκαρε αν χωράει νέα εγγραφή στο block και αν ναι βάλε την,
      // κάνοντας επίσης τις απαραίτητες ενημερώσεις στα μεταδεδομένα 
      // του αρχείου και του block
      if (block_header->capacity - record_size >= 0) {

        // Βάλε το record στην θέση που πρέπει και ενημέρωσε τα μεταδεδομένα
        int offset = record_size*(block_header->num_of_rec);
        memcpy(data + offset, &record, record_size);
        block_header->num_of_rec++;
        block_header->capacity -= record_size;
        header_info->total_rec++;

        // Κάνε το block Dirty
        BF_Block_SetDirty(block);

        // Κάνε το header block του αρχείου Dirty και Destroy
        BF_Block_SetDirty(header_block);
        BF_Block_Destroy(&header_block);
      }
      else {

        if (block_header->local_depth < header_info->global_depth) {

          // Πάρε το παλιό block
          BF_Block *old_block = block;
          HT_block_info* old_block_header = block_header;

          // Αρχικοποίσε ένα block ως new_block και βάλε το ως last_block
          BF_Block *new_block;
          BF_Block_Init(&new_block);
          header_info->last_block = new_block;
          CALL_BF(BF_AllocateBlock(file_desc, new_block));

          // Αρχικοποίησε και βάλε το header που αποθηκεύει το info του 
          // new_block στο τέλος του new_block
          data = BF_Block_GetData(new_block); 
          HT_block_info* new_block_header = data;
          new_block_header->num_of_rec = 0;
          new_block_header->local_depth = old_block_header->local_depth;
          new_block_header->capacity = BF_BLOCK_SIZE - sizeof(new_block_header);
          int offset = BF_BLOCK_SIZE - sizeof(new_block_header);
          memcpy(data + offset, new_block_header, sizeof(new_block_header));

          // Αύξησε το local_depth στο παλιό και το νέο block
          old_block_header->local_depth++;
          new_block_header->local_depth++;
          int count = 0, half_bl = (int)pow(2, header_info->global_depth - old_block_header->local_depth - 1);
          half_bl = half_bl / 2;
          int new_block_num;
          int old_block_num = block_number;
          for (int i = 0; i < header_info->size_of_hash_table; i++) {
            if (hash_table[i] == block_number) {
              if (count < half_bl) {
                count++;
              }
              else {
                if (count == half_bl) break;
                CALL_BF(BF_GetBlockCounter(file_desc, &new_block_num));
                hash_table[i] = new_block_num;
                count++;
              }
            }
          }
          // Βρες ποια record ήταν στο παλιό block και δες αν πρέπει 
          // να μετακινηθούν για να πάνε στο καινούργιο
          void* old_block_data = BF_Block_GetData(old_block);
          void* new_block_data = BF_Block_GetData(new_block);
          Record temp_rec;
          for (int i = 0 ; i < old_block_header->num_of_rec ; i++) {

            // Βάλε στο temp_rec το record που εξετάζουμε σε αυτό το loop
            // και βρες το hash_value του
            int offset = i*record_size;
            memcpy(&temp_rec, old_block_data + offset, record_size);
            int temp_hash_value = hash_function(temp_rec.id, header_info->global_depth);

            // Αν το συγκεκριμένο record πλέον πρέπει να αλλάξει block,
            // γιατί η hash_function δίνει νέο hash_value
            if (hash_table[temp_hash_value] != old_block_num) {

              // Αφαίρεσε το από το παλιό block
              old_block_header->capacity += record_size;
              old_block_header->num_of_rec--;

              // Πρόσθεσε το στο καινούργιο block
              memcpy(new_block_data + offset, &temp_rec, record_size);
              new_block_header->capacity -= record_size;
              new_block_header->num_of_rec++;
            }
          }
          
          // Βρίσκουμε σε ποιο από τα δύο block πρέπει να πάει το νέο record 
          // για το οποίο έγινε το split και το βάζουμε
          hash_value = hash_function(record.id, header_info->global_depth);
          block_number = hash_table[hash_value];
          BF_GetBlock(file_desc, block_number, block);
          data = BF_Block_GetData(block);
          block_header = data + BF_BLOCK_SIZE - sizeof(block_header);
          offset = record_size*(block_header->num_of_rec);
          memcpy(new_block_data + offset, &temp_rec, record_size);
          block_header->num_of_rec++;        
          block_header->capacity -= record_size;
          header_info->total_rec++;
          header_info->last_block = block;
          


          // Κάνε τα block Dirty
          BF_Block_SetDirty(old_block);
          BF_Block_SetDirty(new_block);

          // Κάνε το header block του αρχείου Dirty και Destroy
          BF_Block_SetDirty(header_block);
          BF_Block_Destroy(&header_block);
        }
        else {
          // Άρχισε την διαδικασία του split
          // Διπλασίασε το Hash Table
          header_info->global_depth++;
          double_hash(&hash_table, header_info->size_of_hash_table);
          header_info->size_of_hash_table = 2 * (header_info->size_of_hash_table);

          // Πάρε το παλιό block
          BF_Block *old_block = block;
          HT_block_info* old_block_header = block_header;

          // Αρχικοποίσε ένα block ως new_block και βάλε το ως last_block
          BF_Block *new_block;
          BF_Block_Init(&new_block);
          header_info->last_block = new_block;
          CALL_BF(BF_AllocateBlock(file_desc, new_block));

          // Αρχικοποίησε και βάλε το header που αποθηκεύει το info του 
          // new_block στο τέλος του new_block
          data = BF_Block_GetData(new_block); 
          HT_block_info* new_block_header = data;
          new_block_header->num_of_rec = 0;
          new_block_header->local_depth = old_block_header->local_depth;
          new_block_header->capacity = BF_BLOCK_SIZE - sizeof(new_block_header);
          int offset = BF_BLOCK_SIZE - sizeof(new_block_header);
          memcpy(data + offset, new_block_header, sizeof(new_block_header));

          // Αύξησε το local_depth στο παλιό και το νέο block
          old_block_header->local_depth++;
          new_block_header->local_depth++;

          // Ενημέρωσε τα positions μετά το split
          int old_pos = hash_value;
          int new_pos = 2*old_pos;
          int old_block_num = block_number;
          int new_block_num;
          CALL_BF(BF_GetBlockCounter(file_desc, &new_block_num));
          new_block_num--;
          hash_table[new_pos] = old_block_num;
          hash_table[new_pos+1] = new_block_num;


          // Βρες ποια record ήταν στο παλιό block και δες αν πρέπει 
          // να μετακινηθούν για να πάνε στο καινούργιο
          void* old_block_data = BF_Block_GetData(old_block);
          void* new_block_data = BF_Block_GetData(new_block);
          Record temp_rec;
          for (int i = 0 ; i < old_block_header->num_of_rec ; i++) {

            // Βάλε στο temp_rec το record που εξετάζουμε σε αυτό το loop
            // και βρες το hash_value του
            int offset = i*record_size;
            memcpy(&temp_rec, old_block_data + offset, record_size);
            int temp_hash_value = hash_function(temp_rec.id, header_info->global_depth);

            // Αν το συγκεκριμένο record πλέον πρέπει να αλλάξει block,
            // γιατί η hash_function δίνει νέο hash_value
            if (hash_table[temp_hash_value] != old_block_num) {

              // Αφαίρεσε το από το παλιό block
              old_block_header->capacity += record_size;
              old_block_header->num_of_rec--;

              // Πρόσθεσε το στο καινούργιο block
              memcpy(new_block_data + offset, &temp_rec, record_size);
              new_block_header->capacity -= record_size;
              new_block_header->num_of_rec++;
            }
          }

          // Βρίσκουμε σε ποιο από τα δύο block πρέπει να πάει το νέο record 
          // για το οποίο έγινε το split και το βάζουμε
          hash_value = hash_function(record.id, header_info->global_depth);
          block_number = hash_table[hash_value];
          BF_GetBlock(file_desc, block_number, block);
          data = BF_Block_GetData(block);
          block_header = data + BF_BLOCK_SIZE - sizeof(block_header);
          offset = record_size*(block_header->num_of_rec);
          memcpy(new_block_data + offset, &temp_rec, record_size);
          block_header->num_of_rec++;
          block_header->capacity -= record_size;
          header_info->total_rec++;
          header_info->last_block = block;
          
          // Κάνε τα block Dirty
          BF_Block_SetDirty(old_block);
          BF_Block_SetDirty(new_block);

          // Κάνε το header block του αρχείου Dirty και Destroy
          BF_Block_SetDirty(header_block);
          BF_Block_Destroy(&header_block);
        }
      }
    }
  }

  return HT_OK;
}

void printRecord(Record record){
    printf("(%d,%s,%s,%s)\n",record.id,record.name,record.surname,record.city);
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here

  // Παίρνουμε τα μεταδεδομένα του αρχείου
  HT_info* header_info = &open_files[indexDesc];
  int file_desc = header_info->file_desc;

  BF_Block *block;
  BF_Block_Init(&block);
  if (id != NULL) {
    int hash_value = hash_function(*id, header_info->global_depth);
    int block_number = hash_table[hash_value];
    CALL_BF(BF_GetBlock(file_desc, block_number, block));
    void* data = BF_Block_GetData(block); 
    HT_block_info* block_header = data + BF_BLOCK_SIZE - sizeof(block_header);
    Record temp_rec;
    for (int i = 0 ; i < block_header->num_of_rec ; i++) {
      int offset = i*sizeof(Record);
      memcpy(&temp_rec, data + offset, sizeof(Record));
      if (*id == temp_rec.id) {
        printRecord(temp_rec);
      }
    }
  }
  else {
    int blocks_num;
    CALL_BF(BF_GetBlockCounter(file_desc, &blocks_num));
    for (int i = 1 ; i < blocks_num ; i++) {
      CALL_BF(BF_GetBlock(file_desc, i, block));
      void* data = BF_Block_GetData(block); 
      HT_block_info* block_header = data + BF_BLOCK_SIZE - sizeof(block_header);
      Record temp_rec;
      for (int j = 0 ; j < block_header->num_of_rec ; j++) {
        int offset = j*sizeof(Record);
        memcpy(&temp_rec, data + offset, sizeof(Record));
        printf("Block %d :", i);
        printRecord(temp_rec);
      }
    }
  }
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  return HT_OK;
}