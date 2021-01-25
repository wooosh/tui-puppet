#include <string.h>
#include <vterm.h>
#include <stdio.h>
#include <unistd.h>
#include <pty.h>
#include <sys/wait.h>

VTerm *vt;
VTermScreen *vts;
size_t term_w = 132;
size_t term_h = 43;

struct key_pair {
  char* key;
  char* val;
};

struct key_pair keymap[] = {
  {"<enter>", "\r"},
  {"<space>", " "},
  {NULL, NULL}
};

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

  struct winsize winp = {term_h, term_w};
 
  // resize host terminal
  // TODO: flag to show terminal state
  fprintf(stderr, "\x1b[8;%zu;%zut", term_h, term_w);
 
  // master side accepts output from process and provides input to it
  int master, slave;
  openpty(&master, &slave, NULL, NULL, &winp);

  pid_t pid = fork();
  if (pid == 0) {
    close(master);
    // redirect process io to terminal device
    dup2(slave, STDIN_FILENO);
    dup2(slave, STDOUT_FILENO);
    dup2(slave, STDERR_FILENO);
    close(slave);

    argv += 2;
    execvp(argv[0], argv);
  }
  close(slave);

  fd_set rfds;
  char buf[4097];
  ssize_t size;

  while (1) {
    // check if process is still alive
    if (waitpid(pid, NULL, WNOHANG) == pid) {
      // TODO: how to handle early exits?
      break;
    }

    //write(master, "bruh", 4);
    // check if any terminal output is available
    struct timeval tv = {0, 10000};
    FD_ZERO(&rfds);
    FD_SET(master, &rfds);
    if (select(master + 1, &rfds, NULL, NULL, &tv)) {
      size = read(master, buf, 4096);
      vterm_input_write(vt, buf, size);
      
      buf[size] = '\0';
      fprintf(stderr, buf);
      fflush(stderr);
      
      if (vterm_output_get_buffer_current(vt) > 0) {
        size = vterm_output_read(vt, buf, 4096);
        write(master, buf, size);
      }
    }

    // handle command
    char* line = NULL;
    size_t len = 0;
    // TODO: handle getline returning error
    if (getline(&line, &len, stream) == -1) continue;

    char* cmd = strtok(line, " \n");
    if (strcmp(cmd, "keystream") == 0) {
      char* keyname;
      while (1) {
        keyname = strtok(NULL, " \n");
        if (keyname == NULL) break;

        // translate key if applicable
        struct key_pair *kp = keymap;
        for (struct key_pair *kp = keymap; kp->key != NULL; kp++) {
          if (strcmp(kp->key, keyname) == 0) {
            write(master, kp->val, strlen(kp->val));
            goto found_key;
          }
        }
        // if key not in keymap
        write(master, keyname, strlen(keyname));
        found_key:;
      }
    }
  }

  close(master);

  // cleanup
  fclose(stream);
  vterm_free(vt);

  return 0;
}
