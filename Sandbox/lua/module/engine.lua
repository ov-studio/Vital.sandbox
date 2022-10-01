----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: Sandbox: lua: module: engine.lua
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Engine Utilities ]]--
----------------------------------------------------------------


-----------------
--[[ Imports ]]--
-----------------

local imports = {
    type = type,
    pairs = pairs
}


-----------------------
--[[ Class: Engine ]]--
-----------------------

local engine = class:create("engine", engine)
engine.private.binds = {
    key = {},
    command = {}
}

function engine.private.isBindSource(source, ref, refType, exec)
    if not source or not ref or not refType or not exec then return false end
    if (imports.type(source) ~= "string") or (imports.type(ref) ~= "string") or (imports.type(refType) ~= "string") or (imports.type(exec) ~= "function") then return false end
    if not engine.private.binds[refType] then return false end
    return true
end

function engine.private.bind(source, ref, refType, exec)
    if not engine.private.isBindSource(source, ref, refType, exec) then return false end
    engine.private.binds[refType][ref] = engine.private.binds[refType][ref] or {}
    engine.private.binds[refType][ref][source] = engine.private.binds[refType][ref][source] or {}
    if engine.private.binds[refType][ref][exec] then return false end
    engine.private.binds[refType][ref][exec] = true
    return true
end

function engine.private.unbind(source, ref, refType, exec)
    if not engine.private.isBindSource(source, ref, refType, exec) then return false end
    if not engine.private.binds[refType][ref] or not engine.private.binds[refType][ref][source] or not engine.private.binds[refType][ref][exec] then return false end
    engine.private.binds[refType][ref][exec] = nil
    return true
end

function engine.private.executeBind(ref, refType)

    if not ref or (imports.type(source) ~= "string") then return false end
    if engine.private.binds[refType][ref] then
        for i, j in imports.pairs(engine.private.binds[refType][ref]) do
            for k, v in imports.pairs(j) do
                v()
            end
        end
    end

    if not engine.private.binds[refType][ref] or not engine.private.binds[refType][ref][source] or not engine.private.binds[refType][ref][exec] then return false end
    engine.private.binds[refType][ref][exec] = nil
    return true
end

function engine.public.bindKey(...) return engine.private.bind("key", ...) end
function engine.public.unbindKey(...) return engine.private.unbind("key", ...) end
function engine.public.executeBindKey(ref) return engine.private.executeBind("key", ref) end
function engine.public.bindCommand(...) return engine.private.bind("command", ...) end
function engine.public.unbindComand(...) return engine.private.unbind(s"command", ...) end
function engine.public.executeBindCommand(...) return engine.private.executeBind("command", ...) end

