#include <HaruButton>
#include <HaruFrame>
#include <HaruTextBox>

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
    haru::engine::ui::TextBox textBox({0, 0, 120, 32}, "Patch");
    haru::engine::graphics::RenderQueue textQueue;
    textBox.render(textQueue);
    return button.contains({12, 16}) && !textQueue.commands().empty() ? 0 : 1;
}
