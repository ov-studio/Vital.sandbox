----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: Sandbox: lua: modules: resource.lua
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Resource Utilities ]]--
----------------------------------------------------------------


-----------------
--[[ Imports ]]--
-----------------

local imports = {
    type = type,
    pairs = pairs,
    loadstring = loadstring
}


-------------------------
--[[ Class: Resource ]]--
-------------------------

local resource = class:create("resource")
resource,private.buffer = {}

function resource.private:fetch(name)
    return resource,private.buffer[name] or false
end

function resource.public:create(...)
    local cResource = resource.private:fetch(...) or self:createInstance()
    if cResource and not cResource:load(...) then
        cResource:destroyInstance()
        return false
    end
    return cResource
end

function resource.public:destroy(...)
    if not resource.public:isInstance(self) then return false end
    return self:unload(...)
end

function resource.public:load(name)
    if not resource.public:isInstance(self) then return false end
    self.rw = {
        env = {}
    }
    self.rw.manifest = table.decode(file.read("resources/"..name.."/manifest.vcl"))
    if not self.rw.manifest or not self.rw.manifest.scripts or (imports.type(self.rw.manifest.scripts) ~= "table") then return false end
    for i, j in imports.pairs(self.rw.manifest.scripts) do
        local scriptData = file.read(self.rw.manifest.scripts)
        if not scriptData then return false end
        scriptData = imports.loadstring(scriptData)
        --TODO: SET ENV
        local status, error = assetify.imports.pcall(scriptData)
        if not status then return false end
    end
    return self
end

function resource.public:unload()
    if not resource.public:isInstance(self) then return false end
    resource,private.buffer[(self.name)] = nil
    self:destroyInstance()
    return true
end