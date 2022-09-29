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

function crypto.public:sha1(...)
    return imports.crypto.sha1(...)
end

function crypto.public:sha224(...)
    return imports.crypto.sha224(...)
end

function crypto.public:sha256(...)
    return imports.crypto.sha256(...)
end

function crypto.public:sha384(...)
    return imports.crypto.sha384(...)
end

function crypto.public:sha512(...)
    return imports.crypto.sha512(...)
end
