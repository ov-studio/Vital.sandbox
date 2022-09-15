----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: lua: modules: resource.lua
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