#include <map>
#include <functional>
#include <string>
#include "../keys"
#include <GLFW/glfw3.h>

class KeyHandler {
private:
    using FuncPair = std::pair <std::function<void()>, std::function<void()>>;
    using KeyMap = std::map <int, FuncPair>;
    KeyMap key_map;
    GLFWwindow* win = nullptr;
public:
    KeyHandler(GLFWwindow* win = nullptr) : win(win) {};

    void bind(const int& key, const std::function<void()>& main_func) {
        key_map.emplace(key, FuncPair(main_func, [](){}));
    };

    void bind(const int& key, const std::function<void()>& main_func, const std::function<void()>& else_func) {
        key_map.emplace(key, FuncPair(main_func, else_func));
    };

    void use() {
        for (int i = 32; i < 348; i++) {
            auto it = key_map.find(i);
            if (it == key_map.end()) continue;
            if (glfwGetKey(win, i) == GLFW_PRESS) it->second.first();
            else it->second.second();
        }
    };
};