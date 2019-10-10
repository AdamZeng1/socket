#include <stdio.h>
#include <queue>
//
// Created by adamzeng on 2019-05-20.
//

using namespace std;

struct node {
    int data; // data field
    int layer; // layer
    node *lchild; // pointer points to left node
    node *rchild; // pointer points to right node
};

// generate a new node, v is node value
node *newNode(int v) {
    node *Node = new node; // apply for address location of node
    Node->data = v; // data of node is v
    Node->lchild = Node->rchild = nullptr; // no right and left child node
    return Node;
}

void search(node *root, int x, int newdata) {
    if (root == NULL) {
        return; // empty tree
    }
    if (root->data == x) { // find the node which data == x, and alter it to newdata
        root->data = newdata;
    }
    search(root->lchild, x, newdata);
    search(root->rchild, x, newdata);
}

void insert(node *&root, int x) {
    if (root == nullptr) { // empty tree means find fail, it's insert location
        root = newNode(x);
        return;
    }
    //
    if (/* */nullptr == nullptr) { // depends on tree's property
        insert(root->lchild, x);
    } else {
        insert(root->rchild, x);
    }
}

// build a tree
node *create(int data[], int n) {
    node *root = nullptr; // build new root node
    for (int i = 0; i < n; ++i) {
        insert(root, data[i]); // store data[0]~data[i-1] in the tree
    }
    return root;
}


void preorder(node *root) {
    if (root == nullptr) {
        return; // arrive empty node
    }
    // print the data field of root
    printf("%d\n", root->data);
    // visit left child node
    preorder(root->lchild);
    // visit right child node
    preorder(root->rchild);
}

void inorder(node *root) {
    if (root == nullptr) {
        return; // arrive empty node
    }
    // visit left child node
    inorder(root->lchild);
    // visit root node
    printf("%d\n", root->data);
    // visit right child node
    inorder(root->rchild);
}

void postorder(node *root) {
    if (root == nullptr) {
        return; // arrive empty node
    }
    // visit left child node
    postorder(root->lchild);
    // visit right child node
    postorder(root->rchild);
    // visit root node
    printf("%d\n", root->data);
}

void layerorder(node *root) {
    queue<node *> q;
    root->layer = 1; // root layer is 1
    q.push(root); // put root in queue
    while (!q.empty()) {
        node *now = q.front(); // get the first element in queue
        q.pop();
        printf("%d ", now->data); // visit the first element in queue
        if (now->lchild != nullptr) { // if left child is not empty
            now->lchild->layer = now->layer + 1; // left child node's layer equals to layer of now plus 1
            q.push(now->lchild);
        }
        if (now->rchild != nullptr) {
            now->rchild->layer = now->layer + 1; // right child node's layer equals to layer of now plus 1
            q.push(now->rchild);
        }
    }
}


int pre[10000] = {0};
int in[10000] = {0};


// 当前先序序列区间为[preL,preR],中序序列区间为[inL,inR],返回根结点地址
node *create(int preL, int preR, int inL, int inR) {
    if (preL > preR) {
        return NULL; // 先序序列长度小于等于0时，直接返回
    }

    node *root = new node; // 新建一个新的结点，用来存放当前二叉树的根结点
    root->data = pre[preL]; // 新结点的数据域为根结点的值
    int k;
    for (int k = inL; k <= inR; ++k) {
        if (in[k] == pre[preL]) { // 在中序序列中找到in[k] == pre[L]的结点
            break;
        }
    }
    int numLeft = k - inL; // 左子树的结点个数

    // 左子树的先序区间为[preL+1,preL+numLeft],中序区间为[inL,k-1]
    // 返回左子树的根结点地址，赋值给root的左指针
    root->lchild = create(preL + 1, preL + numLeft, inL, k - 1);

    // 右子树的先序区间为[preL+numLeft + 1,preR],中序区间为[k+1,inR]
    // 返回右子树的根结点地址,赋值给root的右指针
    root->rchild = create(preL + numLeft + 1, preR, k + 1, inR);

    return root;
}

int main() {
    printf("123");
}


