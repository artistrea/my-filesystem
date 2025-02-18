#include "cli.h"
#include "fs.h"
#include <stdio.h>

int main() {
  printf("Enter 'help' to know more about this system!\n");

  setup_filesystem("testano");

  char cmd_str[512];
  cmd_str[511] = 0; // just making sure. may not be necessary

  while (1) {
    printf("%s  |> ", get_cwd());
    fgets(cmd_str, 511, stdin);

    // fs_join("~/opa/", cmd_str, res);

    // printf("p: %s\n", res);
    struct parsed_command com = cli_parse_cmd(cmd_str);

    if (com.call != NULL)
      com.call(com.argc, com.argv);
    else {
      printf("Unrecognized command. Use 'help' to see more info\n");
    }
  }

  return 0;
}
