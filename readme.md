## Overview

<div align="center">
  <img src="https://vital-sandbox.com/logo/neon_background_round.png"/>

  <sub>Founded & maintained by Aviril, Mario, Tron, Anisa & A-Variakojiene</sub>
  
  [![License](https://img.shields.io/badge/License-Open--Source-3A7D44?style=flat-square&logo=apache&logoColor=white)](https://github.com/ov-studio/Vital.sandbox/blob/main/LICENSE)
  [![Engine](https://img.shields.io/badge/Engine-Godot--Engine-2F6DB3?style=flat-square&logo=godotengine&logoColor=white)](https://godotengine.org)
  [![Language](https://img.shields.io/badge/Core-C%2B%2B17-004482?style=flat-square&logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/17)
  [![Website](https://img.shields.io/badge/Website-Vital.sandbox-CFD8DC?style=flat-square&logo=googlechrome&logoColor=white)](https://vital-sandbox.com)
  [![Discord](https://img.shields.io/badge/Discord-Community-404EED?style=flat-square&logo=discord&logoColor=white)](https://discord.vital-sandbox.com)
</div>

##

**Vital.sandbox** is an open-source, high-performance sandbox built on Godot-Engine and powered by C++17 & Lua.
<br/>Full control over rendering, networking, threading, and assets — one seamless workflow.

## Getting Started

- **Releases**: Grab the latest stable build from [Releases](https://github.com/ov-studio/Vital.sandbox/releases).
- **Documentation**: Learn the APIs, scripting patterns, and engine integration in the [Guides](https://vital-sandbox.com/docs).
- **Benchmarks**: See how Vital.sandbox performs against GDScript in the [Benchmarks](https://vital-sandbox.com/benchmarks).
- **Vault**: Browse community resources and scripts in the [Vault](https://vital-sandbox.com/vault).
- **Roadmap**: Track what's planned and in progress on the [Roadmap](https://vital-sandbox.com/roadmap).
- **Vendors**: View all third-party dependencies and licenses in [Vendor](https://github.com/ov-studio/Vital.sandbox/blob/main/vendor.md).
- **Community**: Got questions or want to connect? Join us on [Discord](http://discord.vital-sandbox.com).

## Built-in Features

Vital.sandbox's Lua layer isn't a thin wrapper, it's a full-featured runtime that touches every system in the engine and actively expanding — [Roadmap](https://vital-sandbox.com/roadmap).

- **UI primitives**: Canvas, SVG, image, font, and texture APIs
- **Rendering**: Control scenes, shaders, rendertargets, and draw calls directly from script
- **Shaders**: GLSL compilation from file or raw source, with full uniform control and model / rendertarget binding
- **Models**: Load, instantiate, transform, and animate 3D model assets at runtime
- **Camera**: FOV, clip planes, projection modes, depth of field, auto exposure, and ray projection
- **Particles**: GPU particle emitters with emission shapes, trails, custom materials, mesh particles, and emit / finished signals
- **Display**: Resolution query, live scene texture, and screenshot capture
- **Audio**: 2D and 3D audio sources with effect bus support
- **Crypto**: Built-in cryptographic utilities
- **Networking**: Built-in async networking primitives for multiplayer-ready games, with ENet transport, reliable RPC, and entity replication with client interpolation and late-join sync
- **Threading**: True multi-threading support with promise-based coordination
- **Asset delivery**: Dynamic asset loading and streaming, managed entirely from Lua, with compressed, hash-verified downloads
- **Resources**: Isolated per-resource Lua environments with dependency resolution, start / stop / restart control, and cross-resource exports
- **Event system**: Heartbeats, priority-ordered listeners, built-in engine signals, and reactive patterns that scale cleanly
- **Input**: Key and mouse state, cursor control, key binds, and command handlers
- **Console**: Runtime console with structured logging and built-in client / server commands
- **Utilities**: Lua math, string, and table libraries enriched with sandbox-native helpers, plus timers, promises, easing curves, file access, HTTP, and compression
- **Performance monitors**: Native and custom monitor registration, querying, and listing
- **Database**: Async database access with query support (server-side)
- **Webview**: Embedded webview rendering (client-side)
- **Discord**: Native Discord integration (client-side)
- **GFX pipeline**: Full environment control — fog, volumetric fog, glow, sky (panoramic, physical, procedural), SSAO, SSIL, SSR, SDFGI, antialiasing, color grading, and more
- **Lighting**: Directional, point, spot, and area lights with full scripting control
- **Physics**: Complete physics support — rigid, static, animatable, character, and vehicle bodies, collision shapes, areas, spaces, and space queries (raycast, point / shape intersection, cast motion)
- **Physics debugger**: Per-shape and global collision wireframe visualizer, with color-coded local vs. replicated shapes

## Structure

- **`Vital.sandbox/`** — C++ GDExtension core
  - **`API/`** — Lua-facing bindings, grouped by system: `core`, `gfx`, `light`, `physics`, `utility`
  - **`Bootstrap/`** — build-time dependency scripts (Godot, vendors, Discord, etc.)
  - **`Config/`** — compile-time server configuration headers
  - **`Engine/`** — engine-side implementations backing every API type
  - **`Manager/`** — high-level managers: sandbox lifecycle, asset delivery, networking, masterlist, Discord, kit
  - **`Sandbox/`** — Lua VM: machine, mixin, runtime instance types
  - **`Tool/`** — internal C++ utilities (logging, threading, crypto, HTTP, file, timers, etc.)
  - **`Vital/`** — entrypoint, precompiled header, and top-level includes
- **`Vital.client/`** — Godot client project (loads the GDExtension, ships as the player binary)
- **`Vital.server/`** — Godot server project with `config.yaml` (network, sync, masterlist, bootstrap)
- **`Vital.benchmark/`** — self-contained benchmark project; runs benchmark and exports `benchmark.json`
- **`build.py`** — cross-platform build script

## Privacy Policy

- Vital.sandbox may send server information exposed by the server owner to the masterlist solely for server listing purposes.
- No personal data is collected, stored or shared with any third party.
