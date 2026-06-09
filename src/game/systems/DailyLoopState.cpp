#include "DailyLoopState.hpp"

#include <algorithm>

namespace haru::game::systems {

namespace {

int clampStat(int value) {
    return std::clamp(value, 0, 100);
}

} // namespace

DailyLoopState::DailyLoopState(DailyStats stats) : stats_(stats) {}

DailyActionRule DailyLoopState::ruleFor(DailyAction action) {
    switch (action) {
    case DailyAction::Study:
        return {1, 15, 0, 100};
    case DailyAction::Modding:
        return {2, 30, 12, 100};
    case DailyAction::SpendTimeWithHarufushi:
        return {2, 20, 0, 70};
    case DailyAction::Rest:
        return {1, 0, 0, 100};
    }

    return {};
}

bool DailyLoopState::canApply(DailyAction action, const DailyStats& stats) {
    const DailyActionRule rule = ruleFor(action);
    return stats.day >= rule.minimumDay && stats.energy >= rule.minimumEnergy &&
           stats.studyFocus >= rule.minimumStudyFocus &&
           stats.dependence <= rule.maximumDependence;
}

bool DailyLoopState::canApply(DailyAction action) const {
    return canApply(action, stats_);
}

bool DailyLoopState::apply(DailyAction action) {
    if (!canApply(action)) {
        return false;
    }

    switch (action) {
    case DailyAction::Study:
        stats_.energy = clampStat(stats_.energy - 15);
        stats_.studyFocus = clampStat(stats_.studyFocus + 12);
        break;
    case DailyAction::Modding:
        stats_.energy = clampStat(stats_.energy - 20);
        stats_.modProgress = clampStat(stats_.modProgress + 10);
        stats_.dependence = clampStat(stats_.dependence + 2);
        break;
    case DailyAction::SpendTimeWithHarufushi:
        stats_.energy = clampStat(stats_.energy - 10);
        stats_.harufushiBond = clampStat(stats_.harufushiBond + 15);
        stats_.dependence = clampStat(stats_.dependence + 5);
        break;
    case DailyAction::Rest:
        ++stats_.day;
        stats_.energy = clampStat(stats_.energy + 25);
        break;
    }
    return true;
}

const DailyStats& DailyLoopState::stats() const {
    return stats_;
}

} // namespace haru::game::systems
