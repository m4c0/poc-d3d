#define WIN32_LEAN_AND_MEAN
#include <d3d12.h>
#include <dxgi1_3.h>
#include <windows.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "user32.lib")

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
  switch (message) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hwnd, message, wp, lp);
}

#define COM(obj, method, ...) (obj)->lpVtbl->method(obj, __VA_ARGS__)

static IDXGIFactory3 * d3d_factory;

int d3d_init() {
  if (FAILED(CreateDXGIFactory2(0, &IID_IDXGIFactory3, (void **)&d3d_factory))) return 1;

  return 0;
}

void d3d_deinit() {
  COM(d3d_factory, Release);
}

int WINAPI WinMain(HINSTANCE h_inst, HINSTANCE h_prev, LPSTR cmdline, int n_cmd_show) {
  WNDCLASSEX wnd_class = {
    .cbSize        = sizeof(wnd_class),
    .style         = CS_HREDRAW | CS_VREDRAW,
    .lpfnWndProc   = window_proc,
    .hInstance     = h_inst,
    .hCursor       = LoadCursor(NULL, IDC_ARROW),
    .lpszClassName = "m4c0-window",
  };
  RegisterClassEx(&wnd_class);

  HWND hwnd = CreateWindow("m4c0-window", "Hello D3D",
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      800, 600,
      NULL, NULL, h_inst, NULL);

  if (d3d_init()) return 1;

  ShowWindow(hwnd, n_cmd_show); 

  MSG msg = {0};
  do {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  } while (msg.message != WM_QUIT);

  d3d_deinit();

  return msg.wParam;
}

