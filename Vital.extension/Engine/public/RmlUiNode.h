#pragma once

#include <Vital.sandbox/vital.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <RmlUi/Core.h>
#include <Vital.extension/Engine/public/RmlGodotRenderer.h>
#include <Vital.extension/Engine/public/RmlGodotFile.h>
#include <Vital.extension/Engine/public/RmlGodotSystem.h>


using namespace godot;

class RmlUiNode : public Node2D {
    GDCLASS(RmlUiNode, Node2D)

public:
    RmlUiNode();
    ~RmlUiNode() override;

    static void _bind_methods();

    void _ready() override;
    void _process(double delta) override;
    //void _draw() override;

private:
    // RmlUi context
    Rml::Context* context;

    // Rml interface objects
    RmlGodotRenderer* renderer;
    RmlGodotFile* file;
    RmlGodotSystem* system;
};
