extends Node

# Vital.sandbox Lua vs GDScript benchmark - GDScript side.
# ==============================================================
# This file lives on disk next to the Vital.benchmark executable and
# is loaded at runtime (see benchmark_runner.gd). Anyone can open it,
# read exactly what runs, edit it, and re-run - that's the point.
#
# IMPORTANT: this uses the SAME calibration methodology as
# resources/benchmark/benchmark.lua (target ~150ms per workload, 7
# timed samples, 2 warmups) instead of hardcoded iteration counts
# copied from a past run. That means these numbers stay valid on any
# machine, any engine build, forever - nothing to keep in sync by
# hand.
# ==============================================================

const TARGET_MS := 150
const SAMPLES := 7
const WARMUPS := 2
const CALIBRATION_START := 10000
const CALIBRATION_MAX := 200_000_000


func now_ms() -> int:
	return Time.get_ticks_msec()


func median(values: Array[int]) -> float:
	var copy: Array[int] = values.duplicate()
	copy.sort()
	var middle := copy.size() / 2
	if copy.size() % 2 == 0:
		return (float(copy[middle - 1]) + float(copy[middle])) * 0.5
	return float(copy[middle])


func mean(values: Array[int]) -> float:
	var total := 0
	for value in values:
		total += value
	return float(total) / float(values.size())


func run_test(name: String, fn: Callable, start_iterations: int = CALIBRATION_START) -> Dictionary:
	var iterations := start_iterations
	var elapsed := 0
	var checksum := 0.0

	# Warm-up / calibration (mirrors the Lua harness exactly).
	for _warmup in WARMUPS:
		checksum += float(fn.call(iterations))

	while elapsed < TARGET_MS and iterations < CALIBRATION_MAX:
		var c0 := now_ms()
		checksum += float(fn.call(iterations))
		var c1 := now_ms()
		elapsed = c1 - c0
		if elapsed < TARGET_MS:
			if elapsed <= 0:
				iterations = mini(iterations * 10, CALIBRATION_MAX)
			else:
				var scale: float = clamp(float(TARGET_MS) / float(elapsed), 1.5, 8.0)
				iterations = mini(int(iterations * scale) + 1, CALIBRATION_MAX)

	var samples: Array[int] = []
	for _sample in SAMPLES:
		var t0 := now_ms()
		checksum += float(fn.call(iterations))
		var t1 := now_ms()
		samples.append(t1 - t0)

	var med := median(samples)
	var avg := mean(samples)
	var ops_per_sec := 0.0
	if med > 0.0:
		ops_per_sec = float(iterations) / (med / 1000.0)

	print(
		"BENCH|gdscript|%s|iterations=%d|median_ms=%.1f|mean_ms=%.2f|ops_sec=%.0f|checksum=%.0f"
		% [name, iterations, med, avg, ops_per_sec, checksum]
	)

	return {
		"name": name,
		"iterations": iterations,
		"median_ms": med,
		"mean_ms": avg,
		"ops_sec": ops_per_sec,
		"checksum": checksum,
	}


func add3(a: float, b: float, c: float) -> float:
	return a + b + c


# ==============================================================
# run_all(): runs every workload and returns an Array[Dictionary].
# Called directly by benchmark_runner.gd - no stdout parsing needed
# since this script lives in the same process.
# ==============================================================
func run_all() -> Array:
	var results: Array = []

	print("=== GDScript benchmark ===")
	print("Timer: Time.get_ticks_msec() / milliseconds")
	print("Each workload is calibrated to about %d ms before 7 measured samples." % TARGET_MS)

	var table_data: Array[float] = []
	table_data.resize(4096)
	for i in 4096:
		table_data[i] = float(i + 1) * 0.25

	# 1) Tight arithmetic / branching. Identical workload shape to Lua.
	results.append(run_test("arithmetic", func(n: int) -> float:
		var x := 0.7
		var y := 1.1
		var z := 0.0
		for i in n:
			z += x * 1.234567 + y * 0.987654
			if z > 100000.0:
				z *= 0.5
			x += 0.000001
			y -= 0.0000007
		return z
	))

	# 2) Local function-call overhead.
	# NOTE: argument shape (float, starting at 1.0) matches the Lua
	# version exactly - this used to be off-by-one vs the Lua script
	# and is fixed here so the comparison is fair.
	results.append(run_test("function_calls", func(n: int) -> float:
		var s := 0.0
		for i in n:
			s += add3(float(i + 1), 2.0, 3.0)
		return s
	))

	# 3) GDScript Array indexed access - conceptual equivalent of
	# Vital's Lua table-access test.
	results.append(run_test("table_access", func(n: int) -> float:
		var s := 0.0
		var idx := 0
		for _i in n:
			s += table_data[idx]
			idx += 1
			if idx >= 4096:
				idx = 0
		return s
	))

	# 4) Built-in math calls. Mirrors Lua's util.math.sin/cos test.
	results.append(run_test("math_calls", func(n: int) -> float:
		var s := 0.0
		var x := 0.001
		for _i in n:
			s += sin(x) * cos(x * 0.37)
			x += 0.000001
		return s
	))

	# 5) Native-ish engine call. NOT the same Vital binding - measures
	# the cost of calling Godot's own built-in time API from GDScript,
	# as the closest ordinary equivalent to core.engine.get_tick().
	results.append(run_test("native_api_get_tick", func(n: int) -> float:
		var s := 0.0
		for _i in n:
			s += float(Time.get_ticks_msec())
		return s
	, 1000))

	# 6) 3D distance. Semantic equivalent, not a direct binding
	# equivalence: Vital's Lua version also pays for Lua table ->
	# Vector3 conversion, which plain GDScript never has to do.
	var a := Vector3(1.0, 2.0, 3.0)
	var b := Vector3(8.0, 6.0, 4.0)
	results.append(run_test("native_api_distance_3d", func(n: int) -> float:
		var s := 0.0
		for _i in n:
			s += a.distance_to(b)
		return s
	, 1000))

	print("=== GDScript benchmark finished ===")
	return results


# Lets someone drop this file alone onto a plain Node in a normal
# Godot 4 project and just run it standalone to sanity-check
# GDScript-only numbers, without needing Vital.benchmark at all.
#
# NOTE: when Vital.benchmark loads this script dynamically
# (see benchmark_runner.gd), it calls run_all() directly instead of
# adding this to the scene tree, so _ready() never fires there and
# you won't get a double run.
func _ready() -> void:
	run_all()
