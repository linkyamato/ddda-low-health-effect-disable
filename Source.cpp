#include <windows.h>
#include <iostream>
#include <vector>
#include <TlHelp32.h>

DWORD GetProcId(const wchar_t* procName);

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int>);

//HWND WindowHandle = FindWindowA(NULL, "Dragon's Dogma: Dark Arisen");
int main() {

//    if (WindowHandle)
//        std::cout << "window was found";
//    else std::cout << "window was not found";
    DWORD procId = GetProcId(L"DDDA.exe");
    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"DDDA.exe");
    HANDLE hProcess = 0;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
    uintptr_t instructionBaseAddress = moduleBase + 0x75C65D;
    std::cout << "module base address + offset to jump instruction: " << std::hex << instructionBaseAddress << std::endl;
    char jumpByte = 0;
    char opcode = 0xEB; // jmp
    WriteProcessMemory(hProcess, (BYTE*)instructionBaseAddress, &opcode, 1, nullptr);
    ReadProcessMemory(hProcess, (BYTE*)instructionBaseAddress, &jumpByte, sizeof(jumpByte), nullptr);
    std::cout << "1st byte at the address of the jump instruction: " << std::hex << (UINT)jumpByte << std::endl;

    return 0;
}

DWORD GetProcId(const wchar_t* procName) {
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);
        if (Process32First(hSnap, &procEntry)) {
            do {
                if (!_wcsicmp(procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap  != INVALID_HANDLE_VALUE){
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof ( modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                if (!_wcsicmp(modEntry.szModule, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
        CloseHandle(hSnap);
        return modBaseAddr;
    }
}

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets) {
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i) {
        ReadProcessMemory(hProc, (BYTE*)addr, & addr, sizeof(addr), 0);
        addr += offsets[i];
    }
    return addr;
}
