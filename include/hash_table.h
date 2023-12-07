#include <stdint.h>
#ifndef HASH_TABLE_H
#define HASH_TABLE_H


/*
 * FNV-1a Hash Function, που βρέθηκε εδώ:
 * https://cs.stackexchange.com/questions/60950/hash-multiple-integers-directly-using-fnv-1a
 */
uint32_t hash(char* data, size_t bytes);

/*
 * Εκτυπώνει τις τιμές του Hash Table
 */
void print_HashTable(int* hash_table, int hash_array_size);

/*
 * Αποθηκεύει το Hash Table σε blocks
 */
void save_Hash_table(void* header_inf);

/* 
 * Διπλασίασε τον πίνακα hash_array
 * Παίρνει ορίσματα, τη διεύθυνση του πίνακα Hash Table, και το μέγεθος του
 * Για κάθε θέση του hash_table με d bits πριν τον διπλασιασμό που έδειχνε σε block_number, 
 * μετά τον διπλασιασμό που για αυτή τη θέση πλέον υπάρχουν 2, d+1 bits θέσεις:
 *  a) Αν η παλιά θέση δεν έδειχνε σε block_number == NULL, κάνε τις δύο καινούργιες θέσεις 
 *     να δείχνουν στο block που έδειχνε η παλιά θέση  
 *  b) Αν η παλιά θέση έδειχνε σε block_number == NULL, κάνε τις δύο καινούργιες θέσεις να 
 *     δείχνουν επίσης σε NULL
 */
void double_hash(void* header_inf);

/*
 * Δέχεται έναν int και το global_depth του Hash Table και επιστρέφει ένα key
 */
int hash_function(int dec, int global_depth);

/*
 * Μετέτρεψε έναν δεκαδικό σε bin μέσα σε string και επέτρεψε το στο argument bin_string 
 */
void dec2bin_string(unsigned int dec, char* bin_string);

/*
 * Μετέτρεψε string από bin σε δεκαδικό και επέστρεψε τον
 */
int bin_string2dec(char* bin_string);



#endif