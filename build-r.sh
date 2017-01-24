#!/bin/bash
if [ "$1" = "-R" ]; then
	printf "\nFull rebuild, performing clean...\n\n"
	make -j4 -C bugs/Release clean
else
	printf "\nPass -R to force a full rebuild (clean all first)\n\n"
fi
make -j4 -C bugs/Release all

