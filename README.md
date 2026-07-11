# Lossless File Compression Engine

A standalone Command-Line Interface (CLI) utility built in standard C++ that compresses text and raw image (BMP) files. It uses a custom implementation of a Min-Heap and Huffman Tree to dynamically generate variable-length binary codes, manipulating data at the hardware bit-level.

## Features
* **Universal File Support:** Reads and processes raw bytes, allowing it to compress both text and uncompressed images.
* **Lossless Decompression:** Writes custom binary headers storing the frequency map, ensuring files are restored to their exact original state across different sessions.
* **High Performance:** Utilizes bitwise operations (`<<`, `|`, `&`) to pack generated string codes into tightly compressed 8-bit hardware chunks.

## Technologies Used
* **Language:** C++
* **Data Structures:** Min-Heap (Priority Queue), Binary Tree, Hash Maps
* **Concepts:** Greedy Algorithms (Huffman Coding), Bit Manipulation, Binary File I/O

## How to Run
Compile the code using a standard C++ compiler:
`g++ main2.cpp -o huffman`

**To Compress a file:**
`.\huffman compress input.bmp compressed.bin`

**To Decompress a file:**
`.\huffman decompress compressed.bin restored.bmp`
