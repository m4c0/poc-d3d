# poc-d3d

Self-contained Direct3D 12 examples written in C.

All examples were developed and tested with "clang" but it should work with
MSVC as well.

Files:
* build.c - convenience to build everything
* hello.c - basic example enough to start Direct3D, initalise basic stuff, etc
* pipeline.c - extends "hello.c" to add a very basic pipeline with no inputs
* buffer.c - extends "pipeline.c" to add a ByteAddressBuffer to the shader
* cbuffer.c - extends "pipeline.c" to add a cbuffer using CBV
* cbuffer-desc.c - extends "pipeline.c" to add a cbuffer using a descritor
  table
* cbuffer-root.c - extends "pipeline.c" to add a cbuffer using root constants
* texture.c - extends "pipeline.c" to add a PS-accessible texture and sampler

You should be able to "diff" files to catch what's different between them.
Example: diff between `buffer.c` and `pipeline.c` to see what's needed to pass
data to the Pixel Shader using ByteAddressBuffer.
