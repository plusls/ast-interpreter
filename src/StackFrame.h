//
// Created by plusls on 2020/10/20.
//

#ifndef HOMEWORK_STACKFRAME_H
#define HOMEWORK_STACKFRAME_H

#include <cinttypes>
#include <map>
#include <vector>
#include <clang/AST/Decl.h>

class StackFrame {

public:

    void SetArrayPtr(int64_t* array_ptr);

    void SetDeclVal(clang::Decl *decl, int64_t val);

    int64_t GetDeclVal(clang::Decl *decl);

    void SetExprVal(clang::Expr *expr, int64_t val);

    int64_t GetExprVal(clang::Expr *expr);

    void SetPtrExprVal(clang::Expr *expr, int64_t val);

    int64_t GetPtrExprVal(clang::Expr *expr);

    int64_t return_val{};

    ~StackFrame();

private:
    std::map<clang::Decl *, int64_t> vars_{};
    std::map<clang::Expr *, int64_t> exprs_{};
    std::map<clang::Expr *, int64_t> ptr_exprs_{};
    std::vector<int64_t*> array_ptr_list_{};

};

#endif //HOMEWORK_STACKFRAME_H
