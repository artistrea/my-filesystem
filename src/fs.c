#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fs.h"

const int cdback_len = strlen(CDBACK);
const int mantain_dir_len = strlen(MANTAIN_DIR);

FILE *main_mem;

struct decoded_file_data {
  uint8_t flags;
  uint16_t data_size;
  char* name;
  uint32_t* data; // may be addr or chars
};

struct encoded_file_data {
  uint8_t n_blocks;
  struct block_data *blocks;
};

uint16_t min(uint16_t a, uint16_t b) {
  if (a > b) return b;
  return a;
}

// allocates blocks
void encode_file_data(struct decoded_file_data decoded, struct encoded_file_data *encoded) {
  uint8_t name_size = strlen(decoded.name);
  uint16_t first_block_data_len = min(decoded.data_size, (uint16_t)DATA_SIZE_MAX - (uint16_t)name_size * sizeof(*encoded->blocks->name));
  
  uint16_t rest_len = 0;
  if (decoded.data_size > first_block_data_len) {
    rest_len = decoded.data_size - first_block_data_len;
  }
  uint8_t n_blocks = rest_len/(DATA_SIZE_MAX) + 1;

  encoded->n_blocks = n_blocks;
  encoded->blocks = malloc(sizeof(struct block_data) * n_blocks);

  memcpy(
    encoded->blocks[0].data,
    decoded.data,
    first_block_data_len
  );
  encoded->blocks[0].data_size = first_block_data_len;
  strcpy(encoded->blocks[0].name, decoded.name);
  encoded->blocks[0].name_size = strlen(decoded.name);
  encoded->blocks[0].flags = decoded.flags;

  for (int i=1;i<n_blocks;i++) {
    encoded->blocks[i].data_size = DATA_SIZE_MAX;
    memcpy(
      encoded->blocks[i].data,
      &decoded.data[(i-1)*DATA_SIZE_MAX + first_block_data_len],
      DATA_SIZE_MAX
    );
    encoded->blocks[i].name_size = 0;
    encoded->blocks[i].flags = decoded.flags;
  }
}

void write_encoded_file_data(struct encoded_file_data encoded, uint32_t start_block) {
  fseek(main_mem, start_block * FILE_BLOCK_SIZE, SEEK_SET);

  while (encoded.n_blocks--) {
    fwrite(&encoded.blocks->flags, sizeof(encoded.blocks->flags), 1, main_mem);
    fwrite(&encoded.blocks->name_size, sizeof(encoded.blocks->name_size), 1, main_mem);
    fwrite(&encoded.blocks->data_size, sizeof(encoded.blocks->data_size), 1, main_mem);
    fwrite(encoded.blocks->name,  sizeof(*encoded.blocks->name), encoded.blocks->name_size, main_mem);
    fwrite(encoded.blocks->data,  sizeof(*encoded.blocks->data), encoded.blocks->data_size, main_mem);

    encoded.blocks++;
  }

  fflush(main_mem);
}

uint32_t find_free_contiguous_blocks(uint8_t n);

void reset_mem(FILE *mem) {
  // fills with ones
  uint8_t empty_blocks[MAX_FILES * FILE_BLOCK_SIZE];
  // fills with ones, so that all blocks start as free
  memset(empty_blocks, 0xff, MAX_FILES * FILE_BLOCK_SIZE);
  fseek(mem, 0, SEEK_SET);
  fwrite(empty_blocks, FILE_BLOCK_SIZE, MAX_FILES, mem);

  // create root
  struct decoded_file_data root = {
    .name=ROOT_DIRNAME,
    .flags=FILE_BLOCK_DIRECTORY
  };

  struct encoded_file_data encoded_root;

  encode_file_data(root, &encoded_root);

  write_encoded_file_data(encoded_root, 0);

  fflush(mem);
}

void setup_filesystem(char *main_mem_location) {
  // make sure simulated main memory exists with the expected size
  main_mem = fopen(main_mem_location, "r+"); // file must exist

  if (main_mem == NULL) { // file didnt exist
    main_mem = fopen(main_mem_location, "w+"); // creates new if doesn't exist
    // writes ff to all bytes, creates root
    reset_mem(main_mem);
  }
}

// return 0 means something has gone wrong
uint32_t find_free_contiguous_blocks(uint8_t n) {
  uint32_t i=MAX_FILES-1;
  uint8_t acc=0;

  uint8_t flags;

  while (i > 0) {
    fseek(main_mem, i*FILE_BLOCK_SIZE, SEEK_SET);

    fread(&flags, sizeof(flags), 1, main_mem);

    if (flags & FILE_BLOCK_FREE) {
      acc++;
      if (acc == n) break;
    }
    i--;
  }

  return i;
}



