#include <windows.h>
#include <memory.h>
#include <math.h>
#include <mimalloc.h>
extern "C"
{
#include <lauxlib.h>
}

static lua_Alloc OriginalAlloc = nullptr;
static void* OriginalAllocUserData = nullptr;

/// oldSize == 0 && newSize > 0: malloc
/// oldSize > 0 && newSize == 0: free
/// oldSize > 0 && newSize > 0: realloc
static void* Alloc(void* unused, void* ptr, size_t oldSize, size_t newSize)
{
    if (ptr && !mi_is_in_heap_region(ptr))
    {
        if (oldSize > 0 && newSize > 0)
        {
            // Migrate the allocation to mimalloc
            void* newPtr = mi_malloc(newSize);
            if (newPtr)
            {
                memcpy(newPtr, ptr, min(oldSize, newSize));
                OriginalAlloc(OriginalAllocUserData, ptr, oldSize, 0);
            }
            return newPtr;
        }
        else
        {
            return OriginalAlloc(OriginalAllocUserData, ptr, oldSize, newSize);
        }
    }
    if (newSize == 0)
    {
        mi_free(ptr);
        return nullptr;
    }
    else if (oldSize == 0)
    {
        return mi_malloc(newSize);
    }
    else
    {
        return mi_realloc(ptr, newSize);
    }
}

extern "C" __declspec(dllexport) int luaopen_LuaMimalloc(lua_State* L)
{
    // Save the original allocator, because previous allocations have to be handled by it
    OriginalAlloc = lua_getallocf(L, &OriginalAllocUserData);
    lua_setallocf(L, &Alloc, nullptr);
    return 0;
}

BOOL DllMain(HMODULE module, DWORD reason, void* reserved)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            // Pin the DLL in memory so it's still loaded when Lua frees its memory
            HMODULE mod;
            GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)module, &mod);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
