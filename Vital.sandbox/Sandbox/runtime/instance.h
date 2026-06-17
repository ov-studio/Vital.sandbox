/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: instance.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Instance
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/runtime/module.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    struct vm_instance_base {
        virtual ~vm_instance_base() = default;
        virtual void push_self(Machine* vm) = 0;
    };

    template<typename Derived>
    struct vm_instance : public vm_instance_base, public std::enable_shared_from_this<Derived> {
        using std::enable_shared_from_this<Derived>::shared_from_this;
        private:
            std::vector<std::string> references;
        public:
            int id {};
            std::string env;
            std::atomic<bool> destroyed { false };
            Machine* vm = nullptr;
            void** userdata = nullptr;
            std::string self_reference() const { return fmt::format("vm_instance:{}:{}:self", vm_module::scope_name(Derived::Owner::base_scope), id); }
            std::string value_reference(int index) const { return fmt::format("vm_instance:{}:{}:value:{}", vm_module::scope_name(Derived::Owner::base_scope), id, index); }
            std::string value_reference(const std::string& index) const { return fmt::format("vm_instance:{}:{}:value:{}", vm_module::scope_name(Derived::Owner::base_scope), id, index); }

            bool is_alive() const {
                return true;
            }

            bool is_streamed() const {
                return true;
            }

            bool is_remote() const {
                return env.empty();
            }

            void set_reference(const std::string& name, int index) {
                vm -> set_reference("runtime", name, index);
                if (std::find(references.begin(), references.end(), name) == references.end()) references.push_back(name);
            }

            int get_reference(const std::string& name, bool push_to_stack = false, Machine* target_vm = nullptr) {
                int ref = vm -> get_reference("runtime", name, false);
                if (push_to_stack) {
                    Machine* dest = (target_vm && target_vm != vm) ? target_vm : vm;
                    dest -> get_raw_reference(ref);
                }
                return ref;
            }

            void del_reference(const std::string& name) {
                vm -> del_reference("runtime", name);
                auto it = std::find(references.begin(), references.end(), name);
                if (it != references.end()) references.erase(it);
            }

            void push_self(Machine* vm) override {
                auto instance = Derived::find_unlocked(static_cast<Derived*>(this) -> id);
                if (!instance || !instance -> userdata) vm -> push_nil();
                else instance -> get_reference(self_reference(), true, vm);
            }

            bool store(bool push_to_stack = false) {
                return Derived::store(static_cast<Derived*>(this) -> shared_from_this(), push_to_stack);
            }

            bool erase() {
                return Derived::erase(static_cast<Derived*>(this) -> shared_from_this());
            }

            bool erase_unlocked() {
                return Derived::erase_unlocked(static_cast<Derived*>(this) -> shared_from_this());
            }

            bool release() {
                return Derived::release(static_cast<Derived*>(this) -> shared_from_this());
            }

            void clean() {
                return Derived::clean(static_cast<Derived*>(this) -> shared_from_this());
            }

            static std::shared_ptr<Derived> find(int id) {
                std::lock_guard<std::mutex> lock(Derived::Owner::registry.mutex);
                return find_unlocked(id);
            }

            static std::shared_ptr<Derived> find_unlocked(int id) {
                auto it = Derived::Owner::registry.buffer.find(id);
                if (it == Derived::Owner::registry.buffer.end() || it -> second -> destroyed || !it -> second -> is_alive()) return nullptr;
                return it -> second;
            }
            

            static std::shared_ptr<Derived> find_unlocked(std::shared_ptr<Derived> instance) {
                if (!instance || instance -> destroyed || !instance -> is_alive()) return nullptr;
                return instance;
            }

            static bool store(std::shared_ptr<Derived> instance, bool push_to_stack = false) {
                {
                    std::lock_guard<std::mutex> lock(Derived::Owner::registry.mutex);
                    Derived::Owner::registry.buffer[instance -> id] = instance;
                }
                instance -> vm -> create_object(vm_module::scope_name(Derived::Owner::base_scope), instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(instance -> vm, -1);
                instance -> set_reference(instance -> self_reference(), -1);
                if (!push_to_stack) instance -> vm -> pop(1);
                Manager::Sandbox::get_singleton() -> signal("entity:created", Tool::StackValue(instance));
                return true;
            }

            static bool erase(std::shared_ptr<Derived> instance) {
                std::lock_guard<std::mutex> lock(Derived::Owner::registry.mutex);
                return erase_unlocked(instance);
            }

            static bool erase_unlocked(std::shared_ptr<Derived> instance) {
                auto it = Derived::Owner::registry.buffer.find(instance -> id);
                if (it == Derived::Owner::registry.buffer.end()) return false;
                Manager::Sandbox::get_singleton() -> signal("entity:destroyed", Tool::StackValue(instance));
                Derived::Owner::registry.buffer.erase(it);
                instance -> destroyed = true;
                return true;
            }

            static bool release(std::shared_ptr<Derived> instance) {
                vm_module::release_userdata_ptr(instance -> userdata);
                if (instance -> vm) {
                    auto snapshot = instance -> references;
                    for (auto& name : snapshot) instance -> del_reference(name);
                }
                instance -> vm = nullptr;
                return true;
            }

            static void clean(std::shared_ptr<Derived> instance) {
                if (!erase(instance)) return;
                release(instance);
            }

            static std::shared_ptr<Derived> init(Machine* vm, bool remote = false) {
                auto instance = std::make_shared<Derived>();
                instance -> id = Derived::Owner::registry.next_id.fetch_add(1);
                instance -> vm = remote ? Manager::Sandbox::get_singleton() -> get_vm() -> get_root() : vm -> get_root();
                if (!remote) instance -> env = vm -> get_environment_id();
                return instance;
            }

            static std::shared_ptr<Derived> make(Machine* vm, bool push_to_stack = false) {
                auto instance = Derived::init(vm);
                Derived::store(instance, push_to_stack);
                return instance;
            }

            static int destroy(Machine* vm) {
                auto ud = vm_module::get_userdata_ptr(vm, 1);
                if (ud && *ud) {
                    auto instance = Derived::find_unlocked(static_cast<Derived*>(*ud) -> id);
                    if (instance) instance -> clean();
                }
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            }

            static void collect_env(const std::string& env) {
                vm_module::collect_env(Derived::Owner::registry, env, std::function<void(std::shared_ptr<Derived>)>([](std::shared_ptr<Derived> instance) {
                    instance -> clean();
                }));
            }
    };
}