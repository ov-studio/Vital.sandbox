# godot-wry on Linux — findings

Notes from investigating the vendored `godot-wry` addon on Linux. This is
informational: no engine code changes are proposed here. Upstream fixes are in
[doceazedo/godot_wry#98](https://github.com/doceazedo/godot_wry/pull/98).

Tested on Fedora 44, Godot 4.6.3, WebKitGTK 2.52.5, Intel HD 520, GNOME
(Wayland session, tested both via XWayland and natively).

## 1. The pinned v1.0.2 binary crashes on Linux

`vendor.yaml` pins `version: "v1.0.2"`. The committed Linux binary is
byte-identical to that upstream release:

```
Vital.client/vital.wry/bin/x86_64-unknown-linux-gnu/libgodot_wry.so
  md5 4b66f0a3fd20bd7788a82e94e42ca67d

godot_wry.zip from upstream release v1.0.2
  md5 4b66f0a3fd20bd7788a82e94e42ca67d
```

The extension loads and the `WebView` class registers correctly, but the process
aborts as soon as a `WebView` node enters the scene tree:

```
ERROR: [panic webkit2gtk-2.0.1/src/auto/application_info.rs:21]
       GTK has not been initialized. Call `gtk::init` first.
ERROR: godot-rust function call failed: WebView::ready()
    Reason: GTK has not been initialized. Call `gtk::init` first.
```

WRY needs GTK initialised before creating a webview, and Godot never does it.
`gtk::init()` was dropped from `create_webview()` in v1.0.2 specifically — it is
present in upstream `main` (`rust/src/lib.rs`, `rust/src/godot_window.rs`), so
the fix exists upstream but has not been released. Same failure is reported by
others in [doceazedo/godot_wry#17](https://github.com/doceazedo/godot_wry/issues/17).

**Implication:** any Linux build using this pinned version will abort when a
`WebView` is instantiated, regardless of anything else in this document. A newer
upstream release (or a source build) is required.

## 2. `transparent = true` cannot work in the default rendering mode

This is architectural, not a configuration mistake.

WRY does ask WebKit for a transparent background
(`wry-0.50.5/src/webkitgtk/mod.rs:253`):

```rust
if attributes.transparent {
  webview.set_background_color(&gtk::gdk::RGBA::new(0., 0., 0., 0.));
}
```

…but `build_as_child` then paints that into an X11 child window created with
(`mod.rs:178`):

```rust
XCreateSimpleWindow(display, parent, x, y, width, height, 0, 0, 0)
```

`XCreateSimpleWindow` inherits the parent's visual and depth. Critically, **X11
does not alpha-blend a child window against its parent's rendered content** —
compositors blend *top-level* windows, not subwindows. WebKit computes the alpha
and X11 discards it, so the webview always paints as an opaque rectangle.

Two workarounds were tested and **both failed**:

| Attempt | Result |
| --- | --- |
| `WEBKIT_DISABLE_DMABUF_RENDERER=1` | No change |
| Godot per-pixel transparency (`display/window/per_pixel_transparency/allowed` + `Window.transparent_bg`), granting a 32-bit ARGB parent visual that the child inherits | No change |

The second is the informative one: the ARGB visual *was* granted, and it still
made no difference — which rules out visual depth and confirms the blocker is
child-window compositing itself.

This is why transparency works on Windows (composited child HWNDs) and macOS
(NSView subviews blend natively) but not on Linux.

## 3. Wayland is not supported in the default mode

`rust/src/godot_window.rs` hard-panics with `"GDK backend must be X11"`. There is
no Wayland path, so Godot must be run under XWayland:

```bash
GDK_BACKEND=x11 godot --display-driver x11
```

## 4. Proposed upstream fix

[doceazedo/godot_wry#98](https://github.com/doceazedo/godot_wry/pull/98) adds an
opt-in `render_to_texture` mode (Linux only, default off) that hosts the webview
in a `GtkOffscreenWindow` and composites it in Godot as a texture instead of
using a native X11 child window.

|  | default | `render_to_texture` |
| --- | --- | --- |
| `transparent = true` | ignored | works |
| Wayland | panics | runs natively |
| Appears in screenshots / screen recording | no | yes |
| Usable as a `Texture2D` | no | yes |

Input is translated into GDK events dispatched via `gtk_main_do_event`, so
WebKit treats them as trusted input and text entry, focus, hover and scrolling
behave natively.

Verified there with automated tests covering transparency (pixel-level
inspection of the composited framebuffer and the raw texture), mouse clicks with
hit-testing, keyboard text round-trip through the DOM, and all 21 public
`WebView` methods.

Known limitations are documented on that PR — notably Wayland currently requires
`WEBKIT_DISABLE_COMPOSITING_MODE=1`, and there is a per-frame readback cost.

## Relevance to Vital

Item 1 affects Linux builds today regardless of the rest. Items 2–4 matter only
if HTML UI needs to render over the 3D scene, or if native Wayland support is
wanted.

No action is proposed in this repository beyond awareness — the fixes belong
upstream, and picking them up is a `vendor.yaml` version bump once a release
including them is published.
