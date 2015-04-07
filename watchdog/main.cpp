void test_posix_spawn(void) {
  pid_t pid;
  char *argv[] = {"ls", (char *) 0};
  int status;
  puts("Testing posix_spawn");
  fflush(NULL);
  status = posix_spawn(&pid, "/bin/ls", NULL, NULL, argv, environ);
  if (status == 0) {
    printf("Child id: %i\n", pid);
    fflush(NULL);
    if (waitpid(pid, &status, 0) != -1) {
      printf("Child exited with status %i\n", status);
    } else {
      perror("waitpid");
    }
  } else {
    printf("posix_spawn: %s\n", strerror(status));
  }
}