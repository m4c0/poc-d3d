#define WIN32_LEAN_AND_MEAN
#include <windows.h>

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
  switch (message) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hwnd, message, wp, lp);
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

  ShowWindow(hwnd, n_cmd_show); 

  MSG msg = {0};
  do {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  } while (msg.message != WM_QUIT);

  return msg.wParam;
}

