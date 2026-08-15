#include "huffman.h"

DEFINE_HEAP(HuffmanNode*, HuffmanHeap, compareHuffman)

HuffmanNode* createHuffmanNode (uint8_t character, uint64_t frequency) {
    HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));
    if (!node) return NULL;
    node->character = character;
    node->frequency = frequency;
    node->left = NULL;
    node->right = NULL;
    return node;
}

int compareHuffman (HuffmanNode *const *a, HuffmanNode *const *b) {
    if ((*a)->frequency < (*b)->frequency) return 1;
    if ((*a)->frequency > (*b)->frequency) return -1;
    return 0;
}

HuffmanNode* buildHuffmanTree (HuffmanHeap *heap) {
    if (!heap || sizeHuffmanHeap(heap) == 0) return NULL;
    while (sizeHuffmanHeap(heap) > 1) {
        HuffmanNode *left;
        HuffmanNode *right;
        
        if (!popHuffmanHeap(heap, &left))  return NULL;
        if (!popHuffmanHeap(heap, &right)) return NULL;
        
        HuffmanNode *parent = createHuffmanNode(0, left->frequency + right->frequency);
        if (!parent) return NULL;
        
        parent->left = left;
        parent->right = right;
        
        if(!pushHuffmanHeap(heap, parent)) return NULL;
    }
    HuffmanNode *root = peekHuffmanHeap(heap);
    //~ popHuffmanHeap(heap, &root);
    
    return root;
}

void generateCodes (HuffmanNode *node, char *code, char *codes[256], size_t depth) {
    if (!node) return;
    if (node->left == NULL && node->right == NULL) {
        if (depth == 0) {
            code[0] = '0';
            code[1] = '\0';
        }
        else code[depth] = '\0';
        
        
        codes[node->character] = malloc(strlen(code) + 1);
        if (!codes[node->character]) {
            fprintf(stderr, "malloc failed\n");
            exit(EXIT_FAILURE);
        }
        
        strcpy((char *)codes[node->character], (const char *)code);
        
        printf("%c -> %s\n", node->character, codes[node->character]);/////////
        return;
    }
    
    code[depth] = '0';
    generateCodes(node->left, code, codes, depth + 1);
    
    code[depth] = '1';
    generateCodes(node->right, code, codes, depth + 1);
    
}

bool isLeaf(HuffmanNode *node) {
    return node && node->left == NULL && node->right == NULL;
}

void freeHuffmanTree(HuffmanNode *node) {
    if (!node) return;
    freeHuffmanTree(node->left);
    freeHuffmanTree(node->right);
    free(node);
}


void freeCodes(char *codes[BYTE_COUNT]) {
    for (uint16_t i = 0; i < BYTE_COUNT; i++) {
        free(codes[i]);
        codes[i] = NULL;
    }
}

size_t getTreeSize(HuffmanNode *root) { // gets the length of the serializeTree string
    if (!root) return 0;
    if(isLeaf(root)) return 2;
    return 1 + getTreeSize(root->left) + getTreeSize(root->right);
}

bool serializeTree(HuffmanNode *root, FILE *output) {
    if (!root) return false;
    if (isLeaf(root)) {
        fputc(1, output);
        fputc(root->character, output);
    }
    else {
        fputc(0, output);
        if (!serializeTree(root->left, output)) return false;
        if (!serializeTree(root->right, output)) return false;
    }
    return true;
}

HuffmanNode* deserializeTree(FILE *input) {
    if (!input) return NULL;
    
    int marker = fgetc(input);
    if (marker == EOF) return NULL;
    
    if (marker == 1) {
        int byte = fgetc(input);
        if (byte == EOF) return NULL;
        
        HuffmanNode *node = createHuffmanNode((uint8_t)byte, 0);
        if (!node) return NULL;
        return node;
    }
    if (marker == 0) {
        HuffmanNode *node = createHuffmanNode(0, 0);
        if (!node) return NULL;
        
        node->left = deserializeTree(input);
        if (!node->left) {
            freeHuffmanTree(node);
            return NULL;
        }
        node->right = deserializeTree(input);
        if (!node->right) {
            freeHuffmanTree(node);
            return NULL;
        }
        return node;
    }
    return NULL;
}




