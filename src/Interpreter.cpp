//
// Created by plusls on 2020/10/20.
//
#include <iostream>
#include <cstdlib>
#include <clang/Basic/TypeTraits.h>
#include <clang/AST/OperationKinds.h>
#include "Interpreter.h"
#include "Exception.h"

using namespace clang;
using namespace std;

void Interpreter::VisitIfStmt(IfStmt *if_stmt) {
    Expr *cond_expr = if_stmt->getCond();
    InterpreterExpr(cond_expr);
    Stmt *else_stmt = if_stmt->getElse();
    if (runtime_stack_.top().GetExprVal(cond_expr)) {
        Visit(if_stmt->getThen());
    } else {
        if (else_stmt) {
            Visit(else_stmt);
        }
    }
}

void Interpreter::VisitWhileStmt(WhileStmt *while_stmt) {
    Expr *cond_expr = while_stmt->getCond();
    Stmt *body = while_stmt->getBody();
    while (InterpreterExpr(cond_expr), runtime_stack_.top().GetExprVal(cond_expr)) {
        Visit(body);
    }
}

void Interpreter::VisitForStmt(ForStmt *for_stmt) {
    Expr *cond_expr = for_stmt->getCond(),
            *inc_expr = for_stmt->getInc();
    Stmt *init = for_stmt->getInit(),
            *body = for_stmt->getBody();
    if (init) {
        Visit(init);
    }
    for (;
            InterpreterExpr(cond_expr), runtime_stack_.top().GetExprVal(cond_expr);
            InterpreterExpr(inc_expr)) {
        Visit(body);
    }
}

void Interpreter::VisitReturnStmt(ReturnStmt *return_stmt) {
    InterpreterExpr(return_stmt->getRetValue());
    runtime_stack_.top().return_val = runtime_stack_.top().GetExprVal(return_stmt->getRetValue());
    throw ReturnException();
}


void Interpreter::Parse(ASTContext &context) {
    TranslationUnitDecl *translationUnitDecl = context.getTranslationUnitDecl();

    // 该栈帧用于初始化全局变量
    runtime_stack_.push(StackFrame());

    // 遍历 TranslationUnitDecl
    for (TranslationUnitDecl::decl_iterator decl = translationUnitDecl->decls_begin(),
                 end = translationUnitDecl->decls_end(); decl != end; ++decl) {
        // 函数定义
        if (auto *function_decl = dyn_cast<FunctionDecl>(*decl)) {
            if (function_decl->getName().equals("FREE")) {
                free_decl_ = function_decl;
            } else if (function_decl->getName().equals("MALLOC")) {
                malloc_decl_ = function_decl;
            } else if (function_decl->getName().equals("GET")) {
                get_decl_ = function_decl;
            } else if (function_decl->getName().equals("PRINT")) {
                print_decl_ = function_decl;
            } else if (function_decl->getName().equals("main")) {
                function_decl->getName();
                main_decl_ = function_decl;
            }
        } else if (auto *var_decl = dyn_cast<VarDecl>(*decl)) {
            global_vars_[var_decl] = 0;
            if (var_decl->hasInit()) {
                Expr *expr = var_decl->getInit();
                InterpreterExpr(expr);
                global_vars_[var_decl] = runtime_stack_.top().GetExprVal(expr);
            }
        }
    }
    runtime_stack_.pop();
    // main 函数栈帧
    runtime_stack_.push(StackFrame());
    // 遍历 main 语法树
    try {
        VisitStmt(main_decl_->getBody());
    } catch (ReturnException &e) {}
}

void Interpreter::VisitParenExpr(ParenExpr *paren_expr) {
    // 括号表达式
    Expr *sub_expr = paren_expr->getSubExpr();
    InterpreterExpr(sub_expr);
    runtime_stack_.top().SetExprVal(paren_expr, runtime_stack_.top().GetExprVal(sub_expr));
}

void Interpreter::VisitArraySubscriptExpr(ArraySubscriptExpr *array_subscript_expr) {
    Expr *left_expr = array_subscript_expr->getLHS(),
            *right_expr = array_subscript_expr->getRHS();
    QualType left_type = left_expr->getType(), right_type = right_expr->getType();
    InterpreterExpr(left_expr);
    InterpreterExpr(right_expr);
    int64_t array_ptr = 0,
            left_value = runtime_stack_.top().GetExprVal(left_expr),
            right_value = runtime_stack_.top().GetExprVal(right_expr);
    if (left_type->isPointerType() && right_type->isIntegerType()) {
        array_ptr = left_value + right_value * 8;
    } else if (left_type->isIntegerType() && right_type->isPointerType()) {
        array_ptr = right_value + left_value * 8;
    } else {
        throw "wtf";
    }
    runtime_stack_.top().SetPtrExprVal(array_subscript_expr, array_ptr);
    runtime_stack_.top().SetExprVal(array_subscript_expr, *(int64_t *) array_ptr);
}

