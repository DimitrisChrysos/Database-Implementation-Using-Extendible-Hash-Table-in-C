#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"


void double_hash(int *hash_array, int hash_array_size) {

    // Αντέγραψε το αρχικό hash_array
    int* temp_hash_array = (int*)malloc(hash_array_size*sizeof(int));
    for (int i = 0 ; i < hash_array_size ; i++) {
        temp_hash_array[i] = hash_array[i];
    } 

    // Διπλασίασε το αρχικό array
    hash_array = realloc(hash_array, 2*hash_array_size*sizeof(int));

    // Διόρθωσε τις καινούργιες θέσεις να δείχνουν στα σωστά block μετά το διπλασιασμό
    for (int i = 0 ; i < hash_array_size ; i++) {
        int old_pos = i;
        int new_pos = 2*old_pos;
        if (temp_hash_array[old_pos] != NULL) {
            hash_array[new_pos] = temp_hash_array[old_pos];
            hash_array[new_pos+1] = temp_hash_array[old_pos];
        }
        else {
            hash_array[new_pos] = NULL;
            hash_array[new_pos+1] = NULL;
        }
    }
    free(temp_hash_array);
}

char* dec2bin_string(int dec);

int bin_string2dec(char* bin_string);

int hash_function(int dec, int global_depth) {
    
    int key = 0;
    while (1) {
        if (dec == 0) {
            break;
        }
        int digit = dec % 10;
        key += digit;
        dec = dec / 10;
    }

    char bin_string_key[33] = dec2bin_string(key);
    // Να φτιάξω να επιστρέφει η hash_function έναν δεκαδικό που άμα μπει στο hash_table
    // να μας δίνει το block που πρέπει!

    return key;
}

char* dec2bin_string(int dec) {
    
    int bits_of_int = sizeof(int)*8;
    int bin_int[bits_of_int];
    char bin_string[bits_of_int];
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
  return bin_string;
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
