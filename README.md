## Instructions

* Make sure you have Visual Studio 2017 or 2019 installed with C++ build tools

* Clone and build mimalloc (https://github.com/microsoft/mimalloc) and Lua 5.1 (https://www.lua.org/ftp/lua-5.1.5.tar.gz) in the parent directory, as static libraries

    - To build Lua 5.1 in Visual Studio, create a DLL project, add all files in `src/`, and delete `luac.c`

* Build LuaMimalloc.dll (this project)

* Copy the built .dll and Z_LuaMimallocLoader.lua to `Saved Games/DCS_server/Scripts/Hooks`

* To enable the use of the custom allocator in missions, the mission environment also needs to be unsanitized on the server (edit `Program Files/Eagle Dynamics/DCS World Dedicated Server/Scripts/MissionScripting.lua`)

* Optional: enable the allocator in Export scripts by adding the following to the start of `Saved Games/Scripts/Export.lua`:
```lua
xpcall(function()
    package.cpath = package.cpath..";"..lfs.writedir().."/Scripts/Hooks/?.dll"
    require("LuaMimalloc")
    log.write("Export.lua", log.INFO, "LuaMimalloc.dll loaded")
end, function(err)
    log.write("Export.lua", log.ERROR, "Failed to load LuaMimalloc.dll: "..tostring(err))
end)
```

Note: build paths are set up for x64 only

## Known issues

* The loader lua needs to start with Z to make sure it runs after Tacview, otherwise Tacview fails to load its own DLL for unknown reasons
