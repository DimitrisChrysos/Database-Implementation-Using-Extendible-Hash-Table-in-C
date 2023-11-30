#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"
// #include "bf.h"


int realloc_hash(int *hash_array, int key, int hash_array_size) {

    if (key > hash_array_size) {
        hash_array = realloc(hash_array, key*sizeof(int));
        return 1;
    }
    return 0;
}

int hash_function(int dec, int *hash_array, int hash_array_size) {
    
    int key = 0;
    while (1) {
        if (dec == 0) {
            break;
        }
        int digit = dec % 10;
        key += digit;
        dec = dec / 10;
    }
    realloc_hash(hash_array, key, hash_array_size);
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