bool huffmanCompress(uint64_t frequency[], char *codes[256], const char *inputFilePath, int *count, int *mx) {
    
    char outputFilePath[PATH_MAX];
    snprintf(outputFilePath, sizeof(outputFilePath), "%s.huff", inputFilePath);
    
    
    FILE *input = fopen(inputFilePath, "rb");
    if (!input) return perror("Error opening input file"), false;
    
    
    uint64_t originalSize = 0;
    int byte;
    while((byte = fgetc(input)) != EOF) {
        frequency[byte]++;
        originalSize++;
    }
    
    rewind(input);
    
    HuffmanHeap *heap = createHuffmanHeap();
    if (!heap) return fclose(input), false;
    
    for (uint16_t i = 0; i < BYTE_COUNT; i++) {
        if (frequency[i] == 0) continue;
        
        (*count)++;
        *mx = MAX(*mx, frequency[i]);
        
        HuffmanNode *node = createHuffmanNode((uint8_t)i, frequency[i]);
        if (!node || !pushHuffmanHeap(heap, node)) {
            freeHuffmanHeap(heap);
            fclose(input);
            return false;
        }
    }
    
    HuffmanNode *root = buildHuffmanTree(heap);
    if (!root) {
        freeHuffmanHeap(heap);
        fclose(input);
        return false;
    }
    
    
    char code[256];
    generateCodes(root, code, codes, 0);
    
    
    FILE *output = fopen(outputFilePath, "wb");
    if (!output) {
        perror("Error opening output file");
        freeHuffmanTree(root);
        freeHuffmanHeap(heap);
        fclose(input);
        freeCodes(codes);
        return 1;
    }
    
    size_t treeSize = getTreeSize(root);
    
    fwrite("HUFF", 1, 4, output);
    fwrite(&originalSize, sizeof(originalSize), 1, output);
    fwrite(&treeSize, sizeof(treeSize), 1, output);
    
    if (!serializeTree(root, output)) {
        fclose(input);
        fclose(output);
        freeHuffmanTree(root);
        freeHuffmanHeap(heap);
        freeCodes(codes);
        return false;
    }
    
    uint8_t buffer   = 0;
    uint8_t bitCount = 0;
    
    
    while ((byte = fgetc(input)) != EOF) {
        const char *code = codes[byte];
        for (uint16_t i = 0; code[i] != '\0'; i++) {
            buffer <<= 1;
            if (code[i] == '1') buffer |= 1;
            bitCount++;
            
            if (bitCount == 8) {
                fputc(buffer, output);
                buffer   = 0;
                bitCount = 0;
            }
        }
    }
    
    if (bitCount > 0) {
        buffer <<= (8 - bitCount);
        fputc(buffer, output);
    }
    
    
    fclose(input);
    fclose(output);
    
    
    freeHuffmanTree(root);
    freeHuffmanHeap(heap);
    //~ freeCodes(codes);
    
    
    return 0;
}


