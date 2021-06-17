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

extern "C" __declspec(dllexport) void luaopen_LuaMimalloc(lua_State* L)
{
    // Save the original allocator, because previous allocations have to be handled by it
    OriginalAlloc = lua_getallocf(L, &OriginalAllocUserData);
    lua_setallocf(L, &Alloc, nullptr);
}
