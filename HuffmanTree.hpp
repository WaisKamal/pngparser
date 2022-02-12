#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE

#include <vector>
#include <queue>
#include <algorithm>
#include "bitreader.hpp"

template <typename T>
class Node {
public:
    T value;
    Node* left = nullptr;
    Node* right = nullptr;
    Node() {}
};

template <typename T>
class HuffmanTree {
private:
    Node<T> rootNode;

public:
    // Creates Huffman tree given alphabet range and code lengths
    HuffmanTree(T alphaStart, T alphaEnd, std::vector<int> codeLengths) {
        int maxCodeLength = *std::max_element(codeLengths.begin(), codeLengths.end());
        std::vector<int> codeLengthCount(maxCodeLength + 1, 0);
        for (size_t i = 0; i < codeLengths.size(); i++) {
            codeLengthCount[codeLengths[i]]++;
        }
        std::vector<int> baseLengths(maxCodeLength + 1, 0);
        for (size_t i = 2; i < baseLengths.size(); i++) {
            baseLengths[i] = (baseLengths[i - 1] + codeLengthCount[i - 1]) * 2;
        }
        for (size_t i = 0; i < codeLengths.size(); i++) {
            if (codeLengths[i] > 0) {
                this->insert(baseLengths[codeLengths[i]]++, codeLengths[i], i);
            }
        }
    }

    void insert(int codeword, int codewordLength, T symbol) {
        Node<T>* currentNode = &rootNode;
        for (int i = codewordLength - 1; i >= 0; i--) {
            bool currentBit = (codeword >> i) & 1;
            if (currentBit) {
                if (!(currentNode->right)) {
                    currentNode->right = new Node<T>();
                }
                currentNode = currentNode->right;
            }
            else {
                if (!(currentNode->left)) {
                    currentNode->left = new Node<T>();
                }
                currentNode = currentNode->left;
            }
        }
        currentNode->value = symbol;
    }

    T decode(BitReader& bits) {
        Node<T>* currentNode = &rootNode;
        T result;
        bool currentBit;
        while (true) {
            bits.readBits(currentBit, 1);
            if (currentBit) {
                currentNode = currentNode->right;
            }
            else {
                currentNode = currentNode->left;
            }
            if (!(currentNode->left || currentNode->right)) {
                result = currentNode->value;
                break;
            }
        }
        return result;
    }

    ~HuffmanTree() {
        std::queue<Node<T>*> q;
        q.push(&rootNode);
        while (!q.empty()) {
            Node<T>* currentNode = q.front();
            q.pop();
            if (currentNode->left) {
                q.push(currentNode->left);
            }
            if (currentNode->right) {
                q.push(currentNode->right);
            }
        }
    }
};

#endif