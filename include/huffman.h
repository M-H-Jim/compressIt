#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "dsa.h"


typedef struct HuffmanNode {
    uint8_t character;
    uint64_t frequency;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;

HuffmanNode* createHuffmanNode (uint8_t character, uint64_t frequency);



int compareHuffman (HuffmanNode *const *a, HuffmanNode *const *b);


typedef struct HuffmanHeap HuffmanHeap;

HuffmanNode* buildHuffmanTree (HuffmanHeap *heap);

bool isLeaf(HuffmanNode *node);

size_t getTreeSize(HuffmanNode *root);

void generateCodes (HuffmanNode *node, char *code, char *codes[BYTE_COUNT], size_t depth);

void freeHuffmanTree(HuffmanNode *node);

void freeCodes(char *codes[BYTE_COUNT]);


bool huffmanCompress(uint64_t frequency[], char *codes[BYTE_COUNT], const char *inputFilePath, int *count, int *mx);

bool serializeTree(HuffmanNode *root, FILE *output);

HuffmanNode* deserializeTree(FILE *input);

bool huffmanDecompress(const char *inputFilePath);

#endif // HUFFMAN_H






