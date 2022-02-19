/*******************************************************************************
 * Credit
 * ------
 * Original Author: Stephen Brennan
 * Original Program: LSH (Libstephen SHell)
*******************************************************************************/

/************************************************************
 * Name: Ravy Thach
 * ID: 004992997
 * Date: 2/21/2022
 * 
 * Purpose: A shell program, TSH (Toy SHell), that 
 *          can execute custom built-in commands 
 *          as well as UNIX commands.
 * 
************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  Global variables used by built-in shell commands:
 */
char *shellname = "myshell";
char *terminator = ">";
const int ALIAS_SIZE = 10;
char *alias_names[ALIAS_SIZE] = { "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" };
char *alias_commands[ALIAS_SIZE] = { "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "\0" };

/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int stop(char **args);
int setshellname(char **args);
int setterminator(char **args);
int newname(char **args);
int listnewnames(char **args);
int savenewnames(char **args);
int readnewnames(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "stop",
  "setshellname",
  "setterminator",
  "newname",
  "listnewnames",
  "savenewnames",
  "readnewnames"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &stop,
  &setshellname,
  &setterminator,
  &newname,
  &listnewnames,
  &savenewnames,
  &readnewnames
};

/*
  Helper functions that assist with builtin command implementation
*/
int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/**
   Purpose: Retrieves index of alias from alias_names list

   @param alias String of alias to check
   @return If alias found, returns index
   @return if alias not found, returns -1
 */
int get_alias_index(char *alias) {
  for (int i = 0; i < ALIAS_SIZE; i++)
  {
    if (strcmp(alias_names[i], alias) == 0)
      return i;
  }

  return -1;
}

/**
   Purpose: Retrieve index of command from alias_commands list

   @param alias String of command to check
   @return If command found, returns index
   @return if command not found, returns -1
 */
int get_cmd_index(char *cmd) {
  for (int i = 0; i < ALIAS_SIZE; i++)
  {
    if (strcmp(alias_commands[i], cmd) == 0)
      return i;
  }

  return -1;
}

void remove_alias(char *alias)
{
    int alias_index = get_alias_index(alias);

    if (alias_index == -1) {
      fprintf(stderr, "tsh: alias \"%s\" does not exist\n", alias);
    } else {
      alias_names[alias_index] = "\0";
      alias_commands[alias_index] = "\0";
    }
}


/*
  Builtin function implementations.
*/

/**
   Purpose: Changes the current directory.

   @brief Builtin command: cd
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "tsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("tsh");
    }
  }
  return 1;
}

/**
   Purpose: Displays all built-in commands that the user can execute.

   @brief Builtin command: help
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(char **args)
{
  int i;
  printf("Toy SHell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   Purpose: Terminates execution of Toy SHell.

   @brief: Builtin command: stop
   @param args List of args. Not examined.
   @return Always returns 0
 */
int stop(char **args)
{
  return 0;
}

/**
   Purpose: Set a custom shell name for the command prompt. If no arg is passed, 
            change to default shell name "myshell".

   @brief: Builtin command: setshellname <name>
   @param args List of args.
          args[0] is "setshellname".
          args[1] is custom shell name.
   @return Always returns 1, to continue executing.
 */
int setshellname(char **args)
{
	if (args[1] == NULL)
		shellname = "myshell";
	else
		shellname = args[1];

	return 1;
}

/**
   Purpose: Set a custom terminator for the command prompt. If no arg is passed,
            change to default terminator ">".

   @brief: Builtin command: setterminator <name>
   @param args List of args.
          args[0] is "setterminator". 
          args[1] is custom terminator name.
   @return Always returns 1, to continue executing.
 */
int setterminator(char **args)
{
	if (args[1] == NULL) {
		terminator = ">";
	} else {
		terminator = args[1];
	}

	return 1;
}

/**
   Purpose: Define a custom alias that the user can execute in place of a 
            builtin or UNIX command. Can remove or replace existing aliases.

   @brief: Builtin command: newname <new_name> | newname <new_name> <old_name>
   @param args List of args. 
          args[0] is "newname".
          args[1] is the alias name.
          args[2] is the command name.
   @return Always returns 1, to continue executing.
 */
int newname(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "tsh: expected argument to \"newname\"\n");
  } 
  else if (args[2] == NULL) 
  {
    remove_alias(args[1]);
  } 
  else if (args[3] == NULL) {	  // two arguments: add or replace alias
	  char *alias_new = args[1];
    char *alias_cmd = args[2];
    
    int alias_cmd_pos = get_cmd_index(alias_cmd);
    int alias_new_pos = get_alias_index(alias_new);

    // TODO: make function for adding and removing aliases
    if (alias_new_pos != -1) {                // Replace command for an existing alias
      alias_names[alias_new_pos] = alias_new;
      alias_commands[alias_new_pos] = alias_cmd;

    } else if (alias_cmd_pos != -1) {         // Replace alias for corresponding command
      alias_names[alias_cmd_pos] = alias_new;
      alias_commands[alias_cmd_pos] = alias_cmd;

    } else {
      int alias_added = 0;                    // Add new alias
      for (int i = 0; i < ALIAS_SIZE; i++) {
        if (strcmp(alias_names[i], "\0") == 0) {
          alias_names[i] = alias_new;
          alias_commands[i] = alias_cmd;
          alias_added = 1;
        }

        if (alias_added)
          break;
      }

      if (!alias_added) {
        fprintf(stderr, "tsh: max number of aliases exceeded (%d)\n", ALIAS_SIZE);
      }
    }

	} else {
		fprintf(stderr, "tsh: too many arguments to \"newname\"\n");
	}

  return 1;
}

