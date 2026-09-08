extends Node

# ==============================================================
# Vital.benchmark entry point.
#
# Core boots the Lua sandbox and bootstraps resources (see
# config.yaml's `bootstrap: - "benchmark"`) on its own background
# thread. Core now exposes a real GDScript-facing "native_event"
# signal (see Engine::Core::emit_native_event / _bind_methods in
# Engine/public/core.h) that any resource can fire straight from Lua
# via util.event.emit_native(name, ...) - see
# API/utility/event.h and resources/benchmark/benchmark.lua. So this
# script:
#
#   1. Runs the GDScript half directly, in-process - just a function
#      call, results captured as real Dictionaries, no parsing.
#   2. Connects to Core's "native_event" signal and waits for the Lua
#      resource to fire "benchmark:lua:complete" with its results
#      table - no fixed sleep, no log-file scraping/regex. A generous
#      timeout is kept only as a safety net in case a resource never
#      starts (e.g. a cold Vital.kit download that hangs, or a script
#      error before it reaches emit_native).
#   3. Writes result.json next to the executable.
#   4. Shuts Core down properly (Core.shutdown(), which stops every
#      resource and tears down the sandbox singleton cleanly) before
#      quitting, instead of calling get_tree().quit() cold.
# ==============================================================

# Safety-net ceiling only - the normal path resolves the moment the
# "benchmark:lua:complete" native_event arrives, almost always well
# under this. Covers a cold-cache Vital.kit download on first run
# (observed ~8s extra) plus 6 workloads x ~9 runs x ~150ms each.
const WAIT_FOR_LUA_SECONDS := 60.0

const GDSCRIPT_BENCHMARK_PATH := "resources/benchmark/benchmark.gd"
const LUA_COMPLETE_EVENT := "benchmark:lua:complete"

@onready var core: Node = $"../Core"

var _lua_payload: Dictionary
var _lua_done := false


func _ready() -> void:
	var base_dir := OS.get_executable_path().get_base_dir()
	if OS.has_feature("editor"):
		# Running via `godot --headless --path Vital.benchmark` for
		# local testing rather than the exported binary.
		base_dir = ProjectSettings.globalize_path("res://")

	var gd_path := base_dir.path_join(GDSCRIPT_BENCHMARK_PATH)
	var result_path := base_dir.path_join("result.json")

	print("Vital.benchmark")
	print("Lua/GDScript:  resources/benchmark/ (Lua auto-started via config.yaml bootstrap)")
	print("GDScript path: " + gd_path)
	print("")

	core.native_event.connect(_on_native_event)

	var gd_results := run_gdscript_benchmark(gd_path)

	print("")
	print("Waiting for the Lua resource's \"%s\" event..." % LUA_COMPLETE_EVENT)
	var lua_results := await wait_for_lua_results()

	var report := build_report(lua_results, gd_results)
	write_result_json(result_path, report)

	print_summary(report)
	print("")
	print("Wrote " + result_path)

	shutdown_and_quit()


## Resolves as soon as _on_native_event() records the Lua side's
## results, or after WAIT_FOR_LUA_SECONDS as a safety-net fallback
## (in which case Lua's numbers are simply absent from result.json,
## same graceful-degradation behavior the old log-scraping path had).
func wait_for_lua_results() -> Array:
	if not _lua_done:
		var deadline := Time.get_ticks_msec() + int(WAIT_FOR_LUA_SECONDS * 1000.0)
		# _on_native_event() is connected in _ready() and flips _lua_done
		# the moment "benchmark:lua:complete" arrives (Core's emit_signal()
		# always lands on the main thread - see Core::execute()/drain() -
		# so this poll and that signal are never racing on different
		# threads). Polling one frame at a time is simpler and just as
		# fast in practice as racing a second Signal against native_event,
		# and it keeps the fallback timeout trivially easy to reason about.
		while not _lua_done and Time.get_ticks_msec() < deadline:
			await get_tree().process_frame
		if not _lua_done:
			push_warning("Timed out waiting for \"%s\" - Lua results will be missing from result.json" % LUA_COMPLETE_EVENT)

	if not _lua_done or not _lua_payload.has("array"):
		return []

	# util.event.emit_native("benchmark:lua:complete", results) passes the
	# whole `results` table as a *single* Lua argument, so on the way over
	# it gets one extra layer of nesting versus "each result is its own
	# arg": payload.array (the varargs collected after `name`) has exactly
	# one entry, and that entry - not _lua_payload["array"] itself - is the
	# {"array": [...6 results...], "object": {}} produced from `results`.
	# See Tool::Stack::to_dict()/StackValue::to_variant() on the C++ side.
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
			"name": obj.get("name", ""),
			"iterations": int(obj.get("iterations", 0)),
			"median_ms": float(obj.get("median_ms", 0.0)),
			"mean_ms": float(obj.get("mean_ms", 0.0)),
			"ops_sec": float(obj.get("ops_sec", 0.0)),
			"checksum": float(obj.get("checksum", 0.0)),
		})
	return results


func _on_native_event(name: String, payload: Dictionary) -> void:
	if name != LUA_COMPLETE_EVENT or _lua_done:
		return
	_lua_payload = payload
	_lua_done = true


func shutdown_and_quit() -> void:
	# Core.shutdown() stops every running resource (unloads Lua
	# environments cleanly) and tears down the Sandbox/VM singleton
	# before freeing itself - the same graceful path used when a
	# normal Vital.server/Vital.client process exits, rather than
	# quitting the whole engine out from under a still-live sandbox.
	if core.has_method("shutdown"):
		core.call("shutdown")
	else:
		get_tree().quit()


## Loads resources/benchmark/benchmark.gd from disk and runs it, returning its
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
