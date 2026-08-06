#define WIN32_LEAN_AND_MEAN
#include <initguid.h> // Should come first

#include <d3d12.h>
#include <dxgi1_6.h>
#include <windows.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
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
#define COM_OK(obj, method, ...) SUCCEEDED(COM(obj, method, __VA_ARGS__))

static IDXGIFactory3 * d3d_factory;
static IDXGIAdapter1 * d3d_adapter;

static inline int d3d_enum_adapter_by_gpu(IDXGIFactory6 * f6, unsigned i) {
  return COM_OK(f6, EnumAdapterByGpuPreference, i, DXGI_GPU_PREFERENCE_UNSPECIFIED, &IID_IDXGIAdapter1, (void **)&d3d_adapter);
}
static inline int d3d_enum_adapter(unsigned i) {
  return COM_OK(d3d_factory, EnumAdapters1, i, &d3d_adapter);
}
static inline int d3d_adapter_is_software(void) {
  DXGI_ADAPTER_DESC1 desc;
  COM(d3d_adapter, GetDesc1, &desc);
  return desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE;
}
static inline int d3d_create_device(void ** device) {
  return FAILED(D3D12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, NULL));
}

static int d3d_init_adapter(void) {
  IDXGIFactory6 * factory6;
  if (COM_OK(d3d_factory, QueryInterface, &IID_IDXGIFactory6, (void **)&factory6)) {
    for (unsigned i = 0; d3d_enum_adapter_by_gpu(factory6, i); i++) {
      if (d3d_adapter_is_software()) continue;
      if (0 == d3d_create_device(NULL)) return 0;
    }
  }

  for (unsigned i = 0; d3d_enum_adapter(i); i++) {
    if (d3d_adapter_is_software()) continue;
    if (0 == d3d_create_device(NULL)) return 0;
  }

  IDXGIFactory4 * factory4;
  if (COM_OK(d3d_factory, QueryInterface, &IID_IDXGIFactory4, (void **)&factory4)) {
    if (COM_OK(factory4, EnumWarpAdapter, &IID_IDXGIAdapter1, (void **)&d3d_adapter)) {
      if (0 == d3d_create_device(NULL)) return 0;
    }
  }

  // TODO: warn if no suitable device found
  return 1;
}

int d3d_init(void) {
  if (FAILED(CreateDXGIFactory2(0, &IID_IDXGIFactory3, (void **)&d3d_factory))) return 1;
  if (d3d_init_adapter()) return 1;

  return 0;
}

void d3d_deinit(void) {
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