void Interpreter::InterpreterExpr(Expr *expr) {
    if (auto integer_literal = dyn_cast<IntegerLiteral>(expr)) {
        runtime_stack_.top().SetExprVal(expr, integer_literal->getValue().getSExtValue());
    } else if (auto character_literal = dyn_cast<CharacterLiteral>(expr)) {
        runtime_stack_.top().SetExprVal(expr, character_literal->getValue());
    } else {
        Visit(expr);
    }
}

void Interpreter::VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *unary_expr) {
    auto op = unary_expr->getKind();
    int64_t result = 0;
    QualType type = unary_expr->getArgumentType();
    switch (op) {
        case UETT_SizeOf:
            if (unary_expr->getArgumentType()->isIntegerType()) {
                result = sizeof(int64_t);
            } else if (unary_expr->getArgumentType()->isPointerType()) {
                result = sizeof(int64_t *);
            } else {
                throw "wtf";
            }
            break;
        default:
            throw "wtf";
    }
    runtime_stack_.top().SetExprVal(unary_expr, result);
}


void Interpreter::VisitUnaryOperator(UnaryOperator *unary_operator) {
//// [C99 6.5.2.4] Postfix increment and decrement
//    UNARY_OPERATION(PostInc, "++")
//    UNARY_OPERATION(PostDec, "--")
//// [C99 6.5.3.1] Prefix increment and decrement
//    UNARY_OPERATION(PreInc, "++")
//    UNARY_OPERATION(PreDec, "--")
//// [C99 6.5.3.2] Address and indirection
//    UNARY_OPERATION(AddrOf, "&")
//    UNARY_OPERATION(Deref, "*")
//// [C99 6.5.3.3] Unary arithmetic
//    UNARY_OPERATION(Plus, "+")
//    UNARY_OPERATION(Minus, "-")
//    UNARY_OPERATION(Not, "~")
//    UNARY_OPERATION(LNot, "!")
//// "__real expr"/"__imag expr" Extension.
//    UNARY_OPERATION(Real, "__real")
//    UNARY_OPERATION(Imag, "__imag")
//// __extension__ marker.
//    UNARY_OPERATION(Extension, "__extension__")
//// [C++ Coroutines] co_await operator
//    UNARY_OPERATION(Coawait, "co_await")
    auto op = unary_operator->getOpcode();
    Expr *expr = unary_operator->getSubExpr();
    InterpreterExpr(expr);
    int64_t value = runtime_stack_.top().GetExprVal(expr),
            result = 0;
    switch (op) {
        case UO_Minus:
            result = -1 * value;
            break;
        case UO_Plus:
            result = value;
            break;
        case UO_Deref: // '*'
            result = *(int64_t *) value;
            runtime_stack_.top().SetPtrExprVal(unary_operator, value);
            break;
        default:
            throw "wtf";
    }
    runtime_stack_.top().SetExprVal(unary_operator, result);
}


