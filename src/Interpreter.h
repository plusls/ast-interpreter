//
// Created by plusls on 2020/10/20.
//

#ifndef HOMEWORK_INTERPRETER_H
#define HOMEWORK_INTERPRETER_H

#include <cinttypes>

#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/EvaluatedExprVisitor.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>

#include "StackFrame.h"

class Interpreter :
        public clang::EvaluatedExprVisitor<Interpreter> {
public:
    explicit Interpreter(const clang::ASTContext &context)
            : EvaluatedExprVisitor{context} {}

    void VisitBinaryOperator(clang::BinaryOperator *binary_operator);

    void VisitUnaryOperator(clang::UnaryOperator *unary_operator);

    void VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *unary_expr);

    void VisitDeclRefExpr(clang::DeclRefExpr *expr);

    void VisitCastExpr(clang::CastExpr *expr);

    void VisitParenExpr(clang::ParenExpr *paren_expr);

    void VisitArraySubscriptExpr(clang::ArraySubscriptExpr *array_subscript_expr);

    void VisitCallExpr(clang::CallExpr *call);

    void VisitDeclStmt(clang::DeclStmt *declstmt);

    void VisitWhileStmt(clang::WhileStmt *while_stmt);

    void VisitForStmt(clang::ForStmt *for_stmt);



    [[maybe_unused]] void VisitIfStmt(clang::IfStmt *if_stmt);

    void VisitReturnStmt(clang::ReturnStmt *return_stmt);

    void Parse(clang::ASTContext &);

private:
    std::stack<StackFrame> runtime_stack_{};
    clang::FunctionDecl *free_decl_{};                /// Declartions to the built-in functions
    clang::FunctionDecl *malloc_decl_{};
    clang::FunctionDecl *get_decl_{};
    clang::FunctionDecl *print_decl_{};
    clang::FunctionDecl *main_decl_{}; // main 函数
    // 全局变量
    std::map<clang::Decl *, uint64_t> global_vars_{};

    void InterpreterExpr(clang::Expr *expr);
};

#endif //HOMEWORK_INTERPRETER_H
