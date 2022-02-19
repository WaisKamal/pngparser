# pngparser

A lightweight parser for extracting pixel data from a PNG file.

## Current Limitations
- Only images with color types truecolor and truecolor with alpha are supported
- Interlacing is not supported
- Verification on ADLER32 checksum and cyclic redundancy code (CRC) is not yet implemented
- If an unsupported image is supplied, the behaviour of the application is undefined


## How to use
First build using the following commands:
```
g++ -c BitReader.cpp -o Bitreader.o
g++ -c Inflater.cpp -o Inflater.o
g++ -c PNGParser.cpp -o PNGParser.o
g++ BitReader.o Inflater.o PNGParser.o main.cpp -o main.exe
```

Next, place a PNG image in the same directory as the application.

Finally, run the application using the following commands (replace `<imageFile>` and `<outputFile>` with the names of the image file and the output file respectively):
```
.\main <imageFile> <outputFile>
```

## Output Format
The first line of the output contains three space-separated integers: the image `width`, `height`, and number of values per pixel.

Each of the following $height$ lines represents a row of pixels. Each line contains `width × values_per_pixel` space-separated integers. The pixels are denoted as follows:

- for truecolor images, each pixel is represented by 3 space-separated integers denoting `red`, `green`, and `blue` respectively
- for truecolor images with an alpha channel, each pixel is represented by 4 space-separated integers denoting `red`, `green`, `blue`, and `alpha` respectively
