----------------------------------------------------------------
--[[ Resource: Vital.sandbox
     Script: resources: benchmark: benchmark.lua
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 09/09/2026
     Desc: Lua benchmark suite ]]--
----------------------------------------------------------------

-- Vital.sandbox — Lua 5.4 benchmark side.

local math  = util.math
local str   = util.string
local tbl   = util.table
local sin   = math.sin
local cos   = math.cos
local floor = math.floor
local min   = math.min
local max   = math.max

local TARGET_MS         = 150
local SAMPLES           = 7
local WARMUPS           = 2
local CALIBRATION_START = 10000
local CALIBRATION_MAX   = 200000000

local function now_ms()      return core.engine.get_tick() end
local function print_line(t) core.engine.print("info", t)  end

local function median(values)
    local copy = {}
    for i = 1, #values do copy[i] = values[i] end
    tbl.sort(copy)
    return copy[floor((#copy + 1) / 2)]
end

local function mean(values)
    local total = 0
    for i = 1, #values do total = total + values[i] end
    return total / #values
end

local function run_test(name, fn, start_iterations)
    local iterations = start_iterations or CALIBRATION_START
    local elapsed    = 0
    local checksum   = 0

    for _ = 1, WARMUPS do checksum = checksum + fn(iterations) end

    while elapsed < TARGET_MS and iterations < CALIBRATION_MAX do
        local t0 = now_ms()
        checksum = checksum + fn(iterations)
        elapsed  = now_ms() - t0
        if elapsed < TARGET_MS then
            if elapsed <= 0 then
                iterations = min(iterations * 10, CALIBRATION_MAX)
            else
                local scale = max(1.5, min(TARGET_MS / elapsed, 8.0))
                iterations  = min(floor(iterations * scale) + 1, CALIBRATION_MAX)
            end
        end
    end

    local samples = {}
    for _ = 1, SAMPLES do
        local t0 = now_ms()
        checksum = checksum + fn(iterations)
        samples[#samples + 1] = now_ms() - t0
    end

    local med     = median(samples)
    local avg     = mean(samples)
    local ops_sec = med > 0 and (iterations / (med / 1000.0)) or 0

    print_line(str.format(
        "BENCH|lua|%s|iterations=%d|median_ms=%d|mean_ms=%.2f|ops_sec=%.0f|checksum=%.4f",
        name, iterations, med, avg, ops_sec, checksum
    ))

    return { name=name, iterations=iterations, median_ms=med, mean_ms=avg,
             ops_sec=ops_sec, checksum=checksum }
end

local function run_benchmark()
    local results = {}
    local function add(r) results[#results + 1] = r end

    print_line("=== Vital.sandbox Lua benchmark ===")
    print_line("Lua runtime: bundled Lua 5.4")
    print_line("Timer: core.engine.get_tick() / milliseconds")
    print_line(str.format("Calibrating to ~%d ms, %d samples.", TARGET_MS, SAMPLES))

    add(run_test("arithmetic", function(n)
        local x, y, z = 0.7, 1.1, 0.0
        for _ = 1, n do
            z = z + x * 1.234567 + y * 0.987654
            if z > 100000.0 then z = z * 0.5 end
            x = x + 0.000001; y = y - 0.0000007
        end
        return z
    end))

    local function add3(a, b, c) return a + b + c end
    add(run_test("function_calls", function(n)
        local s = 0.0
        for i = 1, n do s = s + add3(i + 0.0, 2.0, 3.0) end
        return s
    end))

    local tdata = {}
    for i = 1, 4096 do tdata[i] = (i + 1) * 0.25 end
    add(run_test("table_access", function(n)
        local s, idx = 0, 1
        for _ = 1, n do
            s = s + tdata[idx]; idx = idx + 1
            if idx > 4096 then idx = 1 end
        end
        return s
    end))

    add(run_test("math_calls", function(n)
        local s, x = 0, 0.001
        for _ = 1, n do
            s = s + sin(x) * cos(x * 0.37); x = x + 0.000001
        end
        return s
    end))

    add(run_test("string_ops", function(n)
        local s = 0
        for i = 1, n do
            local st = str.format("entity_%d", i)
            s = s + #st + str.byte(st, 1)
        end
        return s
    end))

    add(run_test("table_construction", function(n)
        local s = 0
        for i = 1, n do
            local t = { x = i * 0.1, y = i * 0.2, hp = 100, id = i }
            s = s + t.x + t.hp
        end
        return s
    end))

    add(run_test("closures", function(n)
        local s = 0
        for i = 1, n do
            local base = i * 0.5
            local fn   = function(x) return base + x end
            s = s + fn(1.0)
        end
        return s
    end))

    local function sum_varargs(...) local s=0; for _,v in ipairs({...}) do s=s+v end; return s end
    add(run_test("varargs", function(n)
        local s = 0
        for i = 1, n do s = s + sum_varargs(i, i+1, i+2, i+3) end
        return s
    end))

    local iter_data = {}
    for i = 1, 64 do iter_data[i] = i * 1.5 end
    add(run_test("table_iteration", function(n)
        local s = 0
        for _ = 1, n do
            for _, v in ipairs(iter_data) do s = s + v end
        end
        return s
    end, 1000))

    local entities = {}
    for i = 1, 1000 do
        entities[i] = { x=i*0.1, y=i*0.2, vx=0.01, vy=0.02, hp=100, id=i }
    end
    add(run_test("entity_simulation", function(n)
        local s = 0
        for _ = 1, n do
            for _, e in ipairs(entities) do
                e.x = e.x + e.vx; e.y = e.y + e.vy
                e.hp = e.hp - 0.001; s = s + e.x
            end
        end
        return s
    end, 100))

    print_line("=== Lua benchmark finished ===")
    util.event.emit_native("benchmark:lua:complete", results)
    return results
end

run_benchmark()
