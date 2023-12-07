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

void save_Hash_table(void* header_inf) {
    
    // Πάρε το πρώτο block του Hash Table
    HT_info* header_info = header_inf;
    BF_Block* block;
    BF_Block_Init(&block);
    BF_GetBlock(header_info->file_desc, 1, block);
    void *data = BF_Block_GetData(block);
    int offset = BF_BLOCK_SIZE - sizeof(HT_blocks);
    HT_blocks *htb = data + offset;
    htb->num_of_indices = 0;

    // Αποθήκευσε το Hash Tabble
    int total_indeces = header_info->size_of_hash_table;
    int max_indeces_per_block = (BF_BLOCK_SIZE - sizeof(HT_blocks)) / sizeof(int);
    int indeces_inserted = 0;
    int count_blocks_used = 0;
    if (max_indeces_per_block < total_indeces) {
        while (1) {
            for (int i = 0 ; i < max_indeces_per_block ; i++) {
                int index_offset = count_blocks_used*max_indeces_per_block;
                memcpy(data + i*sizeof(int), &(header_info->hash_table[i + index_offset]), sizeof(int));
                htb->num_of_indices++;
                indeces_inserted++;
                if (indeces_inserted == total_indeces) {
                    htb->next_ht_block_id = -1;
                    BF_Block_SetDirty(block);
                    BF_UnpinBlock(block);
                    break;
                }
            }
            if (indeces_inserted == total_indeces) {
                break;
            }
            if (htb->next_ht_block_id == -1) {
                
                int new_block;
                BF_GetBlockCounter(header_info->file_desc, &new_block);
                htb->next_ht_block_id = new_block;
                BF_Block_SetDirty(block);
                BF_UnpinBlock(block);
                BF_AllocateBlock(header_info->file_desc, block);
                data = BF_Block_GetData(block);
                htb = data + offset;
                header_info->count_blocks_for_HT++;
                htb->next_ht_block_id = -1;
                htb->num_of_indices = 0;
            }
            else {
                int new_block;
                BF_GetBlockCounter(header_info->file_desc, &new_block);
                BF_Block_SetDirty(block);
                BF_UnpinBlock(block);
                BF_GetBlock(header_info->file_desc, htb->next_ht_block_id, block);
                data = BF_Block_GetData(block);
                htb->num_of_indices = 0;
                htb = data + offset;
                header_info->count_blocks_for_HT++;
            }
            count_blocks_used++;

        }
    }
    else {
        for (int i = 0 ; i < total_indeces ; i++) {
            memcpy(data + i*sizeof(int), &(header_info->hash_table[i]), sizeof(int));
            htb->num_of_indices++;
            indeces_inserted++;
        }
    }

    

    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
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
