// model_loader.cpp
#include <Vital.extension/Engine/public/model.h>
#include <godot_cpp/classes/animation_library.hpp>

using namespace godot;

ModelLoader* ModelLoader::singleton = nullptr;

// ========== ModelObject Implementation ==========

void ModelObject::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_model_name", "name"), &ModelObject::set_model_name);
    ClassDB::bind_method(D_METHOD("get_model_name"), &ModelObject::get_model_name);
    
    ClassDB::bind_method(D_METHOD("set_position", "x", "y", "z"), &ModelObject::set_position);
    ClassDB::bind_method(D_METHOD("set_rotation", "x", "y", "z"), &ModelObject::set_rotation);
    ClassDB::bind_method(D_METHOD("get_position_vec"), &ModelObject::get_position_vec);
    ClassDB::bind_method(D_METHOD("get_rotation_vec"), &ModelObject::get_rotation_vec);

    // Animation methods
    ClassDB::bind_method(D_METHOD("play_animation", "animation_name", "loop", "speed"), &ModelObject::play_animation, DEFVAL(true), DEFVAL(1.0f));
    ClassDB::bind_method(D_METHOD("stop_animation"), &ModelObject::stop_animation);
    ClassDB::bind_method(D_METHOD("pause_animation"), &ModelObject::pause_animation);
    ClassDB::bind_method(D_METHOD("resume_animation"), &ModelObject::resume_animation);
    ClassDB::bind_method(D_METHOD("is_animation_playing"), &ModelObject::is_animation_playing);
    ClassDB::bind_method(D_METHOD("get_current_animation"), &ModelObject::get_current_animation);
    ClassDB::bind_method(D_METHOD("get_available_animations"), &ModelObject::get_available_animations);
    ClassDB::bind_method(D_METHOD("set_animation_speed", "speed"), &ModelObject::set_animation_speed);
    ClassDB::bind_method(D_METHOD("get_animation_speed"), &ModelObject::get_animation_speed);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "model_name"), "set_model_name", "get_model_name");
}

ModelObject::ModelObject() : instance_node(nullptr), animation_player(nullptr) {
}

ModelObject::~ModelObject() {
    if (instance_node != nullptr && instance_node->is_inside_tree()) {
        instance_node->queue_free();
    }
}

AnimationPlayer* ModelObject::find_animation_player(Node* node) {
    if (node == nullptr) {
        return nullptr;
    }

    // Check if this node is an AnimationPlayer
    AnimationPlayer* anim_player = Object::cast_to<AnimationPlayer>(node);
    if (anim_player != nullptr) {
        return anim_player;
    }

    // Recursively search children
    for (int i = 0; i < node->get_child_count(); i++) {
        Node* child = node->get_child(i);
        AnimationPlayer* found = find_animation_player(child);
        if (found != nullptr) {
            return found;
        }
    }

    return nullptr;
}

void ModelObject::set_model_name(const String& name) {
    model_name = name;
}

String ModelObject::get_model_name() const {
    return model_name;
}

void ModelObject::set_position(float x, float y, float z) {
    set_global_position(Vector3(x, y, z));
    if (instance_node != nullptr) {
        instance_node->set_global_position(Vector3(x, y, z));
    }
}

void ModelObject::set_rotation(float x, float y, float z) {
    set_rotation_degrees(Vector3(x, y, z));
    if (instance_node != nullptr) {
        instance_node->set_rotation_degrees(Vector3(x, y, z));
    }
}

Vector3 ModelObject::get_position_vec() const {
    return get_global_position();
}

Vector3 ModelObject::get_rotation_vec() const {
    return get_rotation_degrees();
}

bool ModelObject::play_animation(const String& animation_name, bool loop, float speed) {
    if (animation_player == nullptr) {
        UtilityFunctions::push_warning("No AnimationPlayer found in model '", model_name, "'");
        return false;
    }

    if (!animation_player->has_animation(animation_name)) {
        UtilityFunctions::push_warning("Animation '", animation_name, "' not found in model '", model_name, "'");
        return false;
    }

    // Get the animation and set loop mode
    Ref<Animation> anim = animation_player->get_animation(animation_name);
    if (anim.is_valid()) {
        if (loop) {
            anim->set_loop_mode(Animation::LOOP_LINEAR);
        } else {
            anim->set_loop_mode(Animation::LOOP_NONE);
        }
    }

    // Set speed and play
    animation_player->set_speed_scale(speed);
    animation_player->play(animation_name);
    
    UtilityFunctions::print("Playing animation '", animation_name, "' on model '", model_name, "'");
    return true;
}

