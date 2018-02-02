#!/bin/bash
printf "\n Building Debug...\n\n"

if [ ! -f .boglfw_path ]; then
    printf "\n ERROR: file .boglfw_path doesn't exist \n\n"
    printf "\n Enter the path to boglfw distribution: \n"
    read THE_PATH
    printf "$THE_PATH" > ".boglfw_path"
fi
BOGLFW_PATH=`cat .boglfw_path`

mkdir -p build/Debug
cd build/Debug
cmake -DCMAKE_BUILD_TYPE=Debug -DBOGLFW_DIST_DIR="$BOGLFW_PATH" ../..
if [ "$1" = "-R" ]; then
	printf "\nFull rebuild, performing clean...\n\n"
	make clean
else
	printf "\nPass -R to force a full rebuild (clean all first)\n\n"
fi

make -j4

RESULT=$?

cd ../..

if [ $RESULT -eq 0 ]; then
	printf "\n Done. \n\n"
	exit 0
else
	printf "\n Errors encountered. \n\n"
	exit 1
fi

