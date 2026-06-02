#pragma once

namespace haru::game::systems {

enum class DailyAction {
    Study,
    Modding,
    SpendTimeWithHarufushi,
    Rest,
};

struct DailyStats {
    int day = 1;
    int energy = 70;
    int studyFocus = 0;
    int modProgress = 0;
    int harufushiBond = 0;
    int dependence = 0;
};

class DailyLoopState {
public:
    DailyLoopState() = default;
    explicit DailyLoopState(DailyStats stats);

    void apply(DailyAction action);
    const DailyStats& stats() const;

private:
    DailyStats stats_;
};

} // namespace haru::game::systems
