# pngparser

A lightweight parser for extracting pixel data from a PNG file.

## Current Limitations
- Only images with color type truecolor with alpha are supported
- Interlacing is not supported
- If an unsupported image is supplied, the behaviour of the application is undefined


## How to use
First build using the following commands:
```
g++ -c BitReader.cpp -o Iitreader.o
g++ -c Inflater.cpp -o Inflater.o
g++ -c PNGParser.cpp -o PNGParser.o
g++ BitReader.o Inflater.o PNGParser.o main.cpp -o main.exe
```

Next, place an PNG image in the same directory as the application.

Finally, run the application using the following commands (replace `<imageFile>` and `<outputFile>` with the names of the image file and the output file respectively):
```
.\main <imageFile> <outputFile>
```

## Output Format
The first line of the output contains three space-separated integers: the image width, height, and bytes per pixel. The number of bytes per pixel is currently always 4 (since only truecolor with alpha images are supported).

Each of the following lines represents a row of pixels. Each line contains $4 × width$ space-separated integers. Every 4 integers denote a pixel in the format `red green blue alpha`.