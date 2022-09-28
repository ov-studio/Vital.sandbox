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

function file.public:resolve(...)
    return imports.file.resolve(...)
end

function file.public:exists(...)
    return imports.file.exists(...)
end

function file.public:size(...)
    return imports.file.size(...)
end

function file.public:delete(...)
    return imports.file.delete(...)
end

function file.public:read(...)
    return imports.file.read(...)
end

function file.public:write(path, data)
    local result = imports.file.write(path, data)
    data = nil
    imports.collectgarbage()
    return result
end

function file.public:fetchContents(...)
    return imports.file.fetchContents(...)
end
