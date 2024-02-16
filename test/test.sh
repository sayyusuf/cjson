#!/bin/bash

gcc     -Wall -Wextra -Werror -D DEBUG_MODE  -fsanitize=address -g -I../ test-buff.c  ../cjson.c && ./a.out $1 $2
gcc -O2 -Wall -Wextra -Werror -D DEBUG_MODE  -fsanitize=address -g -I../ test-ptr.c   ../cjson.c && ./a.out $1 $2
gcc -O3 -Wall -Wextra -Werror -D DEBUG_MODE -fsanitize=address -g -I../ test-alloc.c ../cjson.c && ./a.out $1 $2

