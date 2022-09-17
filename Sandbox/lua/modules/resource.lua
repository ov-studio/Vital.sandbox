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
    tostring = tostring,
    tonumber = tonumber
}


-------------------------
--[[ Class: Resource ]]--
-------------------------

local resource = class:create("resource")

function resource.public:create(...)
    local cResource = self:createInstance()
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

function resource.public:load()
    if not resource.public:isInstance(self) then return false end
    return self
end

function resource.public:unload()
    if not resource.public:isInstance(self) then return false end
    self:destroyInstance()
    return true
end