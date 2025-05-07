# 🗃️ Database Implementation using Extendible Hash Table in C — Block-Level Record Management

This project implements an extendible hash table over a block-level storage manager, simulating the internal mechanics of a database system. Developed for the course **K18 - Υλοποίηση Συστημάτων Βάσεων Δεδομένων**, it focuses on efficient storage, retrieval, and indexing using extendible hashing principles and a provided block file (BF) layer.

---

## 📅 Course Info

- **Course**: K18 - Υλοποίηση Συστημάτων Βάσεων Δεδομένων  
- **Semester**: Χειμερινό 2021–2022  
- **Instructor**: Καθηγητής Ι. Ιωαννίδης  

---

## 🧑‍💻 Team

- Δημήτριος Χρυσός – 1115202100275  
- Αναστάσιος Μουμουλίδης – 1115202100108  
- Παναγιώτης Τσελίκας – 1115202100192

---

## 📚 Project Description

The system supports:
- Creation and management of **extendible hash files**
- **Insertion**, **lookup**, and **statistics** collection on stored records
- Low-level control using **BF_Block** memory primitives
- Handling records of type:
```c
typedef struct {
  int id;
  char name[15];
  char surname[20];
  char city[20];
} Record;
```

It utilizes a block cache system (BF layer) with LRU/MRU replacement and operates within strict memory management constraints.

---

## 🗂️ Project Structure

```
project/
├── bin/                     # Compiled executables
├── build/
│   └── runner/             # Build artifacts
├── examples/
│   ├── bf_main.c           # Example usage of BF layer
│   └── ht_main.c           # Main driver for testing hash table functions
├── include/
│   ├── bf.h                # Given BF block manager header
│   ├── hash_file.h         # API for high-level hash table operations
│   └── hash_table.h        # Internal hash table structures and helpers
├── lib/
│   └── libbf.so            # Provided shared library for BF system
├── src/
│   ├── hash_file.c         # Implementation of HT_* functions
│   └── hash_table.c        # Lower-level hash logic and bucket management
```

---

## ⚙️ Compilation

Build the entire project using:
```bash
make
```

To clean compiled binaries:
```bash
make clean
```

---

## 📈 Hashing & Storage Logic

- The global depth determines directory size.
- Buckets split when full, and local depths are updated.
- File's first block stores metadata.
- All reads/writes go through `BF_GetBlock`, `BF_UnpinBlock`, and `BF_SetDirty`.

---

## 🧩 Design Internals & Assumptions

- **Global Depth Initialization**: We initialize `global_depth = 1`.
- **Hash Function**: We use **FNV-1a** hash function. After hashing:
  - Convert result to binary string.
  - Take the `global_depth` least significant bits.
  - Convert to decimal — this is the hash value.
- **Hash Table Management**:
  - Stored in custom blocks with `HT_blocks` metadata.
  - When full, `double_hash()` is called to double the table size and increase global depth.
- **Custom Assumptions**:
  - In `HashStatistics`, we pass `int* indexDesc` instead of `char* filename`, per eclass clarification.
  - Block 0 (header) holds hash table metadata and root configuration.
  - Each record block has its own local depth.
  - Hash table entries are stored across multiple blocks if needed.

---

## 🔧 Helper Functions

- `uint32_t hash(char* data, size_t bytes)`: FNV-1a hash
- `int hash_function(int id, int global_depth)`: Trims hash to match directory
- `void double_hash(void* header_info)`: Doubles hash table, increases global depth
- `void save_Hash_table(void* header_info)`: Persists hash table across blocks
- `void print_HashTable(int* table, int size)`: Debug tool
- `void dec2bin_string(unsigned int, char*)` & `int bin_string2dec(char*)`: Conversions

---

## ✅ Completed Functionality

We implemented all core HT_* functions:
- `HT_Init`, `HT_CreateIndex`, `HT_OpenIndex`, `HT_CloseFile`
- `HT_InsertEntry`, `HT_PrintAllEntries`, `HashStatistics`
- Includes modular testable implementation, robust unpinning, and LRU buffer handling.

---

