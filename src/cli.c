#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "cli.h"
#include "fs.h"

// char result[1000];
// char data[] = "testano alguma coisa";
// fs_join(ROOT_DIRNAME, "teste,opa", result);
// printf("%s\n", result);
// fs_join(ROOT_DIRNAME, ",teste,opa,", result);
// printf("%s\n", result);
// fs_join(ROOT_DIRNAME, ",,Opa,,,opa", result);
// printf("%s\n", result);

// create_file(fs_join(cwd, path), sizeof(data), data);

struct recognized_cmd {
  char* cmd_name;
  char* description;
  void (*handler)(int argc, char** argv);
};

static char cwd[100] = ROOT_DIRNAME "/";

char* get_abs(char* path) {
  char* res = malloc(strlen(cwd) + strlen(path) + 1);
  if (memcmp(path, ROOT_DIRNAME, strlen(ROOT_DIRNAME)) == 0) {
    strcpy(res, path);
  } else {
    if (fs_join(cwd, path, res)) {
      printf("Invalid path given. Cannot create directory\n");
      return NULL;
    }
  }
  return res;
}

char* get_cwd() {
  return cwd;
}

void exit_handler(int _, char** __) {
  exit(0);
}

void print_help_handler(int argc, char **argv);

void cli_mkdir(int argc, char **argv);

void cli_touch(int argc, char **argv);

void cli_ls(int argc, char **argv);

void cli_cd(int argc, char **argv);

void cli_mv(int argc, char **argv);

void cli_rm(int argc, char **argv);

void cli_rmdir(int argc, char **argv);

void cli_print_tree(int argc, char **argv);

void cli_mv(int argc, char **argv) {
  if (argc != 3) {
    printf("'mv' command requires 2 parameters\n");
    return;
  }

  int res;

  char* from = get_abs(argv[1]);
  if (from == NULL) {
    printf("Could not resolve path %s", argv[1]);
    return;
  }

  char* to = get_abs(argv[2]);
  if (to == NULL) {
    printf("Could not resolve path %s", argv[2]);
    return;
  }

  if (strcmp(from, to) == 0) {
    free(to);
    free(from);
    return;
  }

  res = fs_link(to, from);

  if (res) {
    printf("Could not move %s to %s\n", from, to);
    free(from);
    free(to);

    return;
  }

  res = fs_unlink(from);

  if (res) {
    printf("Problem unlinking from '%s'\n", from);
    res = fs_unlink(to);
    if (res) {
      printf("Could not rollback operation!\n");
    }

    free(from);
    free(to);

    return;
  }

  free(from);
  free(to);
}

void cli_print_tree(int argc, char **argv) {
  if (argc != 1) {
    printf("'print-tree' command receives no parameters\n");
    return;
  }

  fs_print_full_filetree();
}

void cli_rm(int argc, char **argv) {
  if (argc != 2) {
    printf("rm expects one argument\n");
    return;
  }
  char *abs_path = get_abs(argv[1]);

  if (abs_path == NULL) {
    printf("Invalid path given. Cannot create directory\n");
    return;
  }

  fs_rm(abs_path, 0);
}

void cli_rmdir(int argc, char **argv) {
  if (argc != 2) {
    printf("rm expects one argument\n");
    return;
  }

  char *abs_path = get_abs(argv[1]);

  if (abs_path == NULL) {
    printf("Invalid path given. Cannot create directory\n");
    return;
  }

  fs_rm(abs_path, 1);
}

void cli_chn(int argc, char **argv) {
  if (argc != 3) {
    printf("'chn' expects two arguments\n");
    return;
  }

  char *abs_path = get_abs(argv[1]);

  if (abs_path == NULL) {
    printf("Invalid path given. Cannot create directory\n");
    return;
  }

  int res = fs_change_filename(abs_path, argv[2]);

  if (res) {
    printf("Could not find '%s' to change its name", argv[1]);
  }

  free(abs_path);
}

void cli_cwd() {
  printf("%s\n", get_cwd());
}

