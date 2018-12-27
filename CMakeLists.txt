set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_clang_tool(Example
  Example.cpp
  )

target_link_libraries(Example
  PRIVATE
  clangBasic
clangDriver
clangAST
clangLex
clangParse
  clangFrontend
  clangRewrite
  clangSerialization
  clangTooling
  clangToolingCore
  clangToolingRefactor
  )

install(PROGRAMS Example.py
  DESTINATION share/clang
  COMPONENT Example)
install(PROGRAMS Example.el
  DESTINATION share/clang
  COMPONENT Example)
