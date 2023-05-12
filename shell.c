#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <malloc.h>
#include <stdbool.h>
char *lsh_read_line(){
  char *line = NULL;
  ssize_t buffersize = 0;
  if(getline(&line, &bufsize, stdin)==-1 && feof(stdin))
    exit(EXIT_SUCCESS);
  else
    exit(EXIT_FAILURE);
  return line;
}

void lsh_loop(){
  int status;
  char* line;
  char** args;

  do{
    char cwd[PATH_MAX];
    printf("%s>",getcwd(cwd,sizeof(cwd)));
    
    line = lsh_read_line();
    args = lsh_read_args(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  }while(status);
}

int main (int argc, char *argv[]){
  lsh_loop();
  return EXIT_SUCCESS;
}
