#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/* 
 * Αν χρειάζεται (δηλαδή αν η hash_function δίνει key μεγαλύτερο του size
 * του Hash Array), μεγάλωσε το Hash Array στο μέγεθος του key
 */
int realloc_hash(int *hash_array, int key, int hash_array_size);

/*
 * Δέχεται έναν int, το Hash Array και το μέγεθος του Hash Array, 
 * ελέγχει αν το Hash Array χρειάζεται resize και επιστρέφει το key 
 * του Hash Table
 */
int hash_function(int dec, int *hash_array, int hash_array_size);

/*
 * Μετέτρεψε έναν δεκαδικό σε bin και επέστρεψε ως string
 */
char* dec2bin_string(int dec);

#endif