#pragma once

typedef struct Node
{
  int key;
  char color;
  struct Node *left;
  struct Node *right;
  struct Node *p;
} Node;

typedef struct Tree
{
  Node *root;
  Node *nil;
} Tree;

Node* makeNewNode(Tree *T, int value);
Node* treeSearch(Tree *T, Node *x, int k);  //x is a root of the subtree
Node* treeMinimum(Tree *T, Node *x);
Node* treeMaximum(Tree *T, Node *x);

void leftRotate(Tree *T, Node *x);
void rightRotate(Tree *T, Node *x); //same as left rotate but all lefts changed to rights
void insertFixup(Tree *T, Node *z);
void insert(Tree *T, int val);  //rb-insert
void transplant(Tree *T, Node *u, Node *v); //delete u, place v
void deleteFixup(Tree *T, Node *x);
void erase(Tree *T, int val);
void clearTree(Tree *T, Node *t);
void printTree(Node *q, long n);
int  contains(Tree *T, Node* x, int k);

  // gcc -std=c99 rb_tree.c -lm
  //valgrind --leak-check=full --leak-resolution=med  ... 
