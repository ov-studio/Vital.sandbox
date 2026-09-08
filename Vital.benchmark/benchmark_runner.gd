extends Node

# ==============================================================
# Vital.benchmark entry point.
#
# Core boots the Lua sandbox and bootstraps resources (see
# config.yaml's `bootstrap: - "benchmark"`) on its own background
# thread, and nothing in Vital.core today exposes a GDScript-callable
# way to ask a resource for its results or even know when it finished
# (Core only binds `drain()` - checked against the actual engine
# source, not guessed). So this script:
#
#   1. Runs the GDScript half directly, in-process - just a function
#      call, results captured as real Dictionaries, no parsing.
#   2. Gives the Lua half (running on Core's background thread) a
#      fixed, generous window to finish printing its own
#      "BENCH|lua|..." lines.
#   3. Reads those lines back out of Godot's own log file (enabled in
#      project.godot: debug/file_logging/enable_file_logging) rather
#      than trying to read its own live stdout, which isn't something
#      a process can generally do to itself.
#   4. Writes result.json next to the executable and quits - no
#      external tooling required for the basic case.
# ==============================================================

# Comfortably above worst-case time for 6 workloads x ~9 runs x
# ~150ms each, per language, PLUS a cold-cache Vital.kit download on
# first run (observed ~8s extra). Bump this further if you add slower
# workloads and result.json ends up missing tests.
const WAIT_FOR_LUA_SECONDS := 60.0

const GDSCRIPT_BENCHMARK_PATH := "gdscript/benchmark.gd"

const LUA_BENCH_PATTERN := "BENCH\\|lua\\|(?<name>[^|]+)\\|iterations=(?<iterations>\\d+)\\|median_ms=(?<median_ms>[\\d.]+)\\|mean_ms=(?<mean_ms>[\\d.]+)\\|ops_sec=(?<ops_sec>[\\d.]+)\\|checksum=(?<checksum>-?[\\d.]+)"


func _ready() -> void:
	var base_dir := OS.get_executable_path().get_base_dir()
	if OS.has_feature("editor"):
		# Running via `godot --headless --path Vital.benchmark` for
		# local testing rather than the exported binary.
		base_dir = ProjectSettings.globalize_path("res://")

	var gd_path := base_dir.path_join(GDSCRIPT_BENCHMARK_PATH)
	var result_path := base_dir.path_join("result.json")

	print("Vital.benchmark")
	print("Lua side:      resources/benchmark/ (auto-started via config.yaml bootstrap)")
	print("GDScript side: " + gd_path)
	print("")

	var gd_results := run_gdscript_benchmark(gd_path)

	print("")
	print("Waiting up to %.0fs for the Lua resource to finish on its own thread..." % WAIT_FOR_LUA_SECONDS)
	await get_tree().create_timer(WAIT_FOR_LUA_SECONDS).timeout

	var lua_results := await read_lua_results_from_log()
	var report := build_report(lua_results, gd_results)
	write_result_json(result_path, report)

	print_summary(report)
	print("")
	print("Wrote " + result_path)
	get_tree().quit()


## Loads gdscript/benchmark.gd from disk and runs it, returning its
## Array[Dictionary] results directly - no parsing needed for this
## half. Intentionally excluded from the exported .pck (see
## export_presets.cfg's exclude_filter) so it stays a plain, readable,
## editable text file next to the executable.
func run_gdscript_benchmark(script_path: String) -> Array:
	if not FileAccess.file_exists(script_path):
		push_error("GDScript benchmark not found at: " + script_path)
		return []

	var src := FileAccess.get_file_as_string(script_path)
	var gd_script := GDScript.new()
	gd_script.source_code = src
	var reload_err := gd_script.reload()
	if reload_err != OK:
		push_error("Failed to compile %s (error %d)" % [script_path, reload_err])
		return []

	# Not added to the scene tree on purpose: run_all() is called
	# directly instead, so this instance's own _ready() never fires
	# and the benchmark doesn't run twice.
	var instance = gd_script.new()
	var results: Array = instance.run_all()
	instance.free()
	return results


