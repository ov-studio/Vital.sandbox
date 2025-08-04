----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: Sandbox: lua: module: rest.lua
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: REST Utilities ]]--
----------------------------------------------------------------


-----------------
--[[ Imports ]]--
-----------------

local imports = {
    type = type,
    tonumber = tonumber,
    --TODO: Implement http requests
    fetchRemote = fetchRemote
}


---------------------
--[[ Class: REST ]]--
---------------------

--[[
local rest = class:create("rest")

function rest.public:get(route, timeout, headers, credentials)
    if self ~= rest.public then return false end
    if not route or (imports.type(route) ~= "string") then return false end
    timeout = math.max(imports.tonumber(timeout) or 10000, 1)
    local cPromise = thread:createPromise()
    imports.fetchRemote(route, {queueName = "assetify_library", connectionAttempts = 1, connectTimeout = timeout, method = "GET", headers = headers, username = (credentials and credentials.username) or nil, password = (credentials and credentials.password) or nil}, function(result, status)
        if status.success then cPromise.resolve(result)
        else cPromise.reject(false, status.statusCode) end
    end)
    return cPromise
end

function rest.public:post(route, data, timeout, headers, credentials)
    if (self ~= rest.public) or not data then return false end
    if not route or (imports.type(route) ~= "string") then return false end
    timeout = math.max(imports.tonumber(timeout) or 10000, 1)
    headers = ((imports.type(headers) == "table") and headers) or nil
    credentials = ((imports.type(credentials) == "table") and credentials) or false
    local cPromise = thread:createPromise()
    imports.fetchRemote(route, {queueName = "assetify_library", connectionAttempts = 1, connectTimeout = timeout, method = "POST", headers = headers, username = (credentials and credentials.username) or nil, password = (credentials and credentials.password) or nil, formFields = data}, function(result, status)
        if status.success then cPromise.resolve(result)
        else cPromise.reject(false, status.statusCode) end
    end)
    return cPromise
end
]]
