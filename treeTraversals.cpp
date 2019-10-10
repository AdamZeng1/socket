//
// Created by adamzeng on 2019-05-21.
//

#include <cstdio>
#include <cstring>
#include <queue>
#include <algorithm>

using namespace std;
const int maxn = 50;

struct node {
    int data;
    node *lchild;
    node *rchild;
};

int pre[maxn], in[maxn], post[maxn]; // 先序、中序、后序
int n; // 结点个数

// 当前二叉树的后序序列区间为[postL,postR],中序序列区间为[inL,inR]
// create函数返回构建出的二叉树的根结点地址
node *create(int postL, int postR, int inL, int inR) {
    if (postL > postR) {
        return NULL; // 后序序列长度小于等于0时，直接返回
    }

    node *root = new node; // 新建一个新的结点，用来存放当前二叉树的根结点
    root->data = post[postR]; // 新结点的数据域为根结点的值
    int k;
    for (int k = inL; k <= inR; ++k) {
        if (in[k] == post[postR]) { // 在中序序列中找到in[k] == pre[L]的结点
            break;
        }
    }

    int numLeft = k - inL; // 左子树的结点个数
    // 返回左子树的根结点地址,赋值给root的左指针
    root->lchild = create(postL, postL + numLeft - 1, inL, k - 1);
    // 返回右子树的根结点地址,赋值给root的右指针
    root->rchild = create(postL + numLeft, postR - 1, k + 1, inR);
    return root; // 返回根结点地址
}

int num = 0; // 已输出的结点个数
void BFS(node *root) {
    queue<node *> q; // 注意队列里面存地址
    q.push(root); // 将root地址入队
    while (!q.empty()) {
        node *now = q.front(); // 取出队首元素
        q.pop();
        printf("%d", now->data);
        num++;
        if (num < n) printf(" ");
        if (now->lchild != NULL) q.push(now->lchild); //左子树非空
        if (now->rchild != NULL) q.push(now->rchild); //右子树非空
    }
}

int main() {
    scanf("%d", &n);
    for (int i = 0; i < n; ++i) {
        scanf("%d", &post[i]);
    }

    for (int i = 0; i < n; ++i) {
        scanf("%d", &in[i]);
    }
    node *root = create(0, n - 1, 0, n - 1);
    BFS(root); // 层序遍历
    return 0;
}
