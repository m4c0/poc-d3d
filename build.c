#include <process.h>

int main() {
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "hello.exe", "hello.c", NULL)) return 1;
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "pipeline.exe", "pipeline.c", NULL)) return 1;
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "cbuffer.exe", "cbuffer.c", NULL)) return 1;
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "cbuffer-desc.exe", "cbuffer-desc.c", NULL)) return 1;
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "cbuffer-root.exe", "cbuffer-root.c", NULL)) return 1;
  if (0 != _spawnlp(_P_WAIT, "clang.exe", "clang.exe", "-o", "buffer.exe", "buffer.c", NULL)) return 1;

  return 0;
}
