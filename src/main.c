#include <vterm.h>
#include <stdio.h>
#include <unistd.h>
#include <pty.h>
#include <sys/wait.h>

VTerm *vt;
VTermScreen *vts;
size_t term_w = 50;
size_t term_h = 50;

void command_handler(FILE *stream) {
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, stream)) != -1) {
    printf("line: %s", line);
  }

  free(line);
}

int master;

int main(int argc, char** argv) {
  // TODO: arguments to executable
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <commandfile> <executable> <arguments to executable>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE* stream = fopen(argv[1], "r");
  if (stream == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  // vterm initialization
  vt = vterm_new(term_h, term_w);
  vterm_set_utf8(vt, 1);

  vts = vterm_obtain_screen(vt);
  vterm_screen_reset(vts, 1);

  //command_handler(stream);


  struct winsize winp = {term_h, term_w};
  
  // master side accepts output from process and provides input to it
  int slave;
  openpty(&master, &slave, NULL, NULL, &winp);


 
  pid_t pid = fork();
  if (pid == 0) {
    close(master);
    // redirect process io to terminal device
    dup2(slave, STDIN_FILENO);
    dup2(slave, STDOUT_FILENO);
    close(slave);

    argv += 2;
    execvp(argv[0], argv);
  }
  close(slave);

  fd_set rfds;
  struct timeval tv = {0, 0};
  char buf[4097];
  ssize_t size;
  size_t count = 0;

  while (1) {
    // handle command
    

    // check if process is still alive
    if (waitpid(pid, NULL, WNOHANG) == pid) {
      // TODO: how to handle early exits?
      break;
    }

    // check if any terminal output is available
    FD_ZERO(&rfds);
    FD_SET(master, &rfds);
    if (select(master + 1, &rfds, NULL, NULL, &tv)) {
      size = read(master, buf, 4096);
      vterm_input_write(vt, buf, size);
      
      buf[size] = '\0';
      fprintf(stderr, buf);
      fflush(stderr);
      count += size;
      
      if (vterm_output_get_buffer_current(vt) > 0) {
        size = vterm_output_read(vt, buf, 4096);
        write(master, buf, size);
      }
    }
  }

  close(master);
  close(slave);

  /*
  vterm_input_write(vt, "hello \x1b[31mworld", 17); 

  VTermScreenCell cell;
  VTermPos pos = { .row = 0, .col = 6 };
  vterm_screen_get_cell(vts, pos, &cell);

  printf("%c\n", cell.chars[0]);
  */

  // cleanup
  fclose(stream);
  vterm_free(vt);

  return 0;
}
