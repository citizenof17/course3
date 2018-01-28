#pragma once

typedef struct Node {
  int key;
  int value;
  char color;
  struct Node *left;
  struct Node *right;
  struct Node *p;
} Node;

typedef struct Tree {
  Node *root;
  Node *nil;
} Tree;

Tree* createTree();
Node* makeNewNode(Tree *T, int key, int value);
Node* treeSearch(Tree *T, Node *x, int k);  //x is a root of the subtree
Node* treeMinimum(Tree *T, Node *x);
Node* treeMaximum(Tree *T, Node *x);

void leftRotate(Tree *T, Node *x);
void rightRotate(Tree *T, Node *x); //same as left rotate but all lefts changed to rights
void insertFixup(Tree *T, Node *z);
void insert(Tree *T, int key, int value);  //rb-insert
void transplant(Tree *T, Node *u, Node *v); //delete u, place v
void deleteFixup(Tree *T, Node *x);
void erase(Tree *T, int key);
void clearTree(Tree *T, Node *t);
void deleteTree(Tree *T);
void printTree(Node *q, long n);
int  contains(Tree *T, Node* x, int k);
int  getValue(Tree *T, int k);


  // gcc -std=c99 rb_tree.c -lm
  //valgrind --leak-check=full --leak-resolution=med  ... 
