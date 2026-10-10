#include <stdint.h>

typedef struct {
    uint64_t FrameBufferBase;
    uint64_t FrameBufferSize;
    uint32_t HorizontalResolution;
    uint32_t VerticalResolution;
    uint32_t PixelsPerScanLine;
    uint32_t PixelFormat;
} KernelGopInfo;

static KernelGopInfo LocalGopInfo;

__attribute__((ms_abi))
void kernel_main(void *ImageHandle, void *SystemTable, KernelGopInfo *GopInfo) {
    (void)ImageHandle;
    (void)SystemTable;
    LocalGopInfo.FrameBufferBase = GopInfo->FrameBufferBase;
    LocalGopInfo.FrameBufferSize = GopInfo->FrameBufferSize;
    LocalGopInfo.HorizontalResolution = GopInfo->HorizontalResolution;
    LocalGopInfo.VerticalResolution = GopInfo->VerticalResolution;
    LocalGopInfo.PixelsPerScanLine = GopInfo->PixelsPerScanLine;
    LocalGopInfo.PixelFormat = GopInfo->PixelFormat;

    

    while (1) {
        __asm__ volatile("hlt");
    }
}