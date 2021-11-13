int nowhere();

int foo()
{
    return nowhere();
}

// clang -cc1 foo.h -emit-pch -o foo.pch
