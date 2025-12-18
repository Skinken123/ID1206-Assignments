# Instructions for compiling and running our c program

1. We use gcc compiler to compile our c programs so make sure you have it: gcc --version
2. Use command "dd if=/dev/zero of=file_to_map.txt bs=1M count=1" to create a 1 MB file
3. Compile: gcc q8.c -o q8
4. Run: ./q8 

Run all with: make run