void ModelObject::stop_animation() {
    if (animation_player != nullptr) {
        animation_player->stop();
    }
}

void ModelObject::pause_animation() {
    if (animation_player != nullptr) {
        animation_player->pause();
    }
}

void ModelObject::resume_animation() {
    if (animation_player != nullptr) {
        String current = animation_player->get_current_animation();
        if (!current.is_empty()) {
            animation_player->play(current);
        }
    }
}

bool ModelObject::is_animation_playing() const {
    if (animation_player == nullptr) {
        return false;
    }
    return animation_player->is_playing();
}

String ModelObject::get_current_animation() const {
    if (animation_player == nullptr) {
        return "";
    }
    return animation_player->get_current_animation();
}

Array ModelObject::get_available_animations() const {
    Array animations;
    
    if (animation_player == nullptr) {
        return animations;
    }

    PackedStringArray anim_list = animation_player->get_animation_list();
    for (int i = 0; i < anim_list.size(); i++) {
        animations.append(anim_list[i]);
    }

    return animations;
}

void ModelObject::set_animation_speed(float speed) {
    if (animation_player != nullptr) {
        animation_player->set_speed_scale(speed);
    }
}

float ModelObject::get_animation_speed() const {
    if (animation_player == nullptr) {
        return 1.0f;
    }
    return animation_player->get_speed_scale();
}

void ModelObject::_ready() {
    Node3D::_ready();
    
    // Find the AnimationPlayer in the hierarchy
    animation_player = find_animation_player(this);
    
    if (animation_player != nullptr) {
        UtilityFunctions::print("Found AnimationPlayer in model '", model_name, "'");
        
        // List available animations
        Array anims = get_available_animations();
        if (anims.size() > 0) {
            UtilityFunctions::print("Available animations: ", anims);


            // TODO: TESTING
            godot::String anim_name = anims[0];
            godot::UtilityFunctions::print("yes entered anim", anim_name);
            play_animation(anim_name, true, 1.0f);
            godot::UtilityFunctions::print("Playing animation: ", anim_name);
        }
    }
}

// ========== ModelLoader Implementation ==========

void ModelLoader::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_model", "model_name", "file_path"), &ModelLoader::load_model);
    ClassDB::bind_method(D_METHOD("load_model_absolute", "model_name", "absolute_path"), &ModelLoader::load_model_absolute);
    ClassDB::bind_method(D_METHOD("create_object", "model_name"), &ModelLoader::create_object);
    ClassDB::bind_method(D_METHOD("unload_model", "model_name"), &ModelLoader::unload_model);
    ClassDB::bind_method(D_METHOD("is_model_loaded", "model_name"), &ModelLoader::is_model_loaded);
    ClassDB::bind_method(D_METHOD("get_loaded_models"), &ModelLoader::get_loaded_models);
}

ModelLoader::ModelLoader() {
    if (singleton == nullptr) {
        singleton = this;
    }
}

ModelLoader::~ModelLoader() {
    if (singleton == this) {
        singleton = nullptr;
    }
    loaded_models.clear();
}

ModelLoader* ModelLoader::get_singleton() {
    return singleton;
}

Ref<PackedScene> ModelLoader::load_from_resource_path(const String& file_path) {
    ResourceLoader* loader = ResourceLoader::get_singleton();
    Ref<PackedScene> scene = loader->load(file_path);
    return scene;
}

