#pragma once

#include <Vital.sandbox/vital.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <RmlUi/Core.h>
#include <Vital.extension/Engine/public/RmlGodotRenderer.h>
#include <Vital.extension/Engine/public/RmlGodotFile.h>
#include <Vital.extension/Engine/public/RmlGodotSystem.h>

class RmlGodotRenderer;
class RmlGodotFile;
class RmlGodotSystem;

class RmlUiNode : public godot::Node2D {
    GDCLASS(RmlUiNode, godot::Node2D)

public:
    RmlUiNode();
    ~RmlUiNode();

    void _ready() override;
    void _process(double delta) override;
    void _draw() override;

private:
    Rml::Context* context = nullptr;
    RmlGodotRenderer* renderer = nullptr;
    RmlGodotFile* file = nullptr;
    RmlGodotSystem* system = nullptr;
}