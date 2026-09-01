#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "dsa.h"

typedef struct HuffmanHeap HuffmanHeap;

typedef struct HuffmanNode {
    uint8_t             character;
    uint64_t            frequency;
    struct HuffmanNode* left;
    struct HuffmanNode* right;
} HuffmanNode;

HuffmanNode*    buildHuffmanTree    (HuffmanHeap *heap);
HuffmanNode*    createHuffmanNode   (uint8_t character, uint64_t frequency);
int             compareHuffman      (HuffmanNode *const *a, HuffmanNode *const *b);
bool            isLeaf              (HuffmanNode *node);
size_t          getTreeSize         (HuffmanNode *root);      // gets the length of the serializeTree string
bool            generateCodes       (HuffmanNode *node, char *code, char *codes[BYTE_COUNT], size_t depth);
bool            serializeTree       (HuffmanNode *root, FILE *output);
HuffmanNode*    deserializeTree     (FILE *input);
void            freeHuffmanTree     (HuffmanNode *node);
void            freeCodes           (char *codes[BYTE_COUNT]);
bool            huffmanCompress     (uint64_t frequency[], char *codes[BYTE_COUNT], const char *inputFilePath, int *count, int *mx, HuffmanNode **node);
bool            huffmanDecompress   (const char *inputFilePath);

#endif // HUFFMAN_H






