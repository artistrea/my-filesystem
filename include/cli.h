#ifndef CLI_H
#define CLI_H

struct parsed_command {
  int argc;
  // max argc
  char *argv[100];

  void (*call)(int argc, char** argv);
};

struct parsed_command cli_parse_cmd(char* cmd_str);

char* get_cwd();

#endif // CLI_H
