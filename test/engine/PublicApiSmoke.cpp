#include <HaruFrame>

int main() {
    haru::engine::HaruFrame frame(0.25);
    haru::engine::graphics::RenderQueue queue;
    bool contentRendered = false;
    frame.render(queue, {1280, 720}, 0.016, [&](haru::engine::graphics::RenderQueue&) {
        contentRendered = true;
    });

    if (contentRendered || queue.commands().empty()) {
        return 1;
    }

    haru::engine::ui::Button button({4, 8, 96, 32}, "Patch");
    return button.contains({12, 16}) ? 0 : 1;
}
