################################################################
#     Resource: Vital.sandbox
#     Script: benchmark.gd
#     Author: ov-studio
#     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
#     DOC: 09/09/2026
#     Desc: GDScript benchmark suite
################################################################

const CALIBRATION_START := 10000
const CALIBRATION_MAX   := 200000000

var TARGET_MS := 150
var SAMPLES   := 7
var WARMUPS   := 2

func median(values: Array) -> float:
	var copy := values.duplicate()
	copy.sort()
	return float(copy[copy.size() / 2])

func mean(values: Array) -> float:
	var total := 0.0
	for v in values: total += float(v)
	return total / values.size()

func run_test(name: String, fn: Callable, start_iterations: int = CALIBRATION_START) -> Dictionary:
	var iterations := start_iterations
	var elapsed    := 0
	var checksum   := 0.0

	for _w in range(WARMUPS): checksum += fn.call(iterations)

	while elapsed < TARGET_MS and iterations < CALIBRATION_MAX:
		var t0 := Time.get_ticks_msec()
		checksum += fn.call(iterations)
		elapsed   = Time.get_ticks_msec() - t0
		if elapsed < TARGET_MS:
			if elapsed <= 0:
				iterations = mini(iterations * 10, CALIBRATION_MAX)
			else:
				var scale := clampf(float(TARGET_MS) / float(elapsed), 1.5, 8.0)
				iterations = mini(int(float(iterations) * scale) + 1, CALIBRATION_MAX)

	var samples: Array = []
	for _s in range(SAMPLES):
		var t0 := Time.get_ticks_msec()
		checksum += fn.call(iterations)
		samples.append(Time.get_ticks_msec() - t0)

	var med     := median(samples)
	var avg     := mean(samples)
	var ops_sec := (float(iterations) / (med / 1000.0)) if med > 0 else 0.0

	return { "name": name, "iterations": iterations, "median_ms": med,
	         "mean_ms": avg, "ops_sec": ops_sec, "checksum": checksum }

func run_benchmark(config: Dictionary) -> Array:
	TARGET_MS = config["target_ms"]
	SAMPLES   = config["samples"]
	WARMUPS   = config["warmups"]
	var results: Array = []

	results.append(run_test("arithmetic", func(n: int) -> float:
		var x := 0.7; var y := 1.1; var z := 0.0
		for _i in range(n):
			z += x * 1.234567 + y * 0.987654
			if z > 100000.0: z *= 0.5
			x += 0.000001; y -= 0.0000007
		return z
	))

	results.append(run_test("function_calls", func(n: int) -> float:
		var s := 0.0
		for i in range(n): s += _add3(float(i), 2.0, 3.0)
		return s
	))

	var tdata: Array[float] = []; tdata.resize(4096)
	for i in range(4096): tdata[i] = (i + 1) * 0.25
	results.append(run_test("table_access", func(n: int) -> float:
		var s := 0.0; var idx := 0
		for _i in range(n):
			s += tdata[idx]; idx += 1
			if idx >= 4096: idx = 0
		return s
	))

	results.append(run_test("math_calls", func(n: int) -> float:
		var s := 0.0; var x := 0.001
		for _i in range(n):
			s += sin(x) * cos(x * 0.37); x += 0.000001
		return s
	))

	results.append(run_test("string_ops", func(n: int) -> float:
		var s := 0.0
		for i in range(n):
			var st := "entity_%d" % i
			s += st.length() + st.unicode_at(0)
		return s
	))

	results.append(run_test("table_construction", func(n: int) -> float:
		var s := 0.0
		for i in range(n):
			var t := { "x": i * 0.1, "y": i * 0.2, "hp": 100, "id": i }
			s += t["x"] + t["hp"]
		return s
	))

	results.append(run_test("closures", func(n: int) -> float:
		var s := 0.0
		for i in range(n):
			var base := i * 0.5
			var fn   := func(x: float) -> float: return base + x
			s += fn.call(1.0)
		return s
	))

	results.append(run_test("varargs", func(n: int) -> float:
		var s := 0.0
		for i in range(n): s += _sum_varargs([i, i+1, i+2, i+3])
		return s
	))

	var iter_data: Array[float] = []; iter_data.resize(64)
	for i in range(64): iter_data[i] = (i + 1) * 1.5
	results.append(run_test("table_iteration", func(n: int) -> float:
		var s := 0.0
		for _i in range(n):
			for v in iter_data: s += v
		return s
	, 1000))

	var entities: Array[Dictionary] = []
	for i in range(1000):
		entities.append({ "x": i*0.1, "y": i*0.2, "vx": 0.01, "vy": 0.02, "hp": 100.0, "id": i })
	results.append(run_test("entity_simulation", func(n: int) -> float:
		var s := 0.0
		for _i in range(n):
			for e in entities:
				e["x"] = e["x"] + e["vx"]; e["y"] = e["y"] + e["vy"]
				e["hp"] = e["hp"] - 0.001; s += e["x"]
		return s
	, 100))

	return results

func _add3(a: float, b: float, c: float) -> float: return a + b + c

func _sum_varargs(args: Array) -> float:
	var s := 0.0
	for v in args: s += float(v)
	return s
