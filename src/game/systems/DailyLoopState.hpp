#pragma once

namespace haru::game::systems {

enum class DailyAction {
    Study,
    Modding,
    SpendTimeWithHarufushi,
    Rest,
};

struct DailyActionRule {
    int minimumDay = 1;
    int minimumEnergy = 0;
    int minimumStudyFocus = 0;
    int maximumDependence = 100;
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

    static DailyActionRule ruleFor(DailyAction action);
    static bool canApply(DailyAction action, const DailyStats& stats);

    bool canApply(DailyAction action) const;
    bool apply(DailyAction action);
    const DailyStats& stats() const;

private:
    DailyStats stats_;
};

} // namespace haru::game::systems
