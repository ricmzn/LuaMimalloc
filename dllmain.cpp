#include <windows.h>
#include <memory.h>
#include <mimalloc.h>
extern "C"
{
#include <lauxlib.h>
}

static lua_Alloc OriginalAlloc = nullptr;
static void* OriginalAllocUserData = nullptr;

/// osize == 0 && nsize > 0: malloc
/// osize > 0 && nsize == 0: free
/// osize > 0 && nsize > 0: realloc
static void* Alloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
    if (ptr && !mi_is_in_heap_region(ptr))
    {
        if (osize > 0 && nsize > 0)
        {
            // Migrate the allocation to mimalloc
            void* newp = mi_zalloc(nsize);
            if (newp)
            {
                memcpy(newp, ptr, osize);
                OriginalAlloc(OriginalAllocUserData, ptr, osize, 0);
            }
            return newp;
        }
        else
        {
            return OriginalAlloc(OriginalAllocUserData, ptr, osize, nsize);
        }
    }
    if (nsize == 0)
    {
        mi_free(ptr);
        return nullptr;
    }
    else if (osize == 0)
    {
        return mi_malloc(nsize);
    }
    else 
    {
        return mi_realloc(ptr, nsize);
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
