#ifndef FS_H
#define FS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "fs.h"


#define SEPARATOR '/'
#define CDBACK ".."
#define MANTAIN_DIR "."
#define ROOT_DIRNAME "~"

extern const int cdback_len;
extern const int mantain_dir_len;

#define FILE_BLOCK_SIZE 1024
#define MAX_FILES 256

enum FILE_BLOCK_FLAGS {
  FILE_BLOCK_FREE = 0b1,
  FILE_BLOCK_DIRECTORY = 0b10,
};

enum FS_MKDIR_FLAGS {
  FS_MKDIR_RECURSIVE = 0b1,
};

// rest of block excluding flags, name_size and data_size
// and considering name_size = 0
#define DATA_SIZE_MAX FILE_BLOCK_SIZE - 4

struct block_data {
  uint8_t flags;
  uint8_t n_blocks;
  uint8_t name_size; // if name_size == 0, means it is not start block of file
  uint16_t data_size;
  uint8_t blocks_left;
  char name[DATA_SIZE_MAX];
  uint32_t data[DATA_SIZE_MAX]; // may be addr or chars
};

int get_block_from_path(const char* path, uint32_t *block_addr, struct block_data *block);

int fs_join(char* cwd, char* path, char* result);

int fs_get_filename(char* abs_path, char* result);

void setup_filesystem(char *main_mem_location);

int fs_create_file(char* abs_path, uint32_t *data, uint16_t data_len);

void create_directory(char* abs_path, uint16_t data_len, uint32_t *data);

struct block_data* fs_ls(const char* abs_path, int *blocks_returned);

int fs_mkdir(char* path);

#endif // FS_H

