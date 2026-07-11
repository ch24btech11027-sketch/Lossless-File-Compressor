#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <queue>   
#include <vector> 

// Include your Node structure from Phase 1
#include "Node.h"

// 1. Custom comparator for the Min-Heap
struct Compare {
    bool operator()(Node* left, Node* right) {
        // Puts nodes with the smallest frequencies at the top
        return left->freq > right->freq; 
    }
};

// 2. Recursive DFS to generate the binary codes
void generateCodes(Node* root, std::string currentCode, std::unordered_map<unsigned char, std::string>& huffmanCodes) {
    if (root == nullptr) {
        return;
    }

    // If it's a leaf node, we found a byte! Save its code.
    if (root->left == nullptr && root->right == nullptr) {
        huffmanCodes[root->data] = currentCode;
        return;
    }

    // Traverse left (append '0') and right (append '1')
    generateCodes(root->left, currentCode + "0", huffmanCodes);
    generateCodes(root->right, currentCode + "1", huffmanCodes);
}

int main(int argc, char* argv[]) {
    // 1. Check if the user typed the correct number of commands
    // We expect 4 words: ./main [compress/decompress] [input_file] [output_file]
    if (argc != 4) {
        std::cerr << "Usage Instructions:\n";
        std::cerr << "  To Compress:   .\\main compress <input_file> <compressed_file.bin>\n";
        std::cerr << "  To Decompress: .\\main decompress <compressed_file.bin> <output_file.bmp>\n";
        return 1;
    }

    // 2. Read the arguments from the terminal
    std::string command = argv[1];
    std::string inputFilename = argv[2];
    std::string outputFilename = argv[3];

    // 3. Route to the correct engine
    if (command == "compress") {
        std::cout << "--- COMPRESSION MODE ---\n";
        
        // 1. Open the file provided in the terminal
        std::ifstream inputFile(inputFilename, std::ios::binary);

        // Safety check: Did the file actually open?
        if (!inputFile.is_open()) {
            std::cerr << "Error: Could not find or open " << inputFilename << "!\n";
            return 1;
        }

        std::cout << "Successfully opened " << inputFilename << " in binary mode.\n";

        // 3 & 4. Create the map and read the file byte-by-byte
        std::unordered_map<unsigned char, int> freqMap;
        char ch;

        // inputFile.get(ch) grabs exactly one raw byte at a time
        while (inputFile.get(ch)) {
            unsigned char byteValue = static_cast<unsigned char>(ch);
            freqMap[byteValue]++;
        }

        inputFile.close();
        
        std::cout << "\n--- Phase 2 Complete ---\n";
        std::cout << "Total unique bytes found: " << freqMap.size() << "\n";

        // --- Phase 3: Build the Huffman Tree ---
        std::cout << "Building the Huffman Tree...\n";

        std::priority_queue<Node*, std::vector<Node*>, Compare> minHeap;
        for (auto pair : freqMap) {
            minHeap.push(new Node(pair.first, pair.second));
        }

        while (minHeap.size() > 1) {
            Node* leftChild = minHeap.top(); minHeap.pop();
            Node* rightChild = minHeap.top(); minHeap.pop();

            Node* parent = new Node(0, leftChild->freq + rightChild->freq);
            parent->left = leftChild;
            parent->right = rightChild;

            minHeap.push(parent);
        }

        Node* root = minHeap.top();

        std::unordered_map<unsigned char, std::string> huffmanCodes;
        generateCodes(root, "", huffmanCodes);

        std::cout << "--- Phase 3 Complete ---\n";
        std::cout << "Total bytes processed (Root Frequency): " << root->freq << "\n";
        std::cout << "Generated " << huffmanCodes.size() << " unique Huffman codes.\n";

        // --- Phase 4: Write Header and Pack Bits ---
        std::cout << "Starting compression...\n";

        // 1. Open the output file using the variable from the terminal
        std::ofstream outFile(outputFilename, std::ios::binary);

        // PART A: Write the Header (The Blueprint)
        int mapSize = freqMap.size();
        outFile.write(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

        for (auto pair : freqMap) {
            unsigned char chPair = pair.first;
            int freq = pair.second;
            outFile.write(reinterpret_cast<char*>(&chPair), sizeof(chPair));
            outFile.write(reinterpret_cast<char*>(&freq), sizeof(freq));
        }

        // PART B: The Bit Packing Engine
        // Re-open the input file
        std::ifstream inFile(inputFilename, std::ios::binary);

        unsigned char bitBuffer = 0; 
        int bitCount = 0;            
        char readCh;

        while (inFile.get(readCh)) {
            unsigned char byteValue = static_cast<unsigned char>(readCh);
            std::string code = huffmanCodes[byteValue];
            
            for (char bit : code) {
                bitBuffer = bitBuffer << 1; 
                
                if (bit == '1') {
                    bitBuffer = bitBuffer | 1; 
                }
                
                bitCount++;
                
                if (bitCount == 8) {
                    outFile.write(reinterpret_cast<char*>(&bitBuffer), sizeof(bitBuffer));
                    bitBuffer = 0; 
                    bitCount = 0;  
                }
            }
        }

        if (bitCount > 0) {
            bitBuffer = bitBuffer << (8 - bitCount);
            outFile.write(reinterpret_cast<char*>(&bitBuffer), sizeof(bitBuffer));
        }

        inFile.close();
        outFile.close();

        std::cout << "--- Phase 4 Complete ---\n";
        std::cout << "File successfully compressed and saved as '" << outputFilename << "'.\n";
    }

    else if (command == "decompress") {
        std::cout << "--- DECOMPRESSION MODE ---\n";
        std::cout << "Starting decompression...\n";

        // 1. Open the compressed file for reading using the terminal variable
        std::ifstream compressedIn(inputFilename, std::ios::binary);

        // Safety check: Did the file actually open?
        if (!compressedIn.is_open()) {
            std::cerr << "Error: Could not find or open " << inputFilename << "!\n";
            return 1;
        }

        // Open the output file using the terminal variable
        std::ofstream decompressedOut(outputFilename, std::ios::binary);

        // ==========================================
        // PART A: Read Header & Rebuild Tree
        // ==========================================
        int recoveredMapSize;
        // Read the first 4 bytes (the integer representing map size)
        compressedIn.read(reinterpret_cast<char*>(&recoveredMapSize), sizeof(recoveredMapSize));

        std::unordered_map<unsigned char, int> recoveredMap;

        // Read the exact number of character/frequency pairs we wrote
        for (int i = 0; i < recoveredMapSize; i++) {
            unsigned char ch;
            int freq;
            compressedIn.read(reinterpret_cast<char*>(&ch), sizeof(ch));
            compressedIn.read(reinterpret_cast<char*>(&freq), sizeof(freq));
            
            recoveredMap[ch] = freq;
        }

        // Rebuild the Min-Heap
        std::priority_queue<Node*, std::vector<Node*>, Compare> recoveredHeap;
        for (auto pair : recoveredMap) {
            recoveredHeap.push(new Node(pair.first, pair.second));
        }

        // Rebuild the Huffman Tree
        while (recoveredHeap.size() > 1) {
            Node* leftChild = recoveredHeap.top(); recoveredHeap.pop();
            Node* rightChild = recoveredHeap.top(); recoveredHeap.pop();

            Node* parent = new Node(0, leftChild->freq + rightChild->freq);
            parent->left = leftChild;
            parent->right = rightChild;
            recoveredHeap.push(parent);
        }

        // This is the reconstructed root, identical to the one in Phase 3!
        Node* recoveredRoot = recoveredHeap.top();

        // ==========================================
        // PART B: Decode the Bits
        // ==========================================
        int totalBytesToRecover = recoveredRoot->freq;
        int bytesRecovered = 0;
        
        Node* curr = recoveredRoot;
        unsigned char byteBuffer;

        // Read the remaining file one byte at a time
        while (compressedIn.read(reinterpret_cast<char*>(&byteBuffer), sizeof(byteBuffer)) && bytesRecovered < totalBytesToRecover) {
            
            // Loop through the 8 bits from left (most significant) to right
            for (int i = 7; i >= 0; i--) {
                // Isolate the current bit
                bool bit = (byteBuffer >> i) & 1;

                // Navigate the tree
                if (bit == 0) curr = curr->left;
                else curr = curr->right;

                // If we hit a leaf node (no children), we found our byte!
                if (curr->left == nullptr && curr->right == nullptr) {
                    // Write the raw byte to the new file
                    decompressedOut.write(reinterpret_cast<char*>(&curr->data), sizeof(curr->data));
                    bytesRecovered++;
                    
                    // Jump back to the top of the tree for the next bit
                    curr = recoveredRoot; 

                    // Safety Stop: Break instantly if we recovered all original bytes
                    if (bytesRecovered == totalBytesToRecover) {
                        break;
                    }
                }
            }
        }

        compressedIn.close();
        decompressedOut.close();

        std::cout << "--- Phase 5 Complete ---\n";
        // Dynamically print the output file name
        std::cout << "Decompression finished! File restored as '" << outputFilename << "'.\n";
    }
    else {
        std::cerr << "Error: Unknown command '" << command << "'.\n";
        std::cerr << "Please use 'compress' or 'decompress'.\n";
        return 1;
    }

    return 0;
}