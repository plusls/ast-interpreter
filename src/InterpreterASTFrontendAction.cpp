//
// Created by plusls on 2020/10/20.
//

#include "InterpreterASTFrontendAction.h"
#include "Interpreter.h"
using namespace clang;

class InterpreterASTConsumer : public ASTConsumer {
public:
    explicit InterpreterASTConsumer(clang::ASTContext &context) : interpreter_{context} {}

    void HandleTranslationUnit(clang::ASTContext &context) override {
        interpreter_.Parse(context);
    }

private:
    Interpreter interpreter_;
};


std::unique_ptr<ASTConsumer> InterpreterASTFrontendAction::CreateASTConsumer(
        CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(new InterpreterASTConsumer(Compiler.getASTContext()));
}