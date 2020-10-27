//
// Created by plusls on 2020/10/20.
//

#ifndef HOMEWORK_INTERPRETERASTFRONTENDACTION_H
#define HOMEWORK_INTERPRETERASTFRONTENDACTION_H

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/CompilerInstance.h>

#include "Interpreter.h"

class InterpreterASTFrontendAction : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance &Compiler, llvm::StringRef InFile) override;
};

#endif //HOMEWORK_INTERPRETERASTFRONTENDACTION_H
