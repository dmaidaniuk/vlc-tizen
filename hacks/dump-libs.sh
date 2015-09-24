#!/bin/sh

for i in *.so; do
    nm -D --defined-only $i | awk '{print $3}' \
        | grep -vE "^(__aeabi|__bss_start|_init|_fini|_end)" > $i.symbols
done
