#include "ModProjectState.hpp"

#include <algorithm>

namespace haru::game::systems {

namespace {

int clampProjectStat(int value) {
    return std::clamp(value, 0, 100);
}

} // namespace

void ModProjectState::apply(ModProjectAction action) {
    switch (action) {
    case ModProjectAction::DesignFeature:
        stats_.backlog = clampProjectStat(stats_.backlog + 1);
        stats_.releaseReadiness = clampProjectStat(stats_.releaseReadiness + 12);
        currentEventKey_ = "mod.event.regression_bug";
        break;
    case ModProjectAction::FixBug:
        stats_.backlog = clampProjectStat(stats_.backlog - 1);
        stats_.stability = clampProjectStat(stats_.stability + 14);
        stats_.releaseReadiness = clampProjectStat(stats_.releaseReadiness + 6);
        currentEventKey_ = "mod.event.harufushi_ping";
        break;
    case ModProjectAction::ReadComments:
        stats_.feedbackHeat = clampProjectStat(stats_.feedbackHeat + 9);
        stats_.dependencePressure = clampProjectStat(stats_.dependencePressure + 3);
        currentEventKey_ = "mod.event.harufushi_ping";
        break;
    case ModProjectAction::PublishPreview:
        stats_.releaseReadiness = 0;
        stats_.feedbackHeat = clampProjectStat(stats_.feedbackHeat + 24);
        currentEventKey_ = "mod.event.feedback_wave";
        break;
    }
}

const ModProjectStats& ModProjectState::stats() const {
    return stats_;
}

std::string ModProjectState::currentEventKey() const {
    return currentEventKey_;
}

} // namespace haru::game::systems
