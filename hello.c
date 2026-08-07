#define WIN32_LEAN_AND_MEAN
#include <initguid.h> // Should come first

#include <d3d12.h>
#include <dxgi1_6.h>
#include <stdint.h>
#include <windows.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "user32.lib")

#define SCR_W 800
#define SCR_H 600

#define BUFFER_COUNT 2

#define COM(obj, method, ...) (obj)->lpVtbl->method(obj, __VA_ARGS__)
#define COM_OK(obj, method, ...) SUCCEEDED(COM(obj, method, __VA_ARGS__))
#define COM_CHK(obj, method, ...) if (FAILED(COM(obj, method, __VA_ARGS__))) return 1

static IDXGIFactory4             * d3d_factory;
static IDXGIAdapter1             * d3d_adapter;
static ID3D12Device              * d3d_device;
static ID3D12CommandQueue        * d3d_queue;
static IDXGISwapChain3           * d3d_swc;
static ID3D12DescriptorHeap      * d3d_rtv_heap;
static ID3D12CommandAllocator    * d3d_cmd_alloc;
static ID3D12GraphicsCommandList * d3d_cmd_list;

static ID3D12Resource * d3d_rt[BUFFER_COUNT];

static ID3D12Fence * d3d_fence;
static unsigned      d3d_frame_idx;
static HANDLE        d3d_fence_event;
static uint64_t      d3d_fence_value;

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
static inline int d3d_create_device(void) {
  return FAILED(D3D12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, (void **)&d3d_device));
}
static int d3d_init_adapter(void) {
  IDXGIFactory6 * factory6;
  if (COM_OK(d3d_factory, QueryInterface, &IID_IDXGIFactory6, (void **)&factory6)) {
    for (unsigned i = 0; d3d_enum_adapter_by_gpu(factory6, i); i++) {
      if (d3d_adapter_is_software()) continue;
      if (0 == d3d_create_device()) return 0;
    }
  }

  for (unsigned i = 0; d3d_enum_adapter(i); i++) {
    if (d3d_adapter_is_software()) continue;
    if (0 == d3d_create_device()) return 0;
  }

  COM_CHK(d3d_factory, EnumWarpAdapter, &IID_IDXGIAdapter1, (void **)&d3d_adapter);
  return d3d_create_device();
}

static int d3d_init_queue(void) {
  D3D12_COMMAND_QUEUE_DESC desc = {0};
  COM_CHK(d3d_device, CreateCommandQueue, &desc, &IID_ID3D12CommandQueue, (void **)&d3d_queue);
  return 0;
}

static int d3d_init_swapchain(HWND hwnd) {
  IDXGISwapChain1 * swc;
  DXGI_SWAP_CHAIN_DESC1 desc = {
    .Width       = SCR_W,
    .Height      = SCR_H,
    .Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount = BUFFER_COUNT,
    .SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,

    .SampleDesc = (DXGI_SAMPLE_DESC) {
      .Count = 1,
    },
  };
  COM_CHK(d3d_factory, CreateSwapChainForHwnd, (IUnknown *)d3d_queue, hwnd, &desc, NULL, NULL, &swc);
  COM_CHK(swc, QueryInterface, &IID_IDXGISwapChain3, (void **)&d3d_swc);
  return 0;
}

static int d3d_init_rtv_heap(void) {
  D3D12_DESCRIPTOR_HEAP_DESC desc = {
    .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    .NumDescriptors = BUFFER_COUNT,
  };
  COM_CHK(d3d_device, CreateDescriptorHeap, &desc, &IID_ID3D12DescriptorHeap, (void **)&d3d_rtv_heap);
  return 0;
}

