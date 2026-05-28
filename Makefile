CC := gcc
CFLAGS := -g -std=c17 -Wall -Wextra -Wpedantic -Wvla -Wshadow # -fsanitize=address,undefined -fno-sanitize-recover=all

fricked_up_curl: fricked_up_curl.c
	$(CC) $(CFLAGS) $^ -o $@

main.pdf: main.tex
	lualatex $^

afl.pdf: afl.md
	pandoc -i $^ -o $@

.PHONY: clean
clean:
	rm -f fricked_up_curl main.pdf main.aux main.log main.nav main.out main.snm main.toc main.vrb afl.pdf