int fs_get_filename(char* abs_path, char* result) {
  int j = strlen(abs_path)-1;
  if (abs_path[j] == SEPARATOR) j--;
  int i=j-1;

  while (i > 0 && abs_path[i] != SEPARATOR) {
    i--;
  }

  memcpy(result, &abs_path[i+1], j-i);
  return 0;
}

void get_block(uint32_t block, struct block_data *meta) {
  fseek(main_mem, block * FILE_BLOCK_SIZE, SEEK_SET);

  fread(&meta->flags, sizeof(meta->flags), 1, main_mem);
  fread(&meta->name_size, sizeof(meta->name_size), 1, main_mem);
  fread(&meta->data_size, sizeof(meta->data_size), 1, main_mem);

  fread(meta->name, sizeof(*meta->name), meta->name_size, main_mem);
  meta->name[meta->name_size] = 0;

  fread(meta->data, sizeof(*meta->data), meta->data_size, main_mem);
}

// returns 0 if ok
int get_block_from_path(const char* path, uint32_t *block_addr, struct block_data *block) {
  int i=2;
  int j;
  int path_len = strlen(path);

  struct block_data next_blk;
  *block_addr = 0;

  get_block(*block_addr, block);

  while (i < path_len) {
    j=i+1;

    if (!(block->flags & FILE_BLOCK_DIRECTORY)) {
      return 1;
    }

    while (j < path_len && path[j] != SEPARATOR) {
      j++;
    }


    int found=0;

    for (int k=0; k<block->data_size;k++) {
      *block_addr = block->data[k];

      get_block(*block_addr, &next_blk);

      if (next_blk.name_size == j-i && memcmp(&next_blk.name, &path[i], j-i) == 0) {
        found=1;
        *block=next_blk;
        break;
      }
    }

    if (!found) {
      return 1;
    }

    if (i==j) { // ignore separator
      i++;
      continue;
    }

    i=j+1;
  }

  return 0;
}

int fs_link(char* dir, char* to_link) {
  char *file_name = malloc(strlen(to_link));

  fs_get_filename(to_link, file_name);

  char *new_path = malloc(strlen(file_name) + strlen(dir) + 1);

  fs_join(dir, file_name, new_path);

  struct block_data block;
  uint32_t block_addr;
  int already_has_block = (0 == get_block_from_path(new_path, &block_addr, &block));

  // already has someone using the path
  if (already_has_block) return 1;

  uint32_t parent_block_addr;
  struct block_data parent_block;

  int res = get_block_from_path(dir, &parent_block_addr, &parent_block);

  if (res) {
    free(file_name);
    return 1;
  }

  res = get_block_from_path(to_link, &block_addr, &block);

  if (res) {
    free(file_name);
    return 1;
  }

  return link_blocks(&parent_block, parent_block_addr, block_addr);
}

int unlink_blocks(struct block_data *parent_block, uint32_t parent_addr, uint32_t to_unlink_addr) {
  int i=0;

  while (i < parent_block->data_size && parent_block->data[i] != to_unlink_addr) i++;

  // not linked
  if (i == parent_block->data_size) return 1;

  memcpy(&parent_block->data[i], &parent_block->data[i+1], parent_block->data_size - i - 1);
  parent_block->data_size--;

  write_encoded_file_data(
    (struct encoded_file_data){
      .blocks=parent_block,
      .n_blocks=1
    },
    parent_addr
  );

  return 0;
}

int fs_unlink(char* path) {
  uint32_t block_addr;

  struct block_data block;

  int res = get_block_from_path(path, &block_addr, &block);

  if (res) {
    return 1;
  }

  char *parent_path = malloc(strlen(path)+1);
  fs_join(path, "..", parent_path);

  char *file_name = malloc(strlen(path)+1);

  uint32_t parent_block_addr;

  struct block_data parent_block;

  fs_get_filename(path, file_name);

  res = get_block_from_path(parent_path, &parent_block_addr, &parent_block);
  // printf("parent_addr: %d\n", parent_block_addr);

  if (res) {
    printf("fatal error when getting block from path\n");
    free(parent_path);
    free(file_name);
    return 1;
  }

  return unlink_blocks(&parent_block, parent_block_addr, block_addr);
}

// TODO: check for repeated links (by name, not by addr)
// in the meanwhile, protect before calling this
int link_blocks(struct block_data *parent_block, uint32_t parent_addr, uint32_t to_link_addr) {
  parent_block->data[parent_block->data_size++] = to_link_addr;

  write_encoded_file_data(
    (struct encoded_file_data){
      .blocks=parent_block,
      .n_blocks=1
    },
    parent_addr
  );
  return 0;
}

