#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "hash_table.h"
#include "bf.h"
#include "hash_file.h"

#define OFFSET_BASIS 2166136261ul
#define FNV_PRIME 16777619ul

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HT_ERROR;        \
  }                         \
}

uint32_t hash(char* data, size_t bytes)
{

   uint32_t h = OFFSET_BASIS;

   for (size_t i = 0; i < bytes; ++i)
   {
      h = (h ^ data[i]) * FNV_PRIME;
   }

   return h;
}

void print_HashTable(int* hash_table, int hash_array_size) {
    for (int i = 0 ; i < hash_array_size ; i++) {
        printf("hash_table[%d] = %d\n", i, hash_table[i]);
    }
}

void double_hash(void* header_inf) {

    HT_info* header_info = header_inf;
    int hash_array_size = header_info->size_of_hash_table;
    int **hash_array = &(header_info->hash_table);

    // Αντέγραψε το αρχικό hash_array
    int* temp_hash_array = (int*)malloc(hash_array_size*sizeof(int));
    for (int i = 0 ; i < hash_array_size ; i++) {
        temp_hash_array[i] = (*hash_array)[i];
    }
    
    // Έλεγξε αν υπάρχει διαθέσιμος χώρος στο block του Hash Table, στο αρχείο HT
    // Αν υπάρχει απλά συνέχισε από κάτω
    // Αλλιώς όσα ακόμα block χρειάζονται
    int HT_blocks = header_info->count_blocks_for_HT;
    int cur_hash_table_sz = (header_info->size_of_hash_table)*sizeof(int);
    float blocks_needed_for_HT = (float)(2*cur_hash_table_sz) / (float)BF_BLOCK_SIZE;
    int bl_nd = (int)blocks_needed_for_HT;
    if (blocks_needed_for_HT > (float)bl_nd) {
        bl_nd++;
    }
    if (HT_blocks < bl_nd) {
        int more_blocks_needed = bl_nd - HT_blocks;
        BF_Block* alloc_block;
        BF_Block_Init(&alloc_block);
        for (int i = 0 ; i < more_blocks_needed ; i++) {
            BF_AllocateBlock(header_info->file_desc, alloc_block);
            BF_Block_SetDirty(alloc_block);
            BF_UnpinBlock(alloc_block);
        }
        BF_Block_Destroy(&alloc_block);
    }

    // Διπλασίασε το αρχικό array
    *hash_array = realloc(*hash_array, 2*hash_array_size*sizeof(int));
    // Διόρθωσε τις καινούργιες θέσεις να δείχνουν στα σωστά block μετά το διπλασιασμό
    // Αν ο αριθμός block είναι -1 σημαίνει ότι δεν δείχνει σε block
    for (int i = 0 ; i < hash_array_size ; i++) {
        int old_pos = i;
        int new_pos = 2*old_pos;
        if (temp_hash_array[old_pos] != -1) {
            (*hash_array)[new_pos] = temp_hash_array[old_pos];
            (*hash_array)[new_pos+1] = temp_hash_array[old_pos];
        }
        else {
            (*hash_array)[new_pos] = -1;
            (*hash_array)[new_pos+1] = -1;
        }
    }
    free(temp_hash_array);
}

void dec2bin_string(unsigned int dec, char* bin_string);

int bin_string2dec(char* bin_string);

int hash_function(int dec, int global_depth) {
    
    // Βρες πόσα ψηφία έχει ο dec
    int counter = 0;
    int temp_dec = dec;
    while (temp_dec > 0) {
        temp_dec /= 10;
        counter++;
    }
    
    // Κάνε τον dec, string
    char buffer[counter+1];
    sprintf(buffer, "%d", dec);

    // Κάλεσε την FNV-1a Hash Function για τον dec που είναι σε μορφή string
    unsigned int key = hash(buffer, sizeof(buffer));

    // Κάνε το key από την FNV-1a, bin σε string και βρες το μήκος του
    char bin_string_key[33];
    dec2bin_string(key, bin_string_key);
    int len = strlen(bin_string_key);
    
    // Πάρε τα τελευταία global_depth ψηφία από τον string που βρέθηκε προηγουμένος
    char last_bins[33];
    counter = 0;
    for (int i = len-1 ; i > len-1 - global_depth ; i--) {
        last_bins[counter] = bin_string_key[i];
        counter++;
    }
    last_bins[counter] = '\0';

    // Κάνε αυτά τα τελευταία global_depth ψηφία δεκαδικό και επέστεψε τον ως new_key
    int new_key = bin_string2dec(last_bins);
    return new_key;
}

void dec2bin_string(unsigned int dec, char* bin_string) {
    
    if (dec == 0) {
        strcpy(bin_string, "0");
        return;
    }
    int bits_of_int = sizeof(int)*8;
    int bin_int[bits_of_int];
    int i = 0;
    while(1) {
        if (dec == 0) {
            break;
        }
        bin_int[i] = dec % 2;
        i++;
        dec = dec / 2;
    }
  
  char buffer[2];
  int counter = 0;
  int save_i = i;
  while (1) {
      if (save_i == 0) {
          break;
      }
      sprintf(buffer, "%d", bin_int[save_i-1]);
      save_i--;
      if (counter == 0) {
          strcpy(bin_string, buffer);
          counter++;
      }
      else {
          strcat(bin_string, buffer);
          counter++;
      }
  }
}

int bin_string2dec(char* bin_string) {

    int len = strlen(bin_string);
    int temp_len = len;
    int dec = 0;
    for (int i = 0 ; i < len ; i++) {
        dec += (bin_string[i] - '0') * pow(2, temp_len - 1);
        temp_len--;
    }
    return dec;
}