/**
   Purpose: Outputs all the aliases that have been defined by the user.

   @brief: Builtin command: listnewnames
   @param args List of args. args[0] is "listnewnames".
   @return Always returns 1, to continue executing.
 */
int listnewnames(char **args)
{
  for (int i = 0; i < ALIAS_SIZE; i++)
  {
    if (strcmp(alias_names[i], "\0") != 0)
      printf("%s %s\n", alias_names[i], alias_commands[i]);
  }

  return 1;
}

/**
   Purpose: Stores all currently defined aliases in the file <file_name>

   @brief: Builtin command: savenewnames <file_name>
   @param args List of args.
          args[0] is "savenewnames".
          args[1] is a file name.
   @return Always returns 1, to continue executing.
 */
int savenewnames(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "tsh: expected argument to \"savenewnames\"\n");
    return 1;
  }

  FILE *fp = fopen(args[1], "w");
  if(fp == NULL) {
    fprintf(stderr, "tsh: cannot create file\n");
    return 1;
  }

  for (int i = 0; i < ALIAS_SIZE; i++)
  {
    if (strcmp(alias_names[i], "\0") != 0)
      fprintf(fp, "%s %s\n", alias_names[i], alias_commands[i]);
  }

  fclose(fp);

  return 1;
}

/**
   Purpose: Reads all aliases in the file <file_name> and outputs to the user.

   @brief: Builtin command: readnewnames <file_name>
   @param args List of args.
          args[0] is "readnewnames".
          args[1] is a file name.
   @return Always returns 1, to continue executing.
 */
int readnewnames(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "tsh: expected argument to \"readnewnames\"\n");
    return 1;
  }

  FILE *fp;
  fp = fopen(args[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "tsh: file does not exist\n");
    return 1;
  }

  char c = fgetc(fp);
  while (c != EOF) {
    printf("%c", c);
    c = fgetc(fp);
  }

  fclose(fp);

  return 1;
}


/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("tsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("tsh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  // If alias exists, replace 'args[0]' with corresponding command
  for (int i = 0; i < ALIAS_SIZE; i++)  
  {
    if (strcmp(args[0], alias_names[i]) == 0) {
      args[0] = alias_commands[i];
    }
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void)
{
#ifdef LSH_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We recieved an EOF
    } else  {
      perror(strcat("lsh", ": getline\n"));
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define LSH_RL_BUFSIZE 1024
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "tsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "tsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "tsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "tsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("%s %s ", shellname, terminator);
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    //free(line);
    //free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
