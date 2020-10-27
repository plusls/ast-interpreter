#include <string>
#include <iostream>
#include <fstream>
#include <memory>

#include <clang/Tooling/Tooling.h>
#include "InterpreterASTFrontendAction.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Run as " << argv[0] << " xxx.c" << std::endl;
        return 1;
    }

    std::ifstream codeFile(argv[1]);
    if (codeFile.is_open()) {
        std::string code((std::istreambuf_iterator<char>(codeFile)), std::istreambuf_iterator<char>());
        // std::cout << code << std::endl;
        clang::tooling::runToolOnCode(std::unique_ptr<clang::FrontendAction>(new InterpreterASTFrontendAction), code);
    } else {
        std::cout << "Can't open " << argv[1] << std::endl;
        return 1;
    }
}
