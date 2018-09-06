#!/bin/bash

gcc -l stdc++ -lpthread utility.cpp parse.cpp hanghttp.cpp -I ./ -o test
echo "gcc over"
./test http.config
