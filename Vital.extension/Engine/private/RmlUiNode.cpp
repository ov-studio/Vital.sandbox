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
    using namespace godot;

    // 1️⃣ Initialize RmlUi (call once per process)
    Rml::Initialise();

    // 2️⃣ Create interface objects
    renderer = memnew(RmlGodotRenderer());
    file = memnew(RmlGodotFile());
    system = memnew(RmlGodotSystem());

    // 3️⃣ Register interfaces
    Rml::SetRenderInterface(renderer);
    Rml::SetFileInterface(file);
    Rml::SetSystemInterface(system);

    UtilityFunctions::print("RmlUi interfaces ready");

    // 4️⃣ Create a context (the “browser”)
    context = Rml::CreateContext(
        "main",
        Rml::Vector2i(1024, 768)
    );

    if (!context) {
        UtilityFunctions::print("Failed to create RmlUi context!");
        return;
    }

    UtilityFunctions::print("RmlUi context created");

    // 5️⃣ Load a document
    Rml::ElementDocument* doc = context->LoadDocument("res://main.rml");
    if (doc) {
        doc->Show();
        UtilityFunctions::print("Document loaded and visible");
    } else {
        UtilityFunctions::print("Failed to load document");
    }
}

void RmlUiNode::_process(double delta) {
    /*
    if (context) {
        context->Update();
        context->Render();
    }
    */
}