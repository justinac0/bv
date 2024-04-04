all:
	gcc -W -Wextra -pedantic -std=c99 source/*.c -o build/bv -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

run:
	./build/bv
