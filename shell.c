#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>


int ginsh_cd(char **args);
int ginsh_help(char **args);
int ginsh_exit(char **args);
int ginsh_rm(char **args);
int ginsh_echo(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "echo",
  "rm"
};

int (*builtin_func[]) (char **) = {
  &ginsh_cd,
  &ginsh_help,
  &ginsh_exit
};

int ginsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int ginsh_cd(char **args)
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

int ginsh_help(char **args)
{
  int i;
  printf("Gincsh\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < ginsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int ginsh_exit(char **args)
{
  return 0;
}

int ginsh_launch(char **args)
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

int ginsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < ginsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return ginsh_launch(args);
}

int ginsh_rm(char **args)
{
  struct dirent *d;
  DIR *dir;
  char buf[256];
  dir = opendir(args[0]);
  while(d = readdir(dir))
  {               
    printf(buf, "%s/%s", "mydir", d->d_name);
    remove(buf);
  }
  return 1;
}
int ginsh_echo(char **args)
{
  while (*++args) {
    printf("%s", *args);
    if (args[1])
      printf(" ");
  }
  printf("\n");
  return 1;
}
char *ginsh_read_line(void)
{
#ifdef ginsh_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("lsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define ginsh_RL_BUFSIZE 1024
  int bufsize = ginsh_RL_BUFSIZE;
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
      bufsize += ginsh_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define ginsh_TOK_BUFSIZE 64
#define ginsh_TOK_DELIM " \t\r\n\a"

char **ginsh_split_line(char *line)
{
  int bufsize = ginsh_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, ginsh_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += ginsh_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, ginsh_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void ginsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    char cwd[256];
    printf("%s>",getcwd(cwd,sizeof(cwd)));
    line = ginsh_read_line();
    args = ginsh_split_line(line);
    status = ginsh_execute(args);

    free(line);
    free(args);
  } while (status);
}
int main(int argc, char **argv)
{
  printf("\n\nSHELL BY ROKO DUGUM\n\n");
  ginsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
