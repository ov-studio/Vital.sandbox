----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: lua: timer.lua
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Timer Utilities ]]--
----------------------------------------------------------------


-----------------
--[[ Imports ]]--
-----------------

local imports = {
    type = type,
    tonumber = tonumber,
    os = os,
    coroutine = coroutine,
    math = math
}
os, coroutine = nil, nil


----------------------
--[[ Class: Timer ]]--
----------------------

local timer = class:create("timer")

function timer.public:create(...)
    local cTimer = self:createInstance()
    if cTimer and not cTimer:load(...) then
        cTimer:destroyInstance()
        return false
    end
    return cTimer
end

function timer.public:destroy(...)
    if not timer.public:isInstance(self) then return false end
    return self:unload(...)
end

function timer.public:load(exec, interval, executions, ...)
    if not timer.public:isInstance(self) then return false end
    interval, executions = imports.tonumber(interval), imports.tonumber(executions)
    if not exec or (imports.type(exec) ~= "function") or not interval or not executions then return false end
    interval, executions = imports.math.max(1, interval), imports.math.max(0, executions)
    self.exec = exec
    self.currentExec = 0
    self.interval, self.executions = interval, executions
    self.arguments = table.pack(...)
    self.timer = imports.setTimer(function()
        self.currentExec = self.currentExec + 1
        if (self.executions > 0) and (self.currentExec >= self.executions) then
            self:destroy()
        end
        self.exec(table.unpack(self.arguments))
    end, self.interval, self.executions)
    return self
end

--TODO: MERGE
function timer:load(exec, duration, executions, ...)
    if not self or (self == timer) then return false end
     duration, executions = imports.tonumber(duration), imports.tonumber(executions)
     if not exec or (imports.type(exec) ~= "function") or not duration or not executions then return false end
     duration, executions = imports.math.max(1, duration), imports.math.max(0, executions)
     self.arguments = {...}
     imports.coroutine.create(function()
        self.currentExec = 0
         while ((executions == 0) or (self.currentExec < executions)) do
           exec(imports.table.unpack(self.arguments))
           self.currentExec = self.currentExec + 1
           imports.os.execute("sleep "..duration)
         end
     end)
     return true
end

function timer.public:unload()
    if not timer.public:isInstance(self) then return false end
    if self.timer and imports.isTimer(self.timer) then
        imports.killTimer(self.timer)
    end
    self:destroyInstance()
    return true
end