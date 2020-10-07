# SMIPS -> A MIPS Compiler in C
This project compiles and translates pre-processed MIPS code (hexcode), which executes the program in the shell.

The source code does this by storing the hexcodes into an abstract datatype which allows for tracking the registers used in memory and translating the commands given in the spec into the C equivalents.

## How to run the code
Compile smips.c onto your local machine and then run it like this in shell.

<pre><code>./smips $(test_file)
</code></pre> 
