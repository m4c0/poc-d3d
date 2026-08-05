#include <process.h>

int main() {
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "hello.exe", "hello.c", "-luser32", NULL)) return 1;

  return 0;
}
