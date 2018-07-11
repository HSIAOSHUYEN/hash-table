main:
	gcc ./src/hash_table.c ./src/prime.c main.c -std=c99 -o ./build/main
clean:
	rm -rf build/*