Ref<PackedScene> ModelLoader::load_from_absolute_path(const String& file_path) {
    String lower_path = file_path.to_lower();
    
    if (lower_path.ends_with(".glb") || lower_path.ends_with(".gltf")) {
        Ref<GLTFDocument> gltf_doc = memnew(GLTFDocument);
        Ref<GLTFState> gltf_state = memnew(GLTFState);
        
        Error err = gltf_doc->append_from_file(file_path, gltf_state);
        if (err != OK) {
            UtilityFunctions::push_error("Failed to load GLTF from absolute path: ", file_path);
            return Ref<PackedScene>();
        }
        
        Node* root = gltf_doc->generate_scene(gltf_state);
        if (root == nullptr) {
            UtilityFunctions::push_error("Failed to generate scene from GLTF: ", file_path);
            return Ref<PackedScene>();
        }
        
        Ref<PackedScene> scene = memnew(PackedScene);
        scene->pack(root);
        
        memdelete(root);
        
        return scene;
    } 
    else if (lower_path.ends_with(".tscn") || lower_path.ends_with(".scn")) {
        String user_path = "user://" + file_path;
        ResourceLoader* loader = ResourceLoader::get_singleton();
        Ref<PackedScene> scene = loader->load(user_path);
        return scene;
    }
    
    UtilityFunctions::push_error("Unsupported file format: ", file_path);
    return Ref<PackedScene>();
}

bool ModelLoader::load_model(const String& model_name, const String& file_path) {
    std::string key = std::string(model_name.utf8().get_data());
    
    if (loaded_models.find(key) != loaded_models.end()) {
        UtilityFunctions::push_warning("Model '", model_name, "' is already loaded.");
        return false;
    }

    Ref<PackedScene> scene;
    
    if (file_path.begins_with("res://") || file_path.begins_with("user://")) {
        scene = load_from_resource_path(file_path);
    } else {
        scene = load_from_absolute_path(file_path);
    }
    
    if (scene.is_null()) {
        UtilityFunctions::push_error("Failed to load model from path: ", file_path);
        return false;
    }

    loaded_models[key] = scene;
    UtilityFunctions::print("Model '", model_name, "' loaded successfully from ", file_path);
    return true;
}

bool ModelLoader::load_model_absolute(const String& model_name, const String& absolute_path) {
    std::string key = std::string(model_name.utf8().get_data());
    
    if (loaded_models.find(key) != loaded_models.end()) {
        UtilityFunctions::push_warning("Model '", model_name, "' is already loaded.");
        return false;
    }

    Ref<PackedScene> scene = load_from_absolute_path(absolute_path);
    
    if (scene.is_null()) {
        UtilityFunctions::push_error("Failed to load model from absolute path: ", absolute_path);
        return false;
    }

    loaded_models[key] = scene;
    UtilityFunctions::print("Model '", model_name, "' loaded successfully from ", absolute_path);
    return true;
}

ModelObject* ModelLoader::create_object(const String& model_name) {
    std::string key = std::string(model_name.utf8().get_data());
    
    auto it = loaded_models.find(key);
    if (it == loaded_models.end()) {
        UtilityFunctions::push_error("Model '", model_name, "' is not loaded. Call load_model first.");
        return nullptr;
    }

    ModelObject* obj = memnew(ModelObject);
    obj->set_model_name(model_name);

    Node* instance = it->second->instantiate();
    if (instance == nullptr) {
        UtilityFunctions::push_error("Failed to instantiate model '", model_name, "'");
        memdelete(obj);
        return nullptr;
    }

    obj->add_child(instance);
    instance->set_owner(obj);

    return obj;
}

bool ModelLoader::unload_model(const String& model_name) {
    std::string key = std::string(model_name.utf8().get_data());
    
    auto it = loaded_models.find(key);
    if (it == loaded_models.end()) {
        UtilityFunctions::push_warning("Model '", model_name, "' is not loaded.");
        return false;
    }

    loaded_models.erase(it);
    UtilityFunctions::print("Model '", model_name, "' unloaded successfully.");
    return true;
}

bool ModelLoader::is_model_loaded(const String& model_name) const {
    std::string key = std::string(model_name.utf8().get_data());
    return loaded_models.find(key) != loaded_models.end();
}

Array ModelLoader::get_loaded_models() const {
    Array result;
    for (const auto& pair : loaded_models) {
        result.append(String(pair.first.c_str()));
    }
    return result;
}