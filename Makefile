CC := gcc
CFLAGS := -g -std=c23 -Wall -Wextra -Wpedantic -Wvla -Wshadow # -fsanitize=address,undefined

all: fricked_up_curl main.pdf

fricked_up_curl: fricked_up_curl.c
	$(CC) $(CFLAGS) $^ -o $@

main.pdf: main.tex
	lualatex $^

.PHONY: clean
clean:
	rm fricked_up_curl main.pdf
