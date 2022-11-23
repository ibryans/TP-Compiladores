run:

	@g++ compilador.cpp 
	@./a.out < 1.in
	
	@echo
	@echo

	@nasm codigo.asm -g -w-zeroing -f elf64 -o arq.o 2> /dev/null

	@echo

	@ld arq.o -o arq
	@./arq
