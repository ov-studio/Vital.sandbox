extends SceneTree

func _initialize():
	var config = "Debug"
	if OS.get_cmdline_user_args().size() > 0:
		config = OS.get_cmdline_user_args()[0]
	var result = GDExtensionManager.load_extension("res://regression.%s.gdextension" % config)
	if result != OK:
		push_error("Failed to load regression extension: %s" % result)
		quit(1)
		return
	quit(0)
