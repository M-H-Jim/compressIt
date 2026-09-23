# compressIt

A Huffman-based file compressor and decompressor written in pure C, with a GUI that visualizes the compression process.

![compressIt demo](gifs/compressIt.gif)


## What it does

- Compresses files using **Huffman encoding**
- Decompresses files compressed by compressIt
- Shows compression statistics
- Displays byte frequencies as a bar chart
- Shows the binary code assigned to each byte
- Renders a visual **Huffman tree**

The main purpose of the project is to make Huffman coding easier to understand by showing what's happening during compression.

## How to Run (Windows)

1. Download the latest release
2. Extract the zip file
3. Run `compressIt.exe`

## Tech Used

- **C** — main implementation
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) — GUI
- [GLFW](https://www.glfw.org/) + [GLAD](https://glad.dav1d.de/) + [OpenGL 3.3](https://www.opengl.org/) — backend for Nuklear
- [CMake](https://cmake.org/) + [Ninja](https://ninja-build.org/)— build system
- [Geany](https://www.geany.org/) — development environment / editor


## Building from Source

### Requirements

- **Windows**
- A **C compiler** with C11 support
- **CMake 4.0 or newer + Ninja**
- **OpenGL 3.3** support
- **Nuklear UI**
- **GLFW 3**
- **GLAD**
- The required GLFW and GLAD libraries and headers

### Build Steps
Clone the repository:

```bash
Note: The tree visualization lives on the TreeVisualization branch — make sure you check it out before building.

# Clone the repository and switch to the TreeVisualization branch:

git clone https://github.com/M-H-Jim/compressIt.git
cd compressIt
git checkout TreeVisualization

# Alternatively, clone the branch directly:

git clone -b TreeVisualization https://github.com/M-H-Jim/compressIt.git
cd compressIt

# Configure and build the project:

cmake -G Ninja -B build
cmake --build build --config Release

Run the executable from the build folder, e.g. build\Release\compressIt.exe (the exact location depends on your generator).
```


## Limitations

This is mainly an educational project, not a production-ready compression tool.

- Tested with files around **300 MB**
- Large files can consume a lot of memory
- Very large files may cause crashes
- Compression/decompression isn't heavily optimized
- Currently uses only **Huffman coding**
