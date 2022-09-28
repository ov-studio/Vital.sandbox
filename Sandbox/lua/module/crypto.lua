----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: Sandbox: lua: module: crypto.lua
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Crypto Utilities ]]--
----------------------------------------------------------------


-----------------
--[[ Imports ]]--
-----------------

local imports = {
    crypto = crypto
}


-----------------------
--[[ Class: Crypto ]]--
-----------------------

local crypto = class:create("crypto")

function crypto.public:sha256(path)
    return imports.crypto.sha256(path)
end