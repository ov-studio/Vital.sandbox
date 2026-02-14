// model_loader.cpp
#include <Vital.extension/Engine/public/model.h>

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

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "model_name"), "set_model_name", "get_model_name");
}

ModelObject::ModelObject() : instance_node(nullptr) {
}

ModelObject::~ModelObject() {
    if (instance_node != nullptr && instance_node->is_inside_tree()) {
        instance_node->queue_free();
    }
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

void ModelObject::_ready() {
    Node3D::_ready();
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
    // Check file extension
    String lower_path = file_path.to_lower();
    
    if (lower_path.ends_with(".glb") || lower_path.ends_with(".gltf")) {
        // Load GLTF/GLB from absolute path
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
        
        // Create a PackedScene from the loaded node
        Ref<PackedScene> scene = memnew(PackedScene);
        scene->pack(root);
        
        // Clean up the temporary root node
        memdelete(root);
        
        return scene;
    } 
    else if (lower_path.ends_with(".tscn") || lower_path.ends_with(".scn")) {
        // For scene files, try using ResourceLoader with user:// path
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
    
    // Check if already loaded
    if (loaded_models.find(key) != loaded_models.end()) {
        UtilityFunctions::push_warning("Model '", model_name, "' is already loaded.");
        return false;
    }

    Ref<PackedScene> scene;
    
    // Check if it's a resource path (res://) or absolute path
    if (file_path.begins_with("res://") || file_path.begins_with("user://")) {
        scene = load_from_resource_path(file_path);
    } else {
        // Assume it's an absolute path
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
    
    // Check if already loaded
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
    
    // Check if model is loaded
    auto it = loaded_models.find(key);
    if (it == loaded_models.end()) {
        UtilityFunctions::push_error("Model '", model_name, "' is not loaded. Call load_model first.");
        return nullptr;
    }

    // Create a new ModelObject
    ModelObject* obj = memnew(ModelObject);
    obj->set_model_name(model_name);

    // Instantiate the scene
    Node* instance = it->second->instantiate();
    if (instance == nullptr) {
        UtilityFunctions::push_error("Failed to instantiate model '", model_name, "'");
        memdelete(obj);
        return nullptr;
    }

    // Add instance as child
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