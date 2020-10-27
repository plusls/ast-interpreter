//
// Created by plusls on 2020/10/22.
//
#include "StackFrame.h"

using namespace clang;
using namespace std;

void StackFrame::SetArrayPtr(int64_t* array_ptr) {
    array_ptr_list_.push_back(array_ptr);
}

void StackFrame::SetDeclVal(Decl *decl, int64_t val) {
    vars_[decl] = val;
}

int64_t StackFrame::GetDeclVal(Decl *decl) {
    auto it = vars_.find(decl);
    assert (it != vars_.end());
    return it->second;
}

void StackFrame::SetExprVal(Expr *expr, int64_t val) {
    exprs_[expr] = val;
}

int64_t StackFrame::GetExprVal(Expr *expr) {
    assert (exprs_.find(expr) != exprs_.end());
    return exprs_[expr];
}

void StackFrame::SetPtrExprVal(Expr *expr, int64_t val) {
    ptr_exprs_[expr] = val;
}

int64_t StackFrame::GetPtrExprVal(Expr *expr) {
    assert (ptr_exprs_.find(expr) != ptr_exprs_.end());
    return ptr_exprs_[expr];
}

StackFrame::~StackFrame() {
    for (auto ptr: array_ptr_list_) {
        delete []ptr;
    }
}