struct recognized_cmd default_cmds[] = {
  {
    .cmd_name="help",
    .handler=&print_help_handler,
    .description="Prints helpful information about this program"
  },
  {
    .cmd_name="print-tree",
    .handler=&cli_print_tree,
    .description="Traverses and prints entire filesystem tree"
  },
  {
    .cmd_name="ls",
    .handler=&cli_ls,
    .description="Lists directories and files in a given path"
  },
  {
    .cmd_name="mv",
    .handler=&cli_mv,
    .description="Moves file or directory to a given path (without changing its name)"
  },
  {
    .cmd_name="chn",
    .handler=&cli_chn,
    .description="Moves file or directory to a given path (without changing its name)"
  },
  {
    .cmd_name="mkdir",
    .handler=&cli_mkdir,
    .description="Creates a directory with the given name"
  },
  {
    .cmd_name="rmdir",
    .handler=&cli_rmdir,
    .description="Removes a directory from given path"
  },
  {
    .cmd_name="touch",
    .handler=&cli_touch,
    .description="Creates a file with the given name"
  },
  {
    .cmd_name="rm",
    .handler=&cli_rm,
    .description="Removes a file from given path"
  },
  {
    .cmd_name="cd",
    .handler=&cli_cd,
    .description="Navigates to given directory"
  },
  {
    .cmd_name="exit",
    .handler=&exit_handler,
    .description="Exits program"
  },
};

const int n_recognized_commands = sizeof(default_cmds) / sizeof(struct recognized_cmd);

void print_help_handler(int argc, char **argv) {
  printf("This is a filesystem being simulated inside a file.\n");
  printf("Commands:\n");
  for (int i=0;i<n_recognized_commands;i++) {
    printf(" - %s\n", default_cmds[i].cmd_name);
    printf("\t%s\n", default_cmds[i].description);
  }
}


void cli_mkdir(int argc, char **argv) {
  if (argc != 2) {
    printf("mkdir needs to receive one parameter\n");
    return;
  }

  char *result = get_abs(argv[1]);

  if (result == NULL) {
    printf("Invalid path given. Cannot create directory\n");
    return;
  }

  fs_mkdir(result);
}

void cli_touch(int argc, char **argv) {
  if (argc != 2) {
    printf("touch needs to receive one parameter\n");
    return;
  }

  char *result = get_abs(argv[1]);

  if (result == NULL) {
    printf("Invalid path given. Cannot create file\n");
    return;
  }
  int r = fs_create_file(result, NULL, 0);

  if (r) {
    printf("Could not create file\n");
  }
}

void cli_ls(int argc, char **argv) {
  char *result;
  if (argc == 2) {
    result = get_abs(argv[1]);

    if (result == NULL) {
      printf("Invalid path given. Cannot create directory\n");
      return;
    }
  } else if (argc == 1) {
    result = get_abs(cwd);
  } else {
    printf("You may pass 0 or 1 parameters to ls\n");
    return;
  }

  int blocks_returned;
  struct block_data* data = fs_ls(result, &blocks_returned);
  // fs_ls(result);
  for (int i=0;i<blocks_returned;i++) {
    printf("%s%c\n", data[i].name, (data[i].flags & FILE_BLOCK_DIRECTORY) ? '/' : ' ');
  }

  if (data != NULL) free(data);
  free(result);
}

void cli_cd(int argc, char **argv) {
  char *result;
  if (argc != 2) {
    printf("You need to pass 1 parameter to cd\n");
    return;
  }

  result = get_abs(argv[1]);
  if (result == NULL) {
    printf("Invalid path given.\n");
    return;
  }

  uint32_t t=0;
  struct block_data block;

  if (get_block_from_path(result, &t, &block)) {
    printf("Could not find directory '%s'\n", result);
    free(result);
    return;
  }
  if (!(block.flags & FILE_BLOCK_DIRECTORY)) {
    printf("%s is not a directory!\n", result);
    free(result);
    return;
  }
  strcpy(cwd, result);
  int cwd_len = strlen(cwd);

  if (cwd[cwd_len - 1] != '/') {
    cwd[cwd_len] = '/';
    cwd[cwd_len+1] = 0;
  }
  free(result);
}

// WARNING: mutates the passed string
struct parsed_command cli_parse_cmd(char* cmd_str) {
  int new_param=0;

  struct parsed_command ncdm = {
    .argc=1,
    .call=NULL,
  };
  ncdm.argv[0] = cmd_str;

  while (*cmd_str) {
    if (*cmd_str == ' ' || *cmd_str == '\n' || *cmd_str == '\r') {
      *cmd_str = 0; // just making it work
      new_param=1;
    } else if (new_param) {
      new_param=0;
      ncdm.argv[ncdm.argc++] = cmd_str;
    }

    cmd_str++;
  }

  for (int i=0;i<n_recognized_commands;i++) {
    if (strcmp(default_cmds[i].cmd_name, ncdm.argv[0]) == 0) {
      ncdm.call = default_cmds[i].handler;
    }
  }

  return ncdm;
}

