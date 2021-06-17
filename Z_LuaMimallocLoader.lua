local function print(str)
    log.write("LuaMimallocLoader", log.INFO, str)
end

print("Loading LuaMimalloc...")
package.cpath = package.cpath..";"..lfs.writedir().."/Scripts/Hooks/LuaMimalloc.dll"
require("LuaMimalloc")
print("LuaMimalloc.dll loaded")

local callbacks = {}
function callbacks.onSimulationStart()
    print("onSimulationStart")
    if not DCS.isServer() then
        print("Joining mission as client, not hooking LuaMimalloc")
        return nil
    end
    local res = net.dostring_in("server", [[
        if require == nil then
            err("Cannot load LuaMimalloc because Lua environment is sanitized (`require` is nil)")
        end
        package.cpath = package.cpath..";"..lfs.writedir().."/Scripts/Hooks/LuaMimalloc.dll"
        require("LuaMimalloc")
        env.info("Loaded LuaMimalloc in mission")
    ]])
    if res ~= nil and res ~= '' then
        print("Error while trying to hook LuaMimalloc in mission: "..tostring(res))
    end
end

DCS.setUserCallbacks(callbacks)
