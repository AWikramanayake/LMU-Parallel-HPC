# run `make -B` to build the benchmark application,
# then `./benchmark-double-{add|div|mul}` to run it.
# `make clean` removes the binary executables

# set the C-compiler
CC=gcc

# set the compiler flags
# -fno-tree-vectorize to avoid vector instructions
# -g to add debug information to the binary executable
CFLAGS=-O2 -fno-tree-vectorize -g

.DEFAULT: all
all:    benchmark-double-add \
		benchmark-double-div \
		benchmark-double-mul

benchmark-double-add : benchmark.c
	$(CC) -o $@ $< $(CFLAGS) -D_OP_=+ -D_OP_STRING_='"add"'

benchmark-double-div : benchmark.c
	$(CC) -o $@ $< $(CFLAGS) -D_OP_=/ -D_OP_STRING_='"div"'

benchmark-double-mul : benchmark.c
	$(CC) -o $@ $< $(CFLAGS) -D_OP_=* -D_OP_STRING_='"mul"'

run: all
	echo "op,scenario,num op,result,nseconds" > measurements.csv
	./benchmark-double-add                   >> measurements.csv
	./benchmark-double-div                   >> measurements.csv
	./benchmark-double-mul                   >> measurements.csv
	cat measurements.csv

.PHONY: clean
clean:
	-rm -f benchmark-double-*
	-rm -f *.csv
