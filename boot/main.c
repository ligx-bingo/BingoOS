#include <efi.h>
#include <protocol/efi-sfsp.h>
#include <protocol/efi-lip.h>
#include <protocol/efi-gop.h>
#include <elf.h>

EFI_GUID LoadedImageGuid = {0x5b1b31a1, 0x9562, 0x11d2, {0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
EFI_GUID FileSystemGuid  = {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
EFI_GUID GopGuid         = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

typedef struct {
    UINT64 FrameBufferBase;
    UINT64 FrameBufferSize;
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 PixelsPerScanLine;
    UINT32 PixelFormat;
} KernelGopInfo;

typedef struct EFI_FILE_PROTOCOL_LOCAL EFI_FILE_PROTOCOL_LOCAL;

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This,
    OUT EFI_FILE_PROTOCOL_LOCAL **NewHandle,
    IN CHAR16 *FileName,
    IN UINT64 OpenMode,
    IN UINT64 Attributes);

typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This);

typedef EFI_STATUS (EFIAPI *EFI_FILE_DELETE_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This);

typedef EFI_STATUS (EFIAPI *EFI_FILE_READ_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This,
    IN OUT UINTN *BufferSize,
    OUT VOID *Buffer);

typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This,
    IN OUT UINTN *BufferSize,
    IN VOID *Buffer);

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_POSITION_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This,
    OUT UINT64 *Position);

typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_POSITION_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This,
    IN UINT64 Position);

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_INFO_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This,
    IN EFI_GUID *InformationType,
    IN OUT UINTN *BufferSize,
    OUT VOID *Buffer);

typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_INFO_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This,
    IN EFI_GUID *InformationType,
    IN UINTN BufferSize,
    IN VOID *Buffer);

typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH_LOCAL)(
    IN EFI_FILE_PROTOCOL_LOCAL *This);

struct EFI_FILE_PROTOCOL_LOCAL {
    UINT64 Revision;
    EFI_FILE_OPEN_LOCAL Open;
    EFI_FILE_CLOSE_LOCAL Close;
    EFI_FILE_DELETE_LOCAL Delete;
    EFI_FILE_READ_LOCAL Read;
    EFI_FILE_WRITE_LOCAL Write;
    EFI_FILE_GET_POSITION_LOCAL GetPosition;
    EFI_FILE_SET_POSITION_LOCAL SetPosition;
    EFI_FILE_GET_INFO_LOCAL GetInfo;
    EFI_FILE_SET_INFO_LOCAL SetInfo;
    EFI_FILE_FLUSH_LOCAL Flush;
};