typedef void (STDMETHODCALLTYPE * d3d_get_cpu_desc_t)(ID3D12DescriptorHeap *, D3D12_CPU_DESCRIPTOR_HANDLE *);
static int d3d_init_rtv(void) {
  D3D12_CPU_DESCRIPTOR_HANDLE h;
  ((d3d_get_cpu_desc_t)d3d_rtv_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(d3d_rtv_heap, &h);

  unsigned incr = COM(d3d_device, GetDescriptorHandleIncrementSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  for (int i = 0; i < BUFFER_COUNT; i++) {
    COM_CHK(d3d_swc, GetBuffer, i, &IID_ID3D12Resource, (void **)&d3d_rt[i]);
    COM(d3d_device, CreateRenderTargetView, d3d_rt[i], NULL, h);
    h.ptr += incr;
  }
  return 0;
}

static int d3d_init_cmdlist() {
  COM_CHK(d3d_device, CreateCommandList, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3d_cmd_alloc, NULL, &IID_ID3D12GraphicsCommandList, (void **)&d3d_cmd_list);
  COM_CHK(d3d_cmd_list, Close);
  return 0;
}

int d3d_init(HWND hwnd) {
  if (FAILED(CreateDXGIFactory2(0, &IID_IDXGIFactory4, (void **)&d3d_factory))) return 1;

  if (d3d_init_adapter())       return 1;
  if (d3d_init_queue())         return 1;
  if (d3d_init_swapchain(hwnd)) return 1;

  COM_CHK(d3d_factory, MakeWindowAssociation, hwnd, DXGI_MWA_NO_ALT_ENTER);

  if (d3d_init_rtv_heap()) return 1;
  if (d3d_init_rtv())      return 1;

  COM_CHK(d3d_device, CreateCommandAllocator, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, (void **)&d3d_cmd_alloc);

  if (d3d_init_cmdlist()) return 1;

  COM_CHK(d3d_device, CreateFence, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, (void **)&d3d_fence);
  d3d_frame_idx   = COM(d3d_swc, GetCurrentBackBufferIndex);
  d3d_fence_value = 1;
  d3d_fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (!d3d_fence_event) return 1;

  return 0;
}

static int d3d_wait() {
  uint64_t v = d3d_fence_value;
  COM_CHK(d3d_queue, Signal, d3d_fence, v);
  d3d_fence_value++;

  if (COM(d3d_fence, GetCompletedValue) < v) {
    COM(d3d_fence, SetEventOnCompletion, v, d3d_fence_event);
    WaitForSingleObject(d3d_fence_event, INFINITE);
  }

  d3d_frame_idx = COM(d3d_swc, GetCurrentBackBufferIndex);
  return 0;
}

static void d3d_release(void * obj) {
  if (obj) COM((IUnknown *)obj, Release);
}
void d3d_deinit(void) {
  d3d_wait();

  for (int i = 0; i < BUFFER_COUNT; i++) d3d_release(d3d_rt[i]);

  d3d_release(d3d_fence);

  d3d_release(d3d_cmd_list);
  d3d_release(d3d_cmd_alloc);
  d3d_release(d3d_rtv_heap);
  d3d_release(d3d_swc);
  d3d_release(d3d_queue);
  d3d_release(d3d_device);
  d3d_release(d3d_adapter);
  d3d_release(d3d_factory);

  CloseHandle(d3d_fence_event);
}

int d3d_frame(void) {
  COM_CHK(d3d_cmd_alloc, Reset);
  COM_CHK(d3d_cmd_list, Reset, d3d_cmd_alloc, NULL);

  COM_CHK(d3d_cmd_list, Close);

  ID3D12CommandList * cmd_list = (ID3D12CommandList *)d3d_cmd_list;
  COM(d3d_queue, ExecuteCommandLists, 1, &cmd_list);

  COM_CHK(d3d_swc, Present, 1, 0);
  d3d_wait();

  return 0;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
  switch (message) {
    case WM_PAINT:
      if (d3d_frame()) {
        PostQuitMessage(1);
        return 0;
      }
      return 0;
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
      SCR_W, SCR_H,
      NULL, NULL, h_inst, NULL);

  if (d3d_init(hwnd)) return 1;

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