bool huffmanDecompress(const char *inputFilePath) {
    char outputFilePath[PATH_MAX];
    snprintf(outputFilePath, sizeof(outputFilePath), "%s", inputFilePath);
    size_t pathLength = strlen(outputFilePath);
    
    if (pathLength < 5 || strcmp(outputFilePath + pathLength - 5, ".huff") != 0) {
        fprintf(stderr, "Input file must have .huff extension\n");
        return false;
    }
    
    outputFilePath[pathLength - 5] = '\0';
    
    FILE *input = fopen(inputFilePath, "rb");
    if (!input) {
        perror("Error opening compressed file");
        return false;
    }
    
    char magic[4];
    if (fread(magic, 1, 4, input) != 4) {
        fprintf(stderr, "Invalid Huffman file: cannot read header\n");
        fclose(input);
        return false;
    }
    
    if (memcmp(magic, "HUFF", 4) != 0) {
        fprintf(stderr, "Invalid Huffman file: bad magic\n");
        fclose(input);
        return false;
    }
    
    uint64_t originalSize;
    size_t treeSize;
    if (fread(&originalSize, sizeof(originalSize), 1, input) != 1) {
        fprintf(stderr, "Invalid Huffman file: cannot read original size\n");
        fclose(input);
        return false;
    }
    if(fread(&treeSize, sizeof(treeSize), 1, input) != 1) {
        fprintf(stderr, "Invalid Huffman file: cannot read tree size\n");
        fclose(input);
        return false;
    }
    
    long treeStart = ftell(input);
    if (treeSize == -1L) {
        perror("ftell");
        fclose(input);
        return false;
    }
    
    
    HuffmanNode *root = deserializeTree(input);
    if (!root) {
        fprintf(stderr, "Invalid Huffman Tree\n");
        fclose(input);
        return false;
    }
    
    long treeEnd = ftell(input);
    if (treeEnd == -1L) {
        perror("ftell");
        freeHuffmanTree(root);
        fclose(input);
        return false;
    }
    
    size_t consumedTreeSize = (size_t)(treeEnd - treeStart);
    if (consumedTreeSize != treeSize) {
        fprintf(stderr, "Invalid Huffman Tree Size\n");
        fprintf(stderr, "Expected: %zu\n", treeSize);
        fprintf(stderr, "Consumed: %zu\n", consumedTreeSize);
        freeHuffmanTree(root);
        fclose(input);
        return false;
    }
    
    
    FILE *output = fopen(outputFilePath, "wb");
    if (!output) {
        perror("Error opening output file");
        freeHuffmanTree(root);
        fclose(input);
        return false;
    }
    if (originalSize == 0) {
        fclose(output);
        fclose(input);
        freeHuffmanTree(root);
        return true;
    }
    
    
    if(isLeaf(root)) {
        for (size_t i = 0; i < originalSize; i++) {
            if(fputc(root->character, output) == EOF) {
                perror("Error writing decompressed file");
                fclose(output);
                fclose(input);
                freeHuffmanTree(root);
                return false;
            }
        }
        fclose(output);
        fclose(input);
        freeHuffmanTree(root);
        return true;
    }
    
    
    HuffmanNode *current = root;
    uint64_t decodedBytes = 0;
    
    int byte;
    
    while (decodedBytes < originalSize && (byte = fgetc(input)) != EOF) {
        for (int bit = 7; bit >= 0 && decodedBytes < originalSize; bit--) {
            int currentBit = (byte >> bit) & 1;
            
            if (currentBit == 0) current = current->left;
            else current = current->right;
            
            if (!current) {
                fprintf(stderr, "Invalid compressed data\n");
                fclose(output);
                fclose(input);
                freeHuffmanTree(root);
                return false;
            }
            
            if (isLeaf(current)) {
                if (fputc(current->character, output) == EOF) {
                    perror("Error writing decompressed file");
                    fclose(output);
                    fclose(input);
                    freeHuffmanTree(root);
                    return false;
                }
                decodedBytes++;
                current = root;
            }
        }
    }
    
    if (decodedBytes != originalSize) {
        fprintf(stderr, "Invalid compressed data\n");
        fprintf(stderr, "Expected %llu bytes, decoded %llu bytes\n", (uint64_t)originalSize, (uint64_t)decodedBytes);
        fclose(output);
        fclose(input);
        freeHuffmanTree(root);
        return false;
    }
    
    
    
    fclose(output);
    fclose(input);
    freeHuffmanTree(root);
    
    return true;
}











