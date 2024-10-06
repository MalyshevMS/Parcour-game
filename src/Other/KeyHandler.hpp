#include <map>
#include <functional>
#include <string>
#include "../keys"
#include "../Variables/Client.hpp"
#include <GLFW/glfw3.h>

template <typename T1, typename T2, typename T3>
struct Tripple {
    T1 a;
    T2 b;
    T3 c;

    Tripple(T1 _a, T2 _b, T3 _c) {
        a = _a;
        b = _b;
        c = _c;
    }
};

class KeyHandler {
private:
    using FuncTripple = Tripple <std::function<void()>, std::function<void()>, bool>;
    using KeyMap = std::map <int, FuncTripple>;
    KeyMap key_map;
    GLFWwindow* win = nullptr;
public:
    KeyHandler(GLFWwindow* win = nullptr) : win(win) {};

    void bind(const int& key, const std::function<void()>& main_func, bool in_game = true) {
        key_map.emplace(key, FuncTripple(main_func, [](){}, in_game));
    };

    void bind(const int& key, const std::function<void()>& main_func, const std::function<void()>& else_func, bool in_game = true) {
        key_map.emplace(key, FuncTripple(main_func, else_func, in_game));
    };

    void use(Client& c) {
        for (int i = 32; i < 348; i++) {
            auto it = key_map.find(i);
            if (it == key_map.end()) continue;
            if (c.in_game == it->second.c) {
                if (glfwGetKey(win, i) == GLFW_PRESS) it->second.a();
                else it->second.b();
            }
        }
    };
};