void Interpreter::VisitBinaryOperator(BinaryOperator *binary_operator) {
//    BINARY_OPERATION(PtrMemD, ".*")
//    BINARY_OPERATION(PtrMemI, "->*")
//// [C99 6.5.5] Multiplicative operators.
//    BINARY_OPERATION(Mul, "*")
//    BINARY_OPERATION(Div, "/")
//    BINARY_OPERATION(Rem, "%")
//// [C99 6.5.6] Additive operators.
//    BINARY_OPERATION(Add, "+")
//    BINARY_OPERATION(Sub, "-")
//// [C99 6.5.7] Bitwise shift operators.
//    BINARY_OPERATION(Shl, "<<")
//    BINARY_OPERATION(Shr, ">>")
//// C++20 [expr.spaceship] Three-way comparison operator.
//    BINARY_OPERATION(Cmp, "<=>")
//// [C99 6.5.8] Relational operators.
//    BINARY_OPERATION(LT, "<")
//    BINARY_OPERATION(GT, ">")
//    BINARY_OPERATION(LE, "<=")
//    BINARY_OPERATION(GE, ">=")
//// [C99 6.5.9] Equality operators.
//    BINARY_OPERATION(EQ, "==")
//    BINARY_OPERATION(NE, "!=")
//// [C99 6.5.10] Bitwise AND operator.
//    BINARY_OPERATION(And, "&")
//// [C99 6.5.11] Bitwise XOR operator.
//    BINARY_OPERATION(Xor, "^")
//// [C99 6.5.12] Bitwise OR operator.
//    BINARY_OPERATION(Or, "|")
//// [C99 6.5.13] Logical AND operator.
//    BINARY_OPERATION(LAnd, "&&")
//// [C99 6.5.14] Logical OR operator.
//    BINARY_OPERATION(LOr, "||")
//// [C99 6.5.16] Assignment operators.
//    BINARY_OPERATION(Assign, "=")
//    BINARY_OPERATION(MulAssign, "*=")
//    BINARY_OPERATION(DivAssign, "/=")
//    BINARY_OPERATION(RemAssign, "%=")
//    BINARY_OPERATION(AddAssign, "+=")
//    BINARY_OPERATION(SubAssign, "-=")
//    BINARY_OPERATION(ShlAssign, "<<=")
//    BINARY_OPERATION(ShrAssign, ">>=")
//    BINARY_OPERATION(AndAssign, "&=")
//    BINARY_OPERATION(XorAssign, "^=")
//    BINARY_OPERATION(OrAssign, "|=")
//// [C99 6.5.17] Comma operator.
//    BINARY_OPERATION(Comma, ",")

    Expr *left_expr = binary_operator->getLHS();
    Expr *right_expr = binary_operator->getRHS();

    InterpreterExpr(left_expr);
    InterpreterExpr(right_expr);

    auto op = binary_operator->getOpcode();
    int64_t right_value = runtime_stack_.top().GetExprVal(right_expr),
            left_value = runtime_stack_.top().GetExprVal(left_expr),
            result = 0;
    switch (op) {
        case BO_Assign:
            if (auto *decl_ref_expr = dyn_cast<DeclRefExpr>(left_expr)) {
                Decl *decl = decl_ref_expr->getFoundDecl();
                runtime_stack_.top().SetDeclVal(decl, right_value);
            } else if (auto *array_subscript_expr = dyn_cast<ArraySubscriptExpr>(left_expr)) {
                left_value = runtime_stack_.top().GetPtrExprVal(left_expr);
                *(int64_t *) left_value = right_value;
            } else if (auto *unary_operator = dyn_cast<UnaryOperator>(left_expr)) {
                left_value = runtime_stack_.top().GetPtrExprVal(left_expr);
                *(int64_t *) left_value = right_value;
            } else {
                throw "wtf";
            }
            result = right_value;
            break;
        case BO_Add:
            if (left_expr->getType()->isPointerType() && right_expr->getType()->isIntegerType()) {
                result = left_value + sizeof(int64_t) * right_value;
            } else if (left_expr->getType()->isIntegerType() && right_expr->getType()->isPointerType()) {
                result = right_value + sizeof(int64_t) * left_value;
            } else if (left_expr->getType()->isIntegerType() && right_expr->getType()->isIntegerType()) {
                result = left_value + right_value;
            } else {
                throw "wtf";
            }
            break;
        case BO_Sub:
            result = left_value - right_value;
            if (left_expr->getType()->isPointerType() && right_expr->getType()->isIntegerType()) {
                result = left_value - sizeof(int64_t) * right_value;
            } else if (left_expr->getType()->isPointerType() && right_expr->getType()->isPointerType()) {
                result = (left_value - right_value) / sizeof(int64_t);
            } else if (left_expr->getType()->isIntegerType() && right_expr->getType()->isIntegerType()) {
                result = left_value - right_value;
            } else {
                throw "wtf";
            }
            break;
        case BO_Mul:
            result = left_value * right_value;
            break;
        case BO_Div:
            if (right_value == 0) {
                cout << "div 0 error! " << endl;
                exit(1);
            }
            result = left_value / right_value;
            break;
        case BO_LT:
            result = left_value < right_value;
            break;
        case BO_GT:
            result = left_value > right_value;
            break;
        case BO_EQ:
            result = left_value == right_value;
            break;
        default:
            throw "wtf";
            break;
    }
    runtime_stack_.top().SetExprVal(binary_operator, result);

}


