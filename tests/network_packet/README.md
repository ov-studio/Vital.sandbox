# Native network packet regression tests

These tests use the production packet classifier and Stack implementation with real Godot Variants. They cover malformed discriminators, commands, envelope types, internal grammar, serial limits, deep and cyclic pre-handshake payloads, admission gating, and valid packet roundtrips. They do not start an ENet listener or load the complete Vital server.

Initialize the pinned godot-cpp submodule, then configure and build with CMake, a C++17 compiler, and Python 3:

```powershell
cmake -S tests/network_packet -B .build/network-packet -G "Visual Studio 17 2022"
cmake --build .build/network-packet --config Debug --parallel 4
godot --headless --path .build/network-packet/project --script run.gd -- Debug
```

Use Godot 4.7 or later, matching the pinned godot-cpp API. The extension exits with status 1 on failure and prints the check count on success. Other CMake generators and Release builds are supported (pass the matching configuration to the script).
