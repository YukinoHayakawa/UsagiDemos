#define _CRT_SECURE_NO_WARNINGS

// #define NDEBUG

#include <iostream>

#include <fmt/ostream.h>

#include <Usagi/Modules/Runtime/ProgramModule/ClangJIT.hpp>
#include <Usagi/Modules/Runtime/ProgramModule/RuntimeModule.hpp>

using namespace usagi;

std::string source = R"(
int bar()
{
    return foo();
}
)";

int main(int argc, char *argv[])
{
    ClangJIT jit;

    auto compiler = jit.create_compiler();
    compiler.set_pch("foo.pch");
    MemoryRegion src;
    src.base_address = source.data();
    src.length = source.size();
    compiler.add_source("test", src);
    auto module_ = compiler.compile();
    auto ret = module_->get_function_address<int()>("bar")();

    std::cout << ret << std::endl;
}
