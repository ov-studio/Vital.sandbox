#include <Vital.extension/Engine/public/RmlUiNode.h>
#include <godot_cpp/classes/rendering_server.hpp>

using namespace godot;

void RmlUiNode::_ready() {
    Rml::SetSystemInterface(&system);
    Rml::SetFileInterface(&file);

    Rml::Initialise();

    renderer.canvas = get_canvas_item();
    Rml::SetRenderInterface(&renderer);

    context = Rml::CreateContext(
        "main",
        Rml::Vector2i(1024, 768)
    );

    context->LoadDocument("ui/main.rml")->Show();
}

void RmlUiNode::_process(double) {
    if (!context)
        return;

    context->Update();
    context->Render();
}
