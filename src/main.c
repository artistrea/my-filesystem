#include "cli.h"
#include "colors.h"
#include "fs.h"
#include <stdio.h>

int main(int argc, char** argv) {
  if (argc != 2) {
    printf(
      "You must call this executable passing the path to a file for storage\n"
      "For example: ./bin files\n"
    );
    return 1;
  }

  char* filesys_name = argv[1];

  printf(
    "This is a filesystem implemented very naively (read 'the first solutions that came to head')\n"
    "Content is persisted on storage\n"
    "You can create, remove and move files and directories using commands similar to linux\n"
    "Some "
  );
  textcolor(CLR_BRIGHT, CLR_YELLOW);
  printf(
    "commands may have "
  );
  textcolor(CLR_CLEAR, CLR_WHITE);
  printf(
    "slightly "
  );
  textcolor(CLR_BRIGHT, CLR_YELLOW);
  printf(
    "different behavior"
  );
  textcolor(CLR_CLEAR, CLR_WHITE);
  printf(
    ", such as the "
  );
  textcolor(CLR_BRIGHT, CLR_YELLOW);
  printf(
    "'mv' "
  );
  textcolor(CLR_CLEAR, CLR_WHITE);
  printf(
    "command\n"
    "\tOn linux the 'mv' command may change the name of a file or directory\n"
  );
  printf(
    "\tOn this filesystem you can use "
  );
  textcolor(CLR_BRIGHT, CLR_YELLOW);
  printf(
    "'chn' to change "
  );
  textcolor(CLR_CLEAR, CLR_WHITE);
  printf(
    "a file or directory "
  );
  textcolor(CLR_BRIGHT, CLR_YELLOW);
  printf(
    "name "
  );
  textcolor(CLR_CLEAR, CLR_WHITE);tab=readme-ov-filetab=readme-ov-file
  printf(
    "instead\n"
  );
  printf(
    "Enter 'help' to know more about the commands implemented!\n"
  );

  // create file storage and root directory if file storage doesnt exist
  setup_filesystem(filesys_name);

  char cmd_str[512];
  cmd_str[511] = 0;

  while (1) {
    textcolor(CLR_BRIGHT, CLR_GREEN);
    printf("%s", get_cwd());
    textcolor(CLR_DIM, CLR_CYAN);
    printf(" > ");
    textcolor(CLR_CLEAR, CLR_WHITE);
    fgets(cmd_str, 511, stdin);

    struct parsed_command com = cli_parse_cmd(cmd_str);

    if (com.call != NULL)
      com.call(com.argc, com.argv);
    else {
      printf("Unrecognized command. Use 'help' to see more info\n");
    }
  }

  return 0;
}
