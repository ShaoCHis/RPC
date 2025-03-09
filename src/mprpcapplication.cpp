#include "mprpcapplication.hpp"

void MprpcApplication::Init(int argc,char **argv)
{

}

MprpcApplication& MprpcApplication::getInstance()
{
    static MprpcApplication instance;
    return instance;
}
