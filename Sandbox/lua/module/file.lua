----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: Sandbox: lua: module: file.lua
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: File Utilities ]]--
----------------------------------------------------------------


-----------------
--[[ Imports ]]--
-----------------

local imports = {
    file = file
}


---------------------
--[[ Class: File ]]--
---------------------

local file = class:create("file")

function file.public:resolve(path)
    return imports.file.resolve(path)
end

function file.public:exists(path)
    return imports.file.exists(path)
end

function file.public:size(path)
    return imports.file.size(path)
end

function file.public:delete(path)
    return imports.file.delete(path)
end

function file.public:read(path)
    return imports.file.read(path)
end

function file.public:write(path, data)
    local result = imports.file.write(path, data)
    data = nil
    imports.collectgarbage()
    return result
end