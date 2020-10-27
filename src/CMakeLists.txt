# cmake_minimum_required(VERSION <specify CMake version here>)
project(homework)
cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 17)

add_executable(homework main.cpp Interpreter.cpp Interpreter.h StackFrame.cpp StackFrame.h InterpreterASTFrontendAction.cpp InterpreterASTFrontendAction.h)


find_package(Clang REQUIRED CONFIG HINTS ${LLVM_DIR} ${LLVM_DIR}/lib/cmake/clang NO_DEFAULT_PATH)
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS} SYSTEM)
link_directories(${LLVM_LIBRARY_DIRS})

set( LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Option
        Support
        )


target_link_libraries(homework
        clangAST
        clangBasic
        clangFrontend
        clangTooling
        )

#install(TARGETS ast-interpreter
#        RUNTIME DESTINATION bin)
