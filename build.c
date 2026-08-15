#include <process.h>

// #define OPT "-O3"
#define OPT "-gdwarf"

#define CC(X) if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", OPT, "-o", X".exe", X".c", NULL)) return 1;

int main() {
  CC("hello");
  CC("pipeline");
  CC("cbuffer");
  CC("cbuffer-desc");
  CC("cbuffer-root");
  CC("buffer");
  CC("texture");

  return 0;
}
