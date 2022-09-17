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
    pairs = pairs
}


-------------------------
--[[ Class: Resource ]]--
-------------------------

local resource = class:create("resource")
resource,private.buffer = {}

function resource.private:fetch(name)
    return resource,private.buffer[name] or false
end

function resource.public:create(name, ...)
    local cResource = resource.private:fetch(name)
    cResource = (cResource and resource.public:isInstance(cResource) and cResource) or self:createInstance()
    if cResource and not cResource:load(name, ...) then
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
    local resourceManifest = table.decode(file.read("resources/"..name.."/manifest.vcl"))
    if not resourceManifest then return false end
    for i, j in imports.pairs(resourceManifest.scripts) do
        --TODO: LOAD ALL SCRIPTS IN NEW ENV
    end
    return self
end

function resource.public:unload()
    if not resource.public:isInstance(self) then return false end
    self:destroyInstance()
    return true
end