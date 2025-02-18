#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "cli.h"
#include "fs.h"

enum COMMANDS {
  CMD_CREATE_FILE,
  CMD_CREATE_DIR,
  CMD_LIST_DIR,
  CMD_HELP,
  CMD_UNRECOGNIZED,
  CMD_ERRORED,
  CMD_EXIT
};

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

void exit_handler(int _, char** __) {
  exit(0);
}

void print_help_handler();

void cli_mkdir(int argc, char **argv);

void cli_touch(int argc, char **argv);

void cli_ls(int argc, char **argv);

void cli_cd(int argc, char **argv);

void cli_mv(int argc, char **argv);

void cli_cp(int argc, char **argv);

void cli_rm(int argc, char **argv);

void cli_rmdir(int argc, char **argv);

void cli_mv(int argc, char **argv) {
  printf("TODO\n");
}

void cli_cp(int argc, char **argv) {
  printf("TODO\n");
}

void cli_rm(int argc, char **argv) {
  printf("TODO\n");
}

void cli_rmdir(int argc, char **argv) {
  printf("TODO\n");
}

static char cwd[100] = ROOT_DIRNAME "/";

char* get_cwd() {
  return cwd;
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
    .cmd_name="mv",
    .handler=&cli_mv,
    .description="Prints helpful information about this program"
  },
  {
    .cmd_name="mkdir",
    .handler=&cli_mkdir,
    .description="Creates a directory with the given name"
  },
  {
    .cmd_name="touch",
    .handler=&cli_touch,
    .description="Creates a file with the given name"
  },
  {
    .cmd_name="cd",
    .handler=&cli_cd,
    .description="Navigates to given directory"
  },
  // {
  //   .cmd_name="cwd",
  //   .handler=&cli_cwd,
  //   .description="Prints the current directory path"
  // },
  {
    .cmd_name="ls",
    .handler=&cli_ls,
    .description="Lists directories and files in a given path"
  },
  {
    .cmd_name="mv",
    .handler=&cli_mv,
    .description="Lists directories and files in a given path"
  },
  {
    .cmd_name="cp",
    .handler=&cli_cp,
    .description="Lists directories and files in a given path"
  },
  {
    .cmd_name="rm",
    .handler=&cli_rm,
    .description="Lists directories and files in a given path"
  },
  {
    .cmd_name="rmdir",
    .handler=&cli_rmdir,
    .description="Lists directories and files in a given path"
  },
  {
    .cmd_name="exit",
    .handler=&exit_handler,
    .description="Exits program"
  },
};

const int n_recognized_commands = sizeof(default_cmds) / sizeof(struct recognized_cmd);

void print_help_handler() {
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

  char *result = malloc((strlen(cwd) + strlen(argv[1]) + 1) * sizeof(char) );

  if (fs_join(cwd, argv[1], result)) {
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

  char *result = malloc((strlen(cwd) + strlen(argv[1]) + 1) * sizeof(char) );

  if (fs_join(cwd, argv[1], result)) {
    printf("Invalid path given. Cannot create file\n");
    return;
  }
  int r = fs_create_file(result, "", 0);

  if (r) {
    printf("Could not create file\n");
  }
}

void cli_ls(int argc, char **argv) {
  char *result;
  if (argc == 2) {
    if (memcmp(argv[1], ROOT_DIRNAME, strlen(ROOT_DIRNAME)) == 0) {
      result = argv[1];
    } else {
      result = malloc((strlen(cwd) + strlen(argv[1]) + 1) * sizeof(char) );

      if (fs_join(cwd, argv[1], result)) {
        printf("Invalid path given. Cannot create directory\n");
        return;
      }
    }
  } else if (argc == 1) {
    result = cwd;
  } else {
    printf("You may pass 0 or 1 parameters to ls\n");
    return;
  }

  int blocks_returned;
  struct block_data* data = fs_ls(result, &blocks_returned);
  // fs_ls(result);
  for (int i=0;i<blocks_returned;i++) {
    printf("\t%s%c\n", data[i].name, (data[i].flags & FILE_BLOCK_DIRECTORY) ? '/' : ' ');
  }

  if (data != NULL) free(data);
}

void cli_cd(int argc, char **argv) {
  char *result;
  if (argc != 2) {
    printf("You need to pass 1 parameter to cd\n");
    return;
  }

  if (memcmp(argv[1], ROOT_DIRNAME, strlen(ROOT_DIRNAME)) == 0) {
    result = argv[1];
  } else {
    result = malloc((strlen(cwd) + strlen(argv[1]) + 1) * sizeof(char) );

    if (fs_join(cwd, argv[1], result)) {
      printf("Invalid path given.\n");
      return;
    }
  }

  uint32_t t=0;
  struct block_data block;

  if (get_block_from_path(result, &t, &block)) {
    printf("Could not find directory '%s'\n", result);
    return;
  }
  if (!(block.flags & FILE_BLOCK_DIRECTORY)) {
    printf("%s is not a directory!\n", result);
    return;
  }
  strcpy(cwd, result);
  int cwd_len = strlen(cwd);

  if (cwd[cwd_len - 1] != '/') {
    cwd[cwd_len] = '/';
    cwd[cwd_len+1] = 0;
  }
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

  int found=0;

  for (int i=0;i<n_recognized_commands;i++) {
    if (strcmp(default_cmds[i].cmd_name, ncdm.argv[0]) == 0) {
      ncdm.call = default_cmds[i].handler;
    }
  }

  return ncdm;
}

