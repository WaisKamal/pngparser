# pngparser

A lightweight parser for extracting pixel data from a PNG file. Written completely from scratch.

## Current Limitations
- Only images with color types truecolor and truecolor with alpha are supported
- Interlacing is not supported
- Verification on ADLER32 checksum and cyclic redundancy code (CRC) is not yet implemented
- If an unsupported image is supplied, the behaviour of the application is undefined


## How to use
Make sure you have GCC version 8.0 or newer installed.

First build using the following commands:

**Windows**
```
g++ -c BitReader.cpp -o BitReader.o
g++ -c Inflater.cpp -o Inflater.o
g++ -c PNGParser.cpp -o PNGParser.o
g++ BitReader.o Inflater.o PNGParser.o main.cpp -o main.exe
```

**Linux/MacOS**
```
g++ -c BitReader.cpp -o BitReader.o
g++ -c Inflater.cpp -o Inflater.o
g++ -c PNGParser.cpp -o PNGParser.o
g++ BitReader.o Inflater.o PNGParser.o main.cpp -o main
```

Then run the application using the following command (replace `<imageFile>` and `<outputFile>` with the paths to the image file and the output file respectively):
```
.\main <imageFile> <outputFile>
```

## Output Format
The first line of the output contains three space-separated integers: the image `width`, `height`, and number of values per pixel.

Each of the following `height` lines represents a row of pixels. Each line contains `width × values_per_pixel` space-separated integers. The pixels are denoted as follows:

- for truecolor images, each pixel is represented by 3 space-separated integers denoting `red`, `green`, and `blue` respectively
- for truecolor images with an alpha channel, each pixel is represented by 4 space-separated integers denoting `red`, `green`, `blue`, and `alpha` respectively