int fs_mkdir(char* path) {
  // printf("creating dir %s\n", path);
  uint32_t parent_block_addr;

  struct block_data parent_block;

  int res = get_block_from_path(path, &parent_block_addr, &parent_block);

  if (res == 0) {
    printf("Directory already exists\n");
    return 1;
  }

  char *parent_path = malloc(strlen(path)+1);
  fs_join(path, "..", parent_path);

  char *dir_name = malloc(strlen(path)+1);

  fs_get_filename(path, dir_name);

  res = get_block_from_path(parent_path, &parent_block_addr, &parent_block);
  // printf("parent_addr: %d\n", parent_block_addr);

  if (res) {
    printf("fatal error when getting block from path\n");
    free(parent_path);
    free(dir_name);
    return 1;
  }

  uint32_t block_addr = find_free_contiguous_blocks(1);

  if (block_addr == 0) {
    printf("fatal error when getting new block\n");
    free(parent_path);
    free(dir_name);
    return 1;
  }

  link_blocks(&parent_block, parent_block_addr, block_addr);

  struct encoded_file_data encoded_dir;

  encode_file_data(
    (struct decoded_file_data){
      .data_size=0,
      .flags=FILE_BLOCK_DIRECTORY,
      .name=dir_name
    },
    &encoded_dir
  );

  write_encoded_file_data(
    encoded_dir,
    block_addr
  );


  // printf("at %s create %s\n", parent_path, dir_name);
  // printf("at %d create %d\n", parent_block_addr, block_addr);

  free(parent_path);
  free(dir_name);

  return 0;
}

// allocates block_data*
struct block_data* fs_ls(const char* abs_path, int *blocks_returned) {
  struct block_data dir_block;
  uint32_t dir_addr;

  int res = get_block_from_path(abs_path, &dir_addr, &dir_block);
  if (res) {
    *blocks_returned = 0;
    return NULL;
  }

  *blocks_returned = dir_block.data_size;

  struct block_data* ret = malloc(sizeof(struct block_data) * dir_block.data_size);

  for (int k=0; k<dir_block.data_size;k++) {
    get_block(dir_block.data[k], &ret[k]);
  }

  return ret;
}

int fs_join(char* cwd, char* path, char* result) {
  if (memcmp(ROOT_DIRNAME, path, sizeof(ROOT_DIRNAME)) == 0) {
    strcpy(result, path);
    return 0;
  }

  size_t cwd_len = strlen(cwd), path_len = strlen(path);

  strcpy(result, cwd);

  size_t i = 0, j, append_at=cwd_len;

  if (cwd[cwd_len - 1] != SEPARATOR) {
    result[append_at++] = SEPARATOR;
  }

  while (i < path_len) {
    j=i;
    while (j < path_len && path[j] != SEPARATOR) {
      j++;
    }
    if (i==j && result[append_at-1] == SEPARATOR) { // found extra separator
      i++;
      continue;
    }

    if (j - i == cdback_len && memcmp(CDBACK, &path[i], cdback_len) == 0) {
      if (append_at == strlen(ROOT_DIRNAME)+1) {
        return 1;
      }
      append_at--;
      append_at--;

      while (append_at > strlen(ROOT_DIRNAME) && result[append_at] != SEPARATOR) {
        append_at--;
      }
      append_at++;
      result[append_at]=0;
    } else if (j - i == mantain_dir_len && memcmp(MANTAIN_DIR, &path[i], mantain_dir_len) == 0) {
      // do nothing, mantains dir
    } else {
      memcpy(&result[append_at], &path[i], j-i+1);
      append_at += j-i+1;
    }


    i = j + 1;
  }

  return 0;
}

int fs_create_file(char* path, uint32_t *data, uint16_t data_len) {
  struct block_data block;
  uint32_t block_addr;
  int already_has_block = (0 == get_block_from_path(path, &block_addr, &block));

  if (already_has_block) return 1;

  char *parent_path = malloc(strlen(path)+1);
  fs_join(path, "..", parent_path);

  char *file_name = malloc(strlen(path)+1);

  fs_get_filename(path, file_name);

  uint32_t parent_block_addr;

  struct block_data parent_block;

  int res = get_block_from_path(parent_path, &parent_block_addr, &parent_block);

  if (res) {
    printf("fatal error when getting block from path\n");
    free(parent_path);
    free(file_name);
    return 1;
  }
  struct encoded_file_data encoded_file;

  encode_file_data(
    (struct decoded_file_data){
      .data_size=data_len,
      .data=data,
      .flags=0,
      .name=file_name
    },
    &encoded_file
  );

  block_addr = find_free_contiguous_blocks(encoded_file.n_blocks);

  if (block_addr == 0) {
    printf("fatal error when getting new block\n");
    free(parent_path);
    free(file_name);
    return 1;
  }

  parent_block.data[parent_block.data_size++] = block_addr;

  write_encoded_file_data(
    (struct encoded_file_data){
      .blocks=&parent_block,
      .n_blocks=1
    },
    parent_block_addr
  );

  write_encoded_file_data(
    encoded_file,
    block_addr
  );

  free(parent_path);
  free(file_name);

  return 0;
}
