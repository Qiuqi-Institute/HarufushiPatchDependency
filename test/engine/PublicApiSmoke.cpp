#include <HaruButton>

int main() {
    haru::engine::ui::Button button({4, 8, 96, 32}, "Patch");
    return button.contains({12, 16}) ? 0 : 1;
}
