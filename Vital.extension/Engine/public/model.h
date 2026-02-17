// model_loader.h
#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <Vital.extension/Engine/public/index.h>


namespace godot {

class ModelObject : public Node3D {
    GDCLASS(ModelObject, Node3D)

private:
    String model_name;
    Node3D* instance_node;
    AnimationPlayer* animation_player;

    AnimationPlayer* find_animation_player(Node* node);

protected:
    static void _bind_methods();

public:
    ModelObject();
    ~ModelObject();

    void set_model_name(const String& name);
    String get_model_name() const;

    void set_position(float x, float y, float z);
    void set_rotation(float x, float y, float z);
    Vector3 get_position_vec() const;
    Vector3 get_rotation_vec() const;

    // Animation functions
    bool play_animation(const String& animation_name, bool loop = true, float speed = 1.0f);
    void stop_animation();
    void pause_animation();
    void resume_animation();
    bool is_animation_playing() const;
    String get_current_animation() const;
    Array get_available_animations() const;
    void set_animation_speed(float speed);
    float get_animation_speed() const;

    void _ready() override;
};

class ModelLoader : public Node {
    GDCLASS(ModelLoader, Node)

private:
    static ModelLoader* singleton;
    std::unordered_map<std::string, Ref<PackedScene>> loaded_models;

    Ref<PackedScene> load_from_resource_path(const String& file_path);
    Ref<PackedScene> load_from_absolute_path(const String& file_path);

protected:
    static void _bind_methods();

public:
    ModelLoader();
    ~ModelLoader();

    static ModelLoader* get_singleton();

    bool load_model(const String& model_name, const String& file_path);
    bool load_model_absolute(const String& model_name, const String& absolute_path);
    ModelObject* create_object(const String& model_name);
    bool unload_model(const String& model_name);
    bool is_model_loaded(const String& model_name) const;
    Array get_loaded_models() const;
};

} // namespace godot

#endif // MODEL_LOADER_H