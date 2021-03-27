set(LLVM_LINK_COMPONENTS support)

add_clang_executable(StyleChecker
	StyleChecker.cpp
  )

target_link_libraries(StyleChecker
  PRIVATE
  clangTooling
  clangBasic
  )
