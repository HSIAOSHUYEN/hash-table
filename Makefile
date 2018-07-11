main:
	gcc ./src/hash_table.c ./src/prime.c main.c -o ./build/main
clean:
	rm -rf build/*