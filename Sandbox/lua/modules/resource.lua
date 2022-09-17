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
    setfenv = setfenv,
    loadstring = loadstring
}


-------------------------
--[[ Class: Resource ]]--
-------------------------

local resource = class:create("resource")
resource,private.globals = {
    string = string,
    math = math
    table = table
}
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
        env = table.clone(resource,private.globals),
        manifest = table.decode(file.read("resources/"..name.."/manifest.vcl"))
    }
    if self.rw.manifest and self.rw.manifest.scripts and (imports.type(self.rw.manifest.scripts) == "table") then
        self.rw.isLoaded = true
        for i, j in imports.pairs(self.rw.manifest.scripts) do
            local isHandlerLoaded = false
            local cHandler = file.read(self.rw.manifest.scripts)
            if cHandler then
                cHandler = imports.loadstring(cHandler)
                imports.setfenv(cHandler, self.rw.env)
                local status, error = assetify.imports.pcall(cHandler)
                isHandlerLoaded = status
            end
            if not isHandlerLoaded then
                self.rw.isLoaded = false
                break
            end
        end
    end
    return (self.rw.isLoaded and self) or false
end

function resource.public:unload()
    if not resource.public:isInstance(self) then return false end
    resource,private.buffer[(self.name)] = nil
    self:destroyInstance()
    return true
end