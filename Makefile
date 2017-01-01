build: main.c bmp_header.h
	gcc -Wall main.c -o paint
run: paint
	./paint
clean:
	rm -rf paint
