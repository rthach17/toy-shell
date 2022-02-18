/***************************************************************************//**
  @file         main.c
  @author       Stephen Brennan
  @date         Thursday,  8 January 2015
  @brief        LSH (Libstephen SHell)
*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Global variables used by built-in shell commands:
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
/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "stop",
  "setshellname",
  "setterminator",
  "newname"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &stop,
  &setshellname,
  &setterminator,
  &newname
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//void add_alias(char *alias, char *cmd)

int get_alias_names_pos(char *alias) {
  for (int i = 0; i < ALIAS_SIZE; i++)
  {
    if (strcmp(alias_names[i], alias) == 0)
      return i;
  }

  return -1;
}

int get_alias_cmd_pos(char *cmd) {
  for (int i = 0; i < ALIAS_SIZE; i++)
  {
    if (strcmp(alias_commands[i], cmd) == 0)
      return i;
  }

  return -1;
}

void display_aliases(void) {
  for (int i = 0; i < ALIAS_SIZE; i++)  // Displays all aliases
  {
    printf("Alias %d: %s - %s\n", i+1, alias_names[i], alias_commands[i]);
  }
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: stop.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int stop(char **args)
{
  return 0;
}

/**
 *
 */
int setshellname(char **args)
{
	if (args[1] == NULL)
		shellname = "myshell";
	else
		shellname = args[1];

	return 1;
}

/*
 *
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

/*
 *
 */
int newname(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "lsh: expected argument to \"newname\"\n");
  } else if (args[2] == NULL) {	  // one argument: remove alias
    char *alias = args[1];
    int alias_pos = get_alias_names_pos(alias);

    if (alias_pos == -1) {
      fprintf(stderr, "lsh: the alias \"%s\" does not exist\n", alias);
    } else {
      alias_names[alias_pos] = "\0";
      alias_commands[alias_pos] = "\0";

      display_aliases();
    }
  } else if (args[3] == NULL) {	  // two arguments: add or replace alias
	  char *alias_new = args[1];
    char *alias_cmd = args[2];
    
    int alias_cmd_pos = get_alias_cmd_pos(alias_cmd);
    int alias_new_pos = get_alias_names_pos(alias_new);

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
        fprintf(stderr, "lsh: max number of aliases exceeded (%d)\n", ALIAS_SIZE);
      }
    }

    display_aliases();
	} else {
		fprintf(stderr, "lsh: too many arguments to \"newname\"\n");
	}

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
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
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

  // If alias exists, replace 'arg' with corresponding command from list
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
      perror("lsh: getline\n");
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
    fprintf(stderr, "lsh: allocation error\n");
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
        fprintf(stderr, "lsh: allocation error\n");
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
    fprintf(stderr, "lsh: allocation error\n");
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
        fprintf(stderr, "lsh: allocation error\n");
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

   // free(line);
   // free(args);
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
