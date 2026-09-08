-- Vital.sandbox Lua 5.4 vs GDScript benchmark - Lua side.
-- ==============================================================
-- This is a normal resource: drop it in `resources/benchmark/` next
-- to any Vital.server or Vital.benchmark executable, list it under
-- `bootstrap:` in config.yaml (already done for you in
-- Vital.benchmark/config.yaml), and it runs on startup like any
-- other resource. There's no special API tying this to
-- Vital.benchmark - it's genuinely just a resource.
--
-- Nothing here is hand-tuned. Every workload calibrates itself to
-- ~TARGET_MS before it takes 7 timed samples, so numbers stay valid
-- across machines and engine builds without editing.
--
-- Results only ever leave this resource the normal way any resource
-- talks to the outside world today: core.engine.print(). The
-- "BENCH|lua|..." lines below are what Vital.benchmark's build/run
-- script (scripts/build_and_run.py) parses out of the process's
-- console output to build result.json - there is no back-channel API
-- for a resource to hand data straight to GDScript, so this is the
-- honest, currently-real way to get the numbers out.
-- ==============================================================

local TARGET_MS = 150
local SAMPLES = 7
local WARMUPS = 2
local CALIBRATION_START = 10000
local CALIBRATION_MAX = 200000000

local function now_ms()
    return core.engine.get_tick()
end

local function print_line(text)
    core.engine.print("info", text)
end

local function median(values)
    local copy = {}
    for i = 1, #values do copy[i] = values[i] end
    util.table.sort(copy)
    return copy[util.math.floor((#copy + 1) / 2)]
end

local function mean(values)
    local total = 0
    for i = 1, #values do total = total + values[i] end
    return total / #values
end

local function run_test(name, fn, start_iterations)
    local iterations = start_iterations or CALIBRATION_START
    local elapsed = 0
    local checksum = 0

    -- Warm-up / calibration.
    for _ = 1, WARMUPS do
        checksum = checksum + fn(iterations)
    end

    while elapsed < TARGET_MS and iterations < CALIBRATION_MAX do
        local t0 = now_ms()
        checksum = checksum + fn(iterations)
        local t1 = now_ms()
        elapsed = t1 - t0
        if elapsed < TARGET_MS then
            if elapsed <= 0 then
                iterations = util.math.min(iterations * 10, CALIBRATION_MAX)
            else
                local scale = TARGET_MS / elapsed
                scale = util.math.max(1.5, util.math.min(scale, 8.0))
                iterations = util.math.min(util.math.floor(iterations * scale) + 1, CALIBRATION_MAX)
            end
        end
    end

    local samples = {}
    for _ = 1, SAMPLES do
        local t0 = now_ms()
        checksum = checksum + fn(iterations)
        local t1 = now_ms()
        samples[#samples + 1] = t1 - t0
    end

    local med = median(samples)
    local avg = mean(samples)
    local ops_per_sec = med > 0 and (iterations / (med / 1000.0)) or 0

    print_line(util.string.format(
        "BENCH|lua|%s|iterations=%d|median_ms=%d|mean_ms=%.2f|ops_sec=%.0f|checksum=%.0f",
        name, iterations, med, avg, ops_per_sec, checksum
    ))

    return {
        name = name,
        iterations = iterations,
        median_ms = med,
        mean_ms = avg,
        ops_sec = ops_per_sec,
        checksum = checksum,
    }
end

-- ==============================================================
-- run_benchmark(): runs every workload and prints one BENCH line
-- per workload. Just a local function keeping the script tidy -
-- not a resource export (Vital resources don't expose one today).
-- ==============================================================
local function run_benchmark()
    local results = {}

    print_line("=== Vital.sandbox Lua benchmark ===")
    print_line("Lua runtime: bundled Lua 5.4.5")
    print_line("Timer: core.engine.get_tick() / milliseconds")
    print_line("Each workload is calibrated to about " .. TARGET_MS .. " ms before 7 measured samples.")

    -- 1) Tight arithmetic / branching.
    results[#results + 1] = run_test("arithmetic", function(n)
        local x = 0.7
        local y = 1.1
        local z = 0.0
        for i = 1, n do
            z = z + x * 1.234567 + y * 0.987654
            if z > 100000.0 then z = z * 0.5 end
            x = x + 0.000001
            y = y - 0.0000007
        end
        return z
    end)

    -- 2) Local function-call overhead.
    -- NOTE: uses the same argument shape (float, starting at 1.0) as
    -- the GDScript version so this is an apples-to-apples comparison.
    local function add3(a, b, c)
        return a + b + c
    end
    results[#results + 1] = run_test("function_calls", function(n)
        local s = 0.0
        for i = 1, n do
            s = s + add3(i + 0.0, 2.0, 3.0)
        end
        return s
    end)

    -- 3) Lua table indexed access.
    local table_data = {}
    for i = 1, 4096 do table_data[i] = i * 0.25 end
    results[#results + 1] = run_test("table_access", function(n)
        local s = 0
        local idx = 1
        for _ = 1, n do
            s = s + table_data[idx]
            idx = idx + 1
            if idx > 4096 then idx = 1 end
        end
        return s
    end)

    -- 4) Standard Lua C-library math calls.
    results[#results + 1] = run_test("math_calls", function(n)
        local s = 0
        local x = 0.001
        for _ = 1, n do
            s = s + util.math.sin(x) * util.math.cos(x * 0.37)
            x = x + 0.000001
        end
        return s
    end)

    -- 5) Vital-native binding: a cheap bound C++ function.
    results[#results + 1] = run_test("native_api_get_tick", function(n)
        local s = 0
        for _ = 1, n do
            s = s + core.engine.get_tick()
        end
        return s
    end, 1000)

    -- 6) Vital-native binding with Lua table -> Godot Vector3 conversion and return.
    results[#results + 1] = run_test("native_api_distance_3d", function(n)
        local a = {1.0, 2.0, 3.0}
        local b = {8.0, 6.0, 4.0}
        local s = 0
        for _ = 1, n do
            s = s + util.math.distance_3d(a, b)
        end
        return s
    end, 1000)

    print_line("=== Lua benchmark finished ===")

    return results
end

-- Runs as soon as the resource starts - no manual trigger needed.
run_benchmark()
