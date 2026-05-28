---
geometry: margin=1in
---

# Using AFL++

Now, it's time to use a real fuzzer (AFL++) to find bugs in `fricked_up_curl`.

# Setting up `fricked_up_curl` to be fuzzed with AFL++

1. On plink, `cd` into your clone of `fricked_up_curl`
```sh
cd fricked_up_curl
```
2. Make 2 directories, `in` and `out`, to store the initial corpus and the fuzzer's results.
```sh
mkdir in out
```
3. Put a URL into the starting corpus.
```sh
printf 'http://example.com:80/path/to/whatever' > in/seed
```
4. Modify the `Makefile` to change the value of the `CC` variable (the C compiler) from `clang` to `afl-cc`. This is a special C compiler that instruments the compiled program to automatically collect control-flow edge coverage.
5. Modify the `Makefile` to add `-fsanitize=address,undefined -fno-sanitize-recover=all` to the end of `CFLAGS`. These are compiler flags that make some errors that would otherwise pass silently crash the program instead.
6. Rebuild.
```sh
make clean && make
```
7. Start the fuzzer!
```sh
afl-fuzz -i in -o out -- ./fricked_up_curl @@
```
8. After the fuzzer finds a few crashes, interrupt it with Ctrl-C, and look at what it found (check `out/default/crashes`).
9. What bugs it find? What bugs didn't it find? Why do you think this is? Discuss this with your group, then call over course staff and tell us what you think.
