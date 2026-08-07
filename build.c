#include <process.h>

int main() {
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "hello.exe", "hello.c", NULL)) return 1;
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "pipeline.exe", "pipeline.c", NULL)) return 1;

  return 0;
}
