#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

int label_id = 0;

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("lvalue has to be a variable");

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

// スタック命令のエミュレートを行う関数
void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        label_id++;
        gen(node->cond); // カッコ内のstatementをコンパイル. スタックトップに結果が入っている
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", label_id);
        gen(node->stmt); // if (A) B のBをコンパイル
        printf(".Lend%d:\n", label_id);
        return;
    case ND_IF_ELSE:
        label_id++;
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%d\n", label_id);
        gen(node->stmt);
        printf(".Lelse%d:\n", label_id);
        gen(node->else_stmt);
        printf(".Lend%d:\n", label_id);
        return;
    case ND_WHILE:
        label_id++;
        printf(".Lbegin%d:\n", label_id);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", label_id);
        gen(node->stmt);
        printf("    jmp .Lbegin%d\n", label_id);
        printf(".Lend%d:\n", label_id);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        // idivは符号あり除算を行う命令。ただし idiv rax, rdi　のようには書けない
        // idivは暗黙のうちにRDXとRAXを取って、それを合わせたものを128ビット整数とみなして、
        // それを引数のレジスタの64ビットの値で割り、商をRAXに、余りをRDXにセットする、という
        // 仕様になっている
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NEQ:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LEQ:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}
