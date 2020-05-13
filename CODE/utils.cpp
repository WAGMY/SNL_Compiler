#include "utils.h"

TreeNode *newRootNode() {
    return newSpecNode(ProK);
}


TreeNode *newPheadNode() {
    return newSpecNode(PheadK);
}


TreeNode *newDecANode(NodeKind kind) {
    return newSpecNode(kind);
}


TreeNode *newTypeNode() {
    return newSpecNode(TypeK);
}
TreeNode *newVarNode() {
    return newSpecNode(VarK);
}

TreeNode *newDecNode() {
    return newSpecNode(DecK);
}

TreeNode *newProcNode() {
    return newSpecNode(ProcDecK);
}

TreeNode *newStmlNode() {
    return newSpecNode(StmLK);
}

TreeNode *newStmtNode(StmtKind kind) {
    auto t = newSpecNode(StmtK);

    t->kind.stmt = kind;
    return t;
}

TreeNode *newExpNode(ExpKind kind) {
    auto t = newSpecNode(ExpK);
    t->kind.exp = kind;
    return t;
}

TreeNode *newSpecNode(NodeKind kind)
{

    auto t = new TreeNode;

    int i;

    for (i = 0; i < MAXCHILDREN; i++) t->child[i] = nullptr;
    t->sibling = nullptr;
    t->nodekind = kind;
    t->lineno = lineno;
    t->idnum=0;
    for (i = 0; i < 10; i++) {
        strcpy(t->name[i], "\0");
        t->table[i] = nullptr;
    }
    return t;
}
