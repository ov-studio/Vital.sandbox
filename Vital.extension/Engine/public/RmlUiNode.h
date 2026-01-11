#pragma once

#include <Vital.sandbox/vital.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <RmlUi/Core.h>
#include <Vital.extension/Engine/public/RmlGodotRenderer.h>
#include <Vital.extension/Engine/public/RmlGodotFile.h>
#include <Vital.extension/Engine/public/RmlGodotSystem.h>

class RmlUiNode : public godot::Node2D {
    GDCLASS(RmlUiNode, godot::Node2D)

public:
    static void _bind_methods() {}

    void _ready() override;
    void _process(double delta) override;

private:
    Rml::Context* context = nullptr;
    RmlGodotRenderer renderer;
    RmlGodotFile file;
    RmlGodotSystem system;
};
