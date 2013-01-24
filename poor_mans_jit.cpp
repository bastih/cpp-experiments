#include <string>
#include <iostream>
#include <fstream>
#include <dlfcn.h>

using namespace std;

/// An attempt at a poor man's Just in time compilation

void compile(string filename, string libname)
{
  std::string cmd = "clang++ -shared -o "+libname+ " "+filename;
  system(cmd.c_str());
}

void toFile(string filename, string text)
{
  ofstream o(filename.c_str());
  o<<text;
  o.close();
}

typedef void (*voidfuncptr)();

int main (int argc, char const *argv[])
{
  std::string code = R"C++CODE(
#include <cstdio>
extern "C" 
  void entry() 
  { printf("compiled in runtime, bla me"); }
)C++CODE";
  std::string filename = "myfile.cpp";
  std::string libname = "./myfile.so";
  toFile(filename, code);
  compile(filename, libname);
  
  auto handle = dlopen(libname.c_str(), RTLD_GLOBAL);
  if (handle == nullptr)
  {
    exit(-1);
  }
  auto entry = (voidfuncptr) dlsym(handle, "entry");
  if (entry == nullptr)
  {
    exit(-1);
  }

  entry();
  return 0;
}
