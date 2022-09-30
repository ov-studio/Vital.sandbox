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

function crypto.public:hash(...)
    return imports.crypto.hash(...)
end

function crypto.public:encrypt(...)
    return imports.crypto.encrypt(...)
end

function crypto.public:decrypt(...)
    return imports.crypto.decrypt(...)
end
