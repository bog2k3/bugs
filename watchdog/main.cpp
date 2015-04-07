#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

#include "../bugs/utils/log.h"

extern char **environ;

bool spawn(int &ret) {
	pid_t pid;
	LOGLN("spawning new bugs process...")
	char *argv[] = { "bugs-debug", "--load", "autosave.bin", (char *) 0 };
	if (posix_spawn(&pid, "bugs-debug", NULL, NULL, argv, environ) == 0) {
		LOGLN("bugs PID: " << pid);
		waitpid(pid, &ret, 0);
		return true;
	} else {
		LOGLN("posix_spawn error: " << strerror(ret));
		return false;
	}
}

int main() {
	LOGGER("watchdog");

	int ret = -1;
	do {
		if (spawn(ret)) {
			if (ret != 0) {
				LOGLN("bugs crashed with code : " << ret);
				LOGLN("restarting process after 5 seconds...");
				sleep(5);
			}
		} else {
			LOGLN("failed to spawn bugs process. retrying after 5 seconds...");
			sleep(5);
		}
	} while (ret != 0);
	LOGLN("bugs exited normally. stopping watchdog.");
	return 0;
}
