#ifndef NODE_H
#define NODE_H

struct Node {
    unsigned char data; // Holds the character or image byte (0-255)
    int freq;           // How many times this byte appears
    Node* left;         // Left branch of the tree
    Node* right;        // Right branch of the tree

    // Constructor to easily create new nodes
    Node(unsigned char d, int f) {
        data = d;
        freq = f;
        left = nullptr;
        right = nullptr;
    }
};

#endif