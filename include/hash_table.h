#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/* 
 * Διπλασίασε τον πίνακα hash_array
 * Για κάθε θέση του hash_table με d bits πριν τον διπλασιασμό που έδειχνε σε block_number, 
 * μετά τον διπλασιασμό που για αυτή τη θέση πλέον υπάρχουν 2, d+1 bits θέσεις:
 *  a) Αν η παλιά θέση δεν έδειχνε σε block_number == NULL, κάνε τις δύο καινούργιες θέσεις 
 *     να δείχνουν στο block που έδειχνε η παλιά θέση  
 *  b) Αν η παλιά θέση έδειχνε σε block_number == NULL, κάνε τις δύο καινούργιες θέσεις να 
 *     δείχνουν επίσης σε NULL
 */
void double_hash(int *hash_array, int hash_array_size);

/*
 * Δέχεται έναν int και επιστρέφει ένα key
 */
int hash_function(int dec, int global_depth);

/*
 * Μετέτρεψε έναν δεκαδικό σε bin και επέστρεψε ως string
 */
char* dec2bin_string(int dec);

/*
 * Μετέτρεψε string από bin σε δεκαδικό και επέστρεψε τον
 */
int bin_string2dec(char* bin_string);

#endif