void Interpreter::VisitDeclRefExpr(DeclRefExpr *decl_ref_expr) {
    // VisitStmt 可能会出问题但是我没有证据
    VisitStmt(decl_ref_expr);
    QualType type = decl_ref_expr->getType();
    if (type->isIntegerType() || type->isPointerType() || type->isArrayType()) {
        // getFoundDecl ?
        Decl *decl = decl_ref_expr->getFoundDecl();
        decl_ref_expr->getDecl();
        auto global_var_it = global_vars_.find(decl);
        int64_t val = 0;
        if (global_var_it != global_vars_.end()) {
            val = global_var_it->second;
        } else {
            val = runtime_stack_.top().GetDeclVal(decl);
        }
        runtime_stack_.top().SetExprVal(decl_ref_expr, val);
    } else {
        throw "wtf";
    }
}

void Interpreter::VisitCastExpr(CastExpr *cast_expr) {
    Expr *sub_expr = cast_expr->getSubExpr();
    InterpreterExpr(sub_expr);
    QualType type = cast_expr->getType();
    if (type->isIntegerType() || type->isPointerType()) {
        int64_t val = runtime_stack_.top().GetExprVal(sub_expr);
        runtime_stack_.top().SetExprVal(cast_expr, val);
    } else {
        throw "wtf";
    }
}

void Interpreter::VisitCallExpr(CallExpr *call_expr) {
    FunctionDecl *callee = call_expr->getDirectCallee();
    if (callee == get_decl_) {
        int64_t val = 0;
        llvm::errs() << "Please Input an Integer Value : ";
        cin >> val;
        runtime_stack_.top().SetExprVal(call_expr, val);
    } else if (callee == print_decl_) {
        int64_t val = 0;
        Expr *arg_expr = call_expr->getArg(0);
        InterpreterExpr(arg_expr);
        val = runtime_stack_.top().GetExprVal(arg_expr);
        runtime_stack_.top().SetExprVal(call_expr, 0);
        cerr << val << endl;
    } else if (callee == malloc_decl_) {
        int64_t val = 0;
        Expr *arg_expr = call_expr->getArg(0);
        InterpreterExpr(arg_expr);
        val = runtime_stack_.top().GetExprVal(arg_expr);
        runtime_stack_.top().SetExprVal(call_expr, (int64_t) malloc(val));
    } else if (callee == free_decl_) {
        Expr *arg_expr = call_expr->getArg(0);
        InterpreterExpr(arg_expr);
        void *val = (void *) runtime_stack_.top().GetExprVal(arg_expr);
        free(val);
        runtime_stack_.top().SetExprVal(call_expr, 0);
    } else {
        StackFrame new_stack_frame{};
        // 设置参数
        for (unsigned int i = 0, arg_count = call_expr->getNumArgs(); i < arg_count; ++i) {
            Decl *arg_decl = callee->getParamDecl(i);
            Expr *arg_expr = call_expr->getArg(i);
            InterpreterExpr(arg_expr);
            new_stack_frame.SetDeclVal(arg_decl, runtime_stack_.top().GetExprVal(arg_expr));
        }
        runtime_stack_.push(new_stack_frame);

        try {
            VisitStmt(callee->getBody());
        } catch (ReturnException &e) {}
        // 读取返回值
        int64_t return_value = runtime_stack_.top().return_val;
        runtime_stack_.pop();
        runtime_stack_.top().SetExprVal(call_expr, return_value);
    }
}

void Interpreter::VisitDeclStmt(DeclStmt *decl_stmt) {
    for (DeclStmt::decl_iterator it = decl_stmt->decl_begin(), ie = decl_stmt->decl_end();
         it != ie; ++it) {
        Decl *decl = *it;
        if (auto *var_decl = dyn_cast<VarDecl>(decl)) {
            QualType type = var_decl->getType();
            if (type->isArrayType()) {
                if (auto array = dyn_cast<ConstantArrayType>(type.getTypePtr())) {
                    int64_t array_size = array->getSize().getSExtValue();
                    auto *array_ptr = new int64_t[array_size];
                    runtime_stack_.top().SetDeclVal(var_decl, (int64_t) array_ptr);
                    runtime_stack_.top().SetArrayPtr(array_ptr);
                    for (int64_t i = 0; i < array_size; ++i) {
                        array_ptr[i] = 0;
                    }
                    if (var_decl->hasInit()) {
                        // todo 数组初始化
                        throw "wtf";
                    }
                } else {
                    // todo 动态数组
                    throw "wtf";
                }

            } else if (type->isIntegerType() || type->isPointerType()) {
                if (var_decl->hasInit()) {
                    Expr *expr = var_decl->getInit();
                    InterpreterExpr(expr);
                    runtime_stack_.top().SetDeclVal(var_decl, runtime_stack_.top().GetExprVal(expr));
                } else {
                    runtime_stack_.top().SetDeclVal(var_decl, 0);
                }
            } else {
                throw "wtf";
            }
        }
    }
}
