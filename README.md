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