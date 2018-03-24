#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
void syserror(const char *s);

int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];
    pid_t pid;
    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        int num_words = split_cmd_line(line, line_words);
	char* cmd[num_words + 1];
	int cmdIndex = 0;
	int fd_in = 0;
	int p[2];
        int first = 1;
        
        for (int i=0; i < num_words; i++)
	  {
	    if(strcmp(line_words[i], "|") == 0)
	      {
		pipe(p);
		switch(pid = fork())
		  {
	          case -1:
		    syserror("fork failed");
	          case 0:
		    dup2(fd_in, 0);
		    dup2(p[1],1);
		    close(p[0]);
		    cmd[cmdIndex] = NULL;
		    execvp(cmd[0], cmd);
		    syserror("could not exec");
		  default:
		    while(wait(NULL) != -1);
		    close(p[1]);
		    fd_in = p[0];
		    cmdIndex = 0;
		  }
		
	      }
	    else if(strcmp(line_words[i], ">") == 0)
	      {
		printf("Found a >");
	      }
	    else if(strcmp(line_words[i], "<") == 0)
	      {
		printf("Found a <");
	      }
	    else
	      {
		cmd[cmdIndex] = line_words[i];
		cmdIndex++;
	      }
	  }
	pipe(p);
	switch(pid = fork())
	  {
	  case -1:
	    syserror("fork failed");
	  case 0:
	    dup2(fd_in, 0);
	    close(p[0]);
	    cmd[cmdIndex] = NULL;
	    execvp(cmd[0], cmd);
	    syserror("could not exec");
	  default:
	    while(wait(NULL) != -1);
	    close(p[1]);
	    fd_in = p[0];
	  }

    }

    return 0;
}


void syserror(const char *s)
{
  extern int errno;

  fprintf (stderr, "%s\n", s);
  fprintf( stderr, " (%s)\n", strerror(errno));
  exit(1);
}
