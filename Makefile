main:
	gcc -g ./src/hash_table.c ./src/prime.c main.c -std=c11 -o ./build/main -lm
clean:
	rm -rf build/*