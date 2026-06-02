#include "DailyLoopState.hpp"

#include <algorithm>

namespace haru::game::systems {

namespace {

int clampStat(int value) {
    return std::clamp(value, 0, 100);
}

} // namespace

DailyLoopState::DailyLoopState(DailyStats stats) : stats_(stats) {}

void DailyLoopState::apply(DailyAction action) {
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
}

const DailyStats& DailyLoopState::stats() const {
    return stats_;
}

} // namespace haru::game::systems
