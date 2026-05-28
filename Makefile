CC := gcc
CFLAGS := -g -std=c23 -Wall -Wextra -Wpedantic -Wvla -Wshadow # -fsanitize=address,undefined

fricked_up_curl: fricked_up_curl.c
	$(CC) $(CFLAGS) $^ -o $@

main.pdf: main.tex
	lualatex $^

.PHONY: clean
clean:
	rm -f fricked_up_curl main.pdf main.aux main.log main.nav main.out main.snm main.toc main.vrb
