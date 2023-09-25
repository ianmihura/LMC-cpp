# LMC interpreter (C++)

It is a CLI that can be used to execute LMC code.

### Use

- Compile the C++ source code in the repo (`main.cpp` file) into an `lmc` app
- Execute it:

```bash
# print out help
lmc -h

# interactive CLI
lmc -cli

# -file feeds the file to be run
# -i inputs required by program
lmc -file cpp/mult.lmc -i 5 6 
lmc -file cpp/mult.lmc -i 5 6 -debug
```

### Write your own LMC

You can write your own LMC code in a file and feed it to the tool!

NOTE: only works with pure LMC (no mnemonic or aliasing)

# What is LMC

The Little Man Computer (LMC) is an instructional model of a computer, created by Dr. Stuart Madnick in 1965. The LMC is generally used to teach students, because it models a simple von Neumann architecture computer—which has all of the basic features of a modern computer. It can be programmed in machine code (albeit in decimal rather than binary) or assembly code. [Read more in wikipedia](https://en.wikipedia.org/wiki/Little_man_computer)
