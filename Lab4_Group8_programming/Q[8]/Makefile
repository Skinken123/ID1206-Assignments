q8: q8.c
	gcc q8.c -o q8

run: q8
	 dd if=/dev/zero of=file_to_map.txt bs=1M count=1
	 ./q8