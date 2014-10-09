#include "md5.h"

int main(int argc, char *argv[])
{
    MD5 m(argv[1]);

    std::cerr << "MD5 of " << argv[1] << " is " << m << std::endl;
}
