local function log(str)
    net.log(str)
end

log("Loading LuaMimalloc...")
package.cpath = package.cpath..";"..lfs.writedir().."/Scripts/Hooks/LuaMimalloc.dll"
require("LuaMimalloc")
log("LuaMimalloc.dll loaded")

local callbacks = {}
function callbacks.onSimulationStart()
    log("onSimulationStart")
    if not DCS.isServer() then
        log("Joining mission as client, not hooking LuaMimalloc")
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
        log("Error while trying to hook LuaMimalloc in mission: "..tostring(res))
    end
end

DCS.setUserCallbacks(callbacks)