static void PrintHex(EFI_SYSTEM_TABLE *st, UINT64 v) {
    CHAR16 buf[19];
    buf[0] = L'0'; buf[1] = L'x';
    for (int i = 0; i < 16; i++) {
        UINT8 nibble = (v >> (60 - i * 4)) & 0xF;
        buf[2 + i] = nibble < 10 ? L'0' + nibble : L'A' + nibble - 10;
    }
    buf[18] = L'\0';
    st->ConOut->OutputString(st->ConOut, buf);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Bootloader started\r\n");

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    Status = SystemTable->BootServices->HandleProtocol(
        ImageHandle, &LoadedImageGuid, (VOID **)&LoadedImage);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"HandleProtocol LoadedImage failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    Status = SystemTable->BootServices->HandleProtocol(
        LoadedImage->DeviceHandle, &FileSystemGuid, (VOID **)&FileSystem);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"HandleProtocol FileSystem failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    EFI_FILE_PROTOCOL_LOCAL *Root;
    Status = FileSystem->OpenVolume(FileSystem, (EFI_FILE_PROTOCOL **)&Root);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"OpenVolume failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    EFI_FILE_PROTOCOL_LOCAL *File;
    Status = Root->Open(Root, &File, L"\\Kernel.elf", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Open Kernel.elf failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    File->SetPosition(File, 0xFFFFFFFFFFFFFFFFULL);
    UINT64 FileSize = 0;
    File->GetPosition(File, &FileSize);

    File->SetPosition(File, 0);

    VOID *Buffer;
    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData, (UINTN)FileSize, &Buffer);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"AllocatePool buffer failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    typedef EFI_STATUS (__attribute__((ms_abi)) *READ_FN)(
        void *This,
        UINTN *BufferSize,
        void *Buffer);

    READ_FN ReadFn = (READ_FN)File->Read;
    UINTN BufferSize = (UINTN)FileSize;
    Status = ReadFn(File, &BufferSize, Buffer);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Read failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    File->Close(File);
    Root->Close(Root);

    Elf64_Ehdr *ElfHeader = (Elf64_Ehdr *)Buffer;
    if (ElfHeader->e_ident[0] != 0x7F ||
        ElfHeader->e_ident[1] != 'E' ||
        ElfHeader->e_ident[2] != 'L' ||
        ElfHeader->e_ident[3] != 'F') {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Not an ELF file\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    Elf64_Phdr *ProgramHeaders =
        (Elf64_Phdr *)((UINT8 *)Buffer + ElfHeader->e_phoff);

    EFI_PHYSICAL_ADDRESS BaseAddr = 0xFFFFFFFFFFFFFFFFULL;
    EFI_PHYSICAL_ADDRESS EndAddr = 0;
    for (Elf64_Half i = 0; i < ElfHeader->e_phnum; i++) {
        Elf64_Phdr *ph = &ProgramHeaders[i];
        if (ph->p_type == PT_LOAD) {
            if (ph->p_vaddr < BaseAddr) BaseAddr = ph->p_vaddr;
            if (ph->p_vaddr + ph->p_memsz > EndAddr) EndAddr = ph->p_vaddr + ph->p_memsz;
        }
    }

    UINTN TotalPages = (EndAddr - BaseAddr + 0xFFF) / 0x1000;
    EFI_PHYSICAL_ADDRESS KernelBase = BaseAddr;
    Status = SystemTable->BootServices->AllocatePages(
        AllocateAddress, EfiLoaderData, TotalPages, &KernelBase);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"AllocatePages failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    for (Elf64_Half i = 0; i < ElfHeader->e_phnum; i++) {
        Elf64_Phdr *ph = &ProgramHeaders[i];
        if (ph->p_type != PT_LOAD) continue;
        SystemTable->BootServices->CopyMem(
            (VOID *)ph->p_vaddr,
            (UINT8 *)Buffer + ph->p_offset,
            ph->p_filesz);
        if (ph->p_memsz > ph->p_filesz) {
            SystemTable->BootServices->SetMem(
                (UINT8 *)ph->p_vaddr + ph->p_filesz,
                ph->p_memsz - ph->p_filesz, 0);
        }
    }

    UINT64 EntryPoint = ElfHeader->e_entry;
    SystemTable->BootServices->FreePool(Buffer);

    KernelGopInfo *GopInfo;
    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData, sizeof(KernelGopInfo), (VOID **)&GopInfo);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"AllocatePool GopInfo failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    Status = SystemTable->BootServices->LocateProtocol(
        &GopGuid, 0, (VOID **)&Gop);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"GOP not found\r\n");
        GopInfo->FrameBufferBase = 0;
        GopInfo->FrameBufferSize = 0;
        GopInfo->HorizontalResolution = 0;
        GopInfo->VerticalResolution = 0;
        GopInfo->PixelsPerScanLine = 0;
        GopInfo->PixelFormat = 0;
    } else {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"GOP found\r\n");
        GopInfo->FrameBufferBase = Gop->Mode->FrameBufferBase;
        GopInfo->FrameBufferSize = Gop->Mode->FrameBufferSize;
        GopInfo->HorizontalResolution = Gop->Mode->Info->HorizontalResolution;
        GopInfo->VerticalResolution = Gop->Mode->Info->VerticalResolution;
        GopInfo->PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;
        GopInfo->PixelFormat = Gop->Mode->Info->PixelFormat;
    }

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"GopInfo FB: ");
    PrintHex(SystemTable, GopInfo->FrameBufferBase);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\r\n");

    UINTN MapSize = 0;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    SystemTable->BootServices->GetMemoryMap(
        &MapSize, 0, &MapKey, &DescriptorSize, &DescriptorVersion);
    MapSize += 2 * DescriptorSize;

    VOID *MemoryMap;
    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData, MapSize, &MemoryMap);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"AllocatePool memmap failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    Status = SystemTable->BootServices->GetMemoryMap(
        &MapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"GetMemoryMap failed\r\n");
        while (1) { __asm__ volatile("hlt"); }
    }

    while (1) {
        Status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
        if (!EFI_ERROR(Status)) break;

        MapSize = 0;
        SystemTable->BootServices->GetMemoryMap(
            &MapSize, 0, &MapKey, &DescriptorSize, &DescriptorVersion);
        MapSize += 2 * DescriptorSize;
        Status = SystemTable->BootServices->GetMemoryMap(
            &MapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        if (EFI_ERROR(Status)) {
            SystemTable->ConOut->OutputString(SystemTable->ConOut, L"GetMemoryMap failed on retry\r\n");
            while (1) { __asm__ volatile("hlt"); }
        }
    }

    typedef void (*KernelEntryPoint)(EFI_HANDLE, EFI_SYSTEM_TABLE *, KernelGopInfo *);
    KernelEntryPoint KernelMain = (KernelEntryPoint)EntryPoint;
    KernelMain(ImageHandle, SystemTable, GopInfo);

    while (1) { __asm__ volatile("hlt"); }
    return EFI_SUCCESS;
}