all: 
	clang semantic.c parser.c scanner.c tests/test_utils.c -o cy
	./cy

clean: 
	rm -f cy

debug: all
	@echo "\n\n\n\n\n\n\n"