## Reads Godot's own log file (real-time, since file_logging is on)
## and pulls out every "BENCH|lua|..." line printed by the resource.
## This is the only channel a resource can currently reach the
## outside world through - there's no GDScript-facing API to call
## into one directly (see the header comment).
func read_lua_results_from_log() -> Array:
	var log_setting: String = ProjectSettings.get_setting("debug/file_logging/log_path", "user://logs/godot.log")
	var log_path := ProjectSettings.globalize_path(log_setting)

	var log_text := ""
	for attempt in 3:
		if FileAccess.file_exists(log_path):
			log_text = FileAccess.get_file_as_string(log_path)
			if not log_text.is_empty():
				break
		await get_tree().create_timer(0.5).timeout

	if log_text.is_empty():
		push_warning("Log file not found/empty at " + log_path + " - Lua results will be missing from result.json")
		return []
	var regex := RegEx.new()
	regex.compile(LUA_BENCH_PATTERN)

	var results: Array = []
	for m in regex.search_all(log_text):
		results.append({
			"name": m.get_string("name"),
			"iterations": m.get_string("iterations").to_int(),
			"median_ms": m.get_string("median_ms").to_float(),
			"mean_ms": m.get_string("mean_ms").to_float(),
			"ops_sec": m.get_string("ops_sec").to_float(),
			"checksum": m.get_string("checksum").to_float(),
		})

	if results.is_empty():
		push_warning("No BENCH|lua| lines found in " + log_path + " - did the resource fail to start?")

	return results


func build_report(lua_results: Array, gd_results: Array) -> Dictionary:
	var by_name: Dictionary = {}
	for r in lua_results:
		by_name[r["name"]] = {"lua": r}
	for r in gd_results:
		if not by_name.has(r["name"]):
			by_name[r["name"]] = {}
		by_name[r["name"]]["gdscript"] = r

	var tests: Array = []
	var log_sum := 0.0
	var log_count := 0

	for test_name in by_name.keys():
		var entry: Dictionary = by_name[test_name]
		var row: Dictionary = {"name": test_name}

		if entry.has("lua"):
			row["lua"] = entry["lua"]
		if entry.has("gdscript"):
			row["gdscript"] = entry["gdscript"]

		if entry.has("lua") and entry.has("gdscript"):
			var lua_ms: float = entry["lua"]["median_ms"]
			var gd_ms: float = entry["gdscript"]["median_ms"]
			if lua_ms > 0.0 and gd_ms > 0.0:
				var faster := "lua" if lua_ms < gd_ms else "gdscript"
				var ratio: float = max(lua_ms, gd_ms) / min(lua_ms, gd_ms)
				row["faster"] = faster
				row["speedup"] = ratio
				log_sum += log(ratio)
				log_count += 1

		tests.append(row)

	var geomean := exp(log_sum / float(log_count)) if log_count > 0 else 0.0

	return {
		"generated_at_unix": Time.get_unix_time_from_system(),
		"methodology": {
			"target_ms": 150,
			"samples": 7,
			"warmups": 2,
			"note": "Each workload self-calibrates its iteration count to run for ~target_ms before timing. No iteration counts are hand-tuned or hardcoded.",
		},
		"tests": tests,
		"summary": {"geomean_speedup": geomean, "tests_compared": log_count},
	}


func write_result_json(path: String, report: Dictionary) -> void:
	var file := FileAccess.open(path, FileAccess.WRITE)
	if file == null:
		push_error("Could not write " + path + " (error %d)" % FileAccess.get_open_error())
		return
	file.store_string(JSON.stringify(report, "  "))
	file.close()


func print_summary(report: Dictionary) -> void:
	print("")
	print("=== Summary ===")
	for row in report["tests"]:
		if row.has("speedup"):
			print("%-24s %s faster by %.2fx" % [row["name"], row["faster"], row["speedup"]])
		else:
			print("%-24s incomplete" % row["name"])
	print("")
	print("Geometric mean speedup: %.2fx (%d tests compared)" % [
		report["summary"]["geomean_speedup"], report["summary"]["tests_compared"]
	])
