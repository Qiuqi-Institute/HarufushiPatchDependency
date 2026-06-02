#pragma once

#include <string>

namespace haru::game::systems {

enum class ModProjectAction {
    DesignFeature,
    FixBug,
    ReadComments,
    PublishPreview,
};

struct ModProjectStats {
    int backlog = 3;
    int stability = 68;
    int releaseReadiness = 0;
    int feedbackHeat = 0;
    int dependencePressure = 0;
};

class ModProjectState {
public:
    void apply(ModProjectAction action);
    const ModProjectStats& stats() const;
    std::string currentEventKey() const;

private:
    ModProjectStats stats_;
    std::string currentEventKey_ = "mod.event.feature_request";
};

} // namespace haru::game::systems
