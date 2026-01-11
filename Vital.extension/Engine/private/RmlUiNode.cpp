#include <Vital.extension/Engine/public/RmlUiNode.h>
#include <godot_cpp/classes/rendering_server.hpp>
#include <Vital.extension/Engine/public/RmlGodotRenderer.h>
#include <Vital.extension/Engine/public/RmlGodotFile.h>
#include <Vital.extension/Engine/public/RmlGodotSystem.h>

using namespace godot;

RmlUiNode::RmlUiNode() {
    // DO NOT create Rml objects here
}

RmlUiNode::~RmlUiNode() {
    if (context) {
        Rml::RemoveContext(context->GetName());
        context = nullptr;
    }

    delete renderer;
    delete file;
    delete system;
}

void RmlUiNode::_bind_methods() {
    // Even if empty, MUST exist as a real symbol
}

void RmlUiNode::_ready() {
    renderer = new RmlGodotRenderer();
    file = new RmlGodotFile();
    system = new RmlGodotSystem();

    Rml::SetRenderInterface(renderer);
    Rml::SetFileInterface(file);
    Rml::SetSystemInterface(system);

    context = Rml::CreateContext(
        "main",
        Rml::Vector2i(1024, 768)
    );
    UtilityFunctions::print("RmlUiNode ready");
}

void RmlUiNode::_process(double delta) {
    if (context) {
        context->Update();
        context->Render();
    }
}