extends Node

const WAIT_FOR_LUA_SECONDS    := 60.0
const GDSCRIPT_BENCHMARK_PATH := "resources/benchmark/benchmark.gd"
const LUA_COMPLETE_EVENT      := "benchmark:lua:complete"
const SCRIPTING_TESTS         := ["arithmetic", "function_calls", "table_access", "math_calls", "string_ops", "table_construction", "closures", "varargs", "table_iteration", "entity_simulation"]

@onready var core: Node = $"../Core"

var _lua_payload: Dictionary
var _lua_done := false


func _ready() -> void:
	var base_dir := OS.get_executable_path().get_base_dir()
	if OS.has_feature("editor"):
		base_dir = ProjectSettings.globalize_path("res://")

	var gd_path     := base_dir.path_join(GDSCRIPT_BENCHMARK_PATH)
	var result_path := base_dir.path_join("result.json")

	print("Vital.benchmark")
	print("GDScript path: " + gd_path)
	print("")

	core.native_event.connect(_on_native_event)

	var gd_results  := run_gdscript_benchmark(gd_path)
	print("")
	print("Waiting for Lua results...")
	var lua_results := await wait_for_lua_results()

	var report := build_report(lua_results, gd_results)
	write_result_json(result_path, report)
	print_summary(report)
	print("")
	print("Wrote " + result_path)

	shutdown_and_quit()


func wait_for_lua_results() -> Array:
	if not _lua_done:
		var deadline := Time.get_ticks_msec() + int(WAIT_FOR_LUA_SECONDS * 1000.0)
		while not _lua_done and Time.get_ticks_msec() < deadline:
			await get_tree().process_frame
		if not _lua_done:
			push_warning("Timed out waiting for \"%s\"" % LUA_COMPLETE_EVENT)

	if not _lua_done or not _lua_payload.has("array"):
		return []

	var top_level: Array = _lua_payload["array"]
	if top_level.is_empty() or typeof(top_level[0]) != TYPE_DICTIONARY:
		return []
	var wrapped: Dictionary = top_level[0]
	if not wrapped.has("array"):
		return []

	var results: Array = []
	for entry_variant in (wrapped["array"] as Array):
		if typeof(entry_variant) != TYPE_DICTIONARY or not entry_variant.has("object"):
			continue
		var obj: Dictionary = entry_variant["object"]
		results.append({
			"name":       obj.get("name", ""),
			"iterations": int(obj.get("iterations", 0)),
			"median_ms":  float(obj.get("median_ms", 0.0)),
			"mean_ms":    float(obj.get("mean_ms", 0.0)),
			"ops_sec":    float(obj.get("ops_sec", 0.0)),
			"checksum":   float(obj.get("checksum", 0.0)),
		})
	return results


func _on_native_event(name: String, payload: Dictionary) -> void:
	if name != LUA_COMPLETE_EVENT or _lua_done:
		return
	_lua_payload = payload
	_lua_done = true


func shutdown_and_quit() -> void:
	if core.has_method("shutdown"):
		core.call("shutdown")
	else:
		get_tree().quit()


func run_gdscript_benchmark(script_path: String) -> Array:
	if not FileAccess.file_exists(script_path):
		push_error("GDScript benchmark not found at: " + script_path)
		return []

	var src       := FileAccess.get_file_as_string(script_path)
	var gd_script := GDScript.new()
	gd_script.source_code = src
	var err := gd_script.reload()
	if err != OK:
		push_error("Failed to compile %s (error %d)" % [script_path, err])
		return []

	var instance = gd_script.new()
	return instance.run_all()


func build_report(lua_results: Array, gd_results: Array) -> Dictionary:
	var by_name: Dictionary = {}
	for r in lua_results:
		by_name[r["name"]] = {"lua": r}
	for r in gd_results:
		if not by_name.has(r["name"]):
			by_name[r["name"]] = {}
		by_name[r["name"]]["gdscript"] = r

	var scripting_tests: Array = []
	var native_tests: Array    = []
	var log_sum  := 0.0
	var log_count := 0

	for test_name in by_name.keys():
		var entry: Dictionary = by_name[test_name]
		var row: Dictionary   = {"name": test_name}

		if entry.has("lua"):      row["lua"]      = entry["lua"]
		if entry.has("gdscript"): row["gdscript"] = entry["gdscript"]

		if entry.has("lua") and entry.has("gdscript"):
			var lua_ops: float = entry["lua"]["ops_sec"]
			var gd_ops: float  = entry["gdscript"]["ops_sec"]
			if lua_ops > 0.0 and gd_ops > 0.0:
				var ratio: float = lua_ops / gd_ops
				row["throughput_ratio"] = ratio
				row["faster"] = "lua" if ratio >= 1.0 else "gdscript"
				if test_name in SCRIPTING_TESTS:
					log_sum   += log(ratio)
					log_count += 1

		if test_name in SCRIPTING_TESTS:
			scripting_tests.append(row)
		else:
			native_tests.append(row)

	var scripting_geomean := exp(log_sum / float(log_count)) if log_count > 0 else 0.0

	return {
		"generated_at_unix": Time.get_unix_time_from_system(),
		"methodology": {
			"target_ms": 150,
			"samples":   7,
			"warmups":   2,
			"note": "throughput_ratio = lua_ops_sec / gdscript_ops_sec. Ratio > 1.0 means Lua is faster.",
		},
		"scripting_tests":  scripting_tests,
		"native_api_tests": native_tests,
		"summary": {
			"scripting_geomean_throughput_ratio": scripting_geomean,
			"scripting_tests_compared":           log_count,
		},
	}


func write_result_json(path: String, report: Dictionary) -> void:
	var file := FileAccess.open(path, FileAccess.WRITE)
	if file == null:
		push_error("Could not write " + path + " (error %d)" % FileAccess.get_open_error())
		return
	file.store_string(JSON.stringify(report, "  "))
	file.close()


func print_summary(report: Dictionary) -> void:
	print("=== Scripting Performance (Lua ops/sec / GDScript ops/sec) ===")
	for row in report["scripting_tests"]:
		if row.has("throughput_ratio"):
			var ratio: float  = row["throughput_ratio"]
			var faster: String = row["faster"]
			var label := "%.2fx  Lua" % ratio if faster == "lua" else "%.2fx  GDScript" % (1.0 / ratio)
			print("  %-28s %s" % [row["name"], label])
		else:
			print("  %-28s incomplete" % row["name"])
	print("")
	print("  Geomean: %.2fx Lua  (%d scripting tests)" % [
		report["summary"]["scripting_geomean_throughput_ratio"],
		report["summary"]["scripting_tests_compared"]
	])
	print("")
	print("=== Native API (binding overhead) ===")
	for row in report["native_api_tests"]:
		if row.has("throughput_ratio"):
			var ratio: float   = row["throughput_ratio"]
			var faster: String = row["faster"]
			var label := "%.2fx  Lua" % ratio if faster == "lua" else "%.2fx  GDScript" % (1.0 / ratio)
			print("  %-28s %s" % [row["name"], label])
		else:
			print("  %-28s incomplete" % row["name"])
