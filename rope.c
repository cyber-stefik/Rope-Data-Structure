#include "./rope.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define EMPTY ""

RopeNode* makeRopeNode(const char* str) {
    RopeNode *new = malloc(sizeof(*new));
    new->left = NULL;
    new->right = NULL;

    // Se creaza nodul in functie de stringul dat ca parametru:
    // Frunza pentru str = NULL si subradacina in caz contrar.
    if (str) {
        new->weight = strlen(str);
        new->str = str;
    } else {
        new->weight = 0;
        new->str = NULL;
    }

    return new;
}

RopeTree* makeRopeTree(RopeNode* root) {
    RopeTree *new = malloc(sizeof(*new));
    new->root = root;

    return new;
}

void printRopeNode(RopeNode* rn) {
    if (!rn)
        return;
    if (!(rn->left) && !(rn->right)) {
        printf("%s", rn->str);
        return;
    }

    printRopeNode(rn->left);
    printRopeNode(rn->right);
}

void printRopeTree(RopeTree* rt) {
    if (rt && rt->root) {
        printRopeNode(rt->root);
        printf("%s", "\n");
    }
}

void debugRopeNode(RopeNode* rn, int indent) {
    if (!rn)
        return;

    for (int i = 0; i < indent; ++i)
        printf("%s", " ");

    if (!strcmp(rn->str, EMPTY))
        printf("# %d\n", rn->weight);
    else
        printf("%s %d\n", rn->str, rn->weight);

    debugRopeNode(rn->left, indent+2);
    debugRopeNode(rn->right, indent+2);
}

int getTotalWeight(RopeNode* rt) {
    if (!rt)
        return 0;

    return rt->weight + getTotalWeight(rt->right);
}


RopeTree* concat(RopeTree* rt1, RopeTree* rt2) {
    RopeNode *root = makeRopeNode(NULL);
    RopeTree *rt3 = makeRopeTree(root);

    rt3->root->left = rt1->root;
    rt3->root->right = rt2->root;
    rt3->root->weight = getTotalWeight(rt1->root);

    return rt3;
}

char indexRope(RopeTree* rt, int idx) {
    RopeNode *node = rt->root;

    // Se parcurge rope-ul pana cand se ajunge la o frunza.
    while (node->left || node->right) {
        if (idx >= node->weight) {
            idx = idx - node->weight;
            node = node->right;
        }
        else
            node = node->left;
    }
    // Se extrage caracterul de pe pozitia idx.
    return node->str[idx];
}

char* search(RopeTree* rt, int start, int end) {
    int i;
    char *word = malloc(end - start + 1);

    // Se extrage fiecare caracter a cuvantului de cautat apeland de
    // (end - start) ori functia indexRope.
    for (i = 0; i < (end - start); i++) {
        word[i] = indexRope(rt, (start + i));
    }
    // Copierea litera cu litera necesita un \0 la finalul cuvantului.
    word[i] = '\0';

    return word;
}

void go_through(RopeNode *rn, RopeNode *node1, RopeNode *node2, int idx) {
    // Caz de taiere a unei frunze
    if (!rn->left && !rn->right) {
        // Se creeaza noile stringuri pentru cele 2 frunze derivate din cea
        // principala.
        char *string1 = malloc(idx + 1);
        char *string2 = malloc(rn->weight - idx + 1);
        int i;

        for (i = 0; i < idx; i++)
            string1[i] = rn->str[i];
        string1[i] = '\0';

        for (i = 0; i < rn->weight - idx; i++)
            string2[i] = rn->str[i + idx];
        string2[i] = '\0';

        node1->str = string1;
        node1->weight = idx;
        node2->str = string2;
        node2->weight = strlen(string2);

        return;
    }
    // Split-ul se realizeaza la finalul unei frunze.
    if (idx == rn->weight) {
        node2->left = rn->right;
        node1->left = rn->left;
        node1->weight = getTotalWeight(node1->left);
        node2->weight = getTotalWeight(node2->left);
    } else if (idx < rn->weight){
        node2->right = rn->right;
        // Verificam ce fel de noduri trebuie sa cream.
        if (rn->left->left || rn->left->right)
            node2->left = makeRopeNode(NULL);
        else
            node2->left = makeRopeNode(rn->left->str);
        if (rn->left->left || rn->left->right)
            node1->left = makeRopeNode(NULL);
        else
            node1->left = makeRopeNode(rn->left->str);
        // Ne mutam pe subarborele stang.
        go_through(rn->left, node1->left, node2->left, idx);
        // Modificam greutatea noilor noduri adaugate.
        node1->weight = getTotalWeight(node1->left);
        node2->weight =  getTotalWeight(node2->left);
    } else {
        // Daca idx > greutate (sub)radacina atunci primul nod
        // va pointa la branch stang.
        node1->left = rn->left;
        // Verificam ce fel de nod trebuie sa cream.
        if (rn->right->left || rn->right->right)
            node1->right = makeRopeNode(NULL);
        else
            node1->right = makeRopeNode(rn->right->str);
        // Se scade valoarea indexului pentru a cauta in branch-ul drept.
        idx  = idx - rn->weight;

        go_through(rn->right, node1->right, node2, idx);
        // Modificam greutatea nodului nou adaugat.
        node1->weight = getTotalWeight(node1->left);
    }
}
SplitPair split(RopeTree* rt, int idx) {
    SplitPair pair;

    // Caz particular cand idx > greutate totala a Rope-ului.
    if (idx > getTotalWeight(rt->root)) {
        pair.left = rt->root;
        pair.right = NULL;
        return pair;
    }

    // Initializare pereche de noduri.
    RopeNode *root = rt->root;
    RopeNode *node1 = makeRopeNode(NULL);
    RopeNode *node2 = makeRopeNode(NULL);

    go_through(root, node1, node2, idx);

    pair.left = node1;
    pair.right = node2;

    return pair;
}

RopeTree* insert(RopeTree* rt, int idx, const char* str) {
    SplitPair pair = split(rt, idx);
    RopeTree *left = makeRopeTree(pair.left);
    RopeTree *right = makeRopeTree(pair.right);
    RopeNode *middle_node = makeRopeNode(str);
    RopeTree *middle = makeRopeTree(middle_node);
    RopeTree *first_half = concat(left, middle);
    RopeTree *new = concat(first_half, right);

    // Eliberare memorie alocata suplimentar.
    free(left);
    free(right);
    free(middle);
    free(first_half);

    return new;
}

RopeTree* delete(RopeTree* rt, int start, int len) {
    SplitPair pair1 = split(rt, start);
    SplitPair pair2 = split(rt, start +len);
    RopeTree *left = makeRopeTree(pair1.left);
    RopeTree *right = makeRopeTree(pair2.right);
    RopeTree *new = concat(left, right);

    // Eliberare memorie suplimentara alocata.
    free(left);
    free(right);

    return new;
}
