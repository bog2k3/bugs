#!/bin/bash
g++ -std=c++14 -o watchdog main.cpp -I ../../boglfw/build/dist/include/ -L ../../boglfw/build/dist/lib/ -lboglfw
