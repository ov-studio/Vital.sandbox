----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: Sandbox: lua: module: engine.lua
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Engine Utilities ]]--
----------------------------------------------------------------


-----------------------
--[[ Class: Engine ]]--
-----------------------

local engine = class:create("engine", engine)
engine.private.binds = {
    key = {},
    command = {}
}

function engine.public.bindKey(source, ref, exec)
    engine.private.binds.key[ref] = engine.private.binds.key[ref] or {}
    engine.private.binds.key[source] = engine.private.binds.key[source] or {}
    if engine.private.binds.key[exec] then return false end
    engine.private.binds.key[exec] = engine.private.binds.key[exec] or true
    return true
end

function engine.public.unbindKey(source, ref, exec)
    if not engine.private.binds.key[ref] or not engine.private.binds.key[source] or not engine.private.binds.key[exec] then return false end
    engine.private.binds.key[exec] = nil
    return true
end

function engine.public.bindCommand(source, ref, exec)
    engine.private.binds.command[ref] = engine.private.binds.command[ref] or {}
    engine.private.binds.command[source] = engine.private.binds.command[source] or {}
    if engine.private.binds.command[exec] then return false end
    engine.private.binds.command[exec] = engine.private.binds.command[exec] or true
    return true
end

function engine.public.unbindComand(source, ref, exec)
    if not engine.private.binds.command[ref] or not engine.private.binds.command[source] or not engine.private.binds.command[exec] then return false end
    engine.private.binds.command[exec] = nil
    return true
end