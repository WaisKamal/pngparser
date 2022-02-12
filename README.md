# pngparser

A lightweight parser for extracting pixel data from a PNG file.

## Limitations
- Currently only DEFLATE decompression is implemented, with PNG parsing to follow.

## How to use
First build using the following commands:
```
g++ -c bitreader.cpp -o bitreader.o
g++ -c inflater.cpp -o inflater.o
g++ bitreader.o inflater.o main.cpp -o main.exe
```

Next, place an [LZ77-compressed file](https://datatracker.ietf.org/doc/html/draft-deutsch-deflate-spec-00) in the same directory as the application. Name the file **input.dat**.

Finally, run the application. It will output the uncompressed data to a file named **output.dat**.