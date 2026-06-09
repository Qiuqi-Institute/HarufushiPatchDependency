#include "support/TestHarness.hpp"

#include "game/systems/DailyDialogueScript.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string defaultDialogueSource() {
    const std::filesystem::path path =
        std::filesystem::path(HARUFUSHI_SOURCE_DIR) / "resources" / "data" /
        "scripts" / "daily_dialogues.harudlg";
    std::ifstream input(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::size_t countOccurrences(const std::string& source, const std::string& token) {
    std::size_t count = 0;
    std::size_t position = source.find(token);
    while (position != std::string::npos) {
        ++count;
        position = source.find(token, position + token.size());
    }
    return count;
}

} // namespace

HARU_TEST(daily_dialogue_script_parses_activity_dialogues_by_locale) {
    const std::string source =
        "harudlg v1\n"
        "dialogue en-US study Akioki\n"
        "line \"Notebook first. Harufushi keeps the chair warm.\"\n"
        "line \"One clean note now saves three confused commits later.\"\n"
        "dialogue zh-CN modding 春伏\n"
        "branch ui_review\n"
        "line \"你又在调 UI。\"\n";

    const auto script = haru::game::systems::DailyDialogueScript::parse(source);
    const auto study =
        script.entryFor("en-US", haru::game::systems::DailyAction::Study);
    const auto modding =
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Modding);
    const auto fallback =
        script.entryFor("ja-JP", haru::game::systems::DailyAction::Study);

    HARU_EXPECT_TRUE(study.has_value());
    HARU_EXPECT_EQ(study->branchId, "default");
    HARU_EXPECT_EQ(study->speaker, "Akioki");
    HARU_EXPECT_EQ(study->lines.size(), static_cast<std::size_t>(2));
    HARU_EXPECT_EQ(study->lines[0], "Notebook first. Harufushi keeps the chair warm.");
    HARU_EXPECT_TRUE(modding.has_value());
    HARU_EXPECT_EQ(modding->branchId, "ui_review");
    HARU_EXPECT_EQ(modding->speaker, "春伏");
    HARU_EXPECT_EQ(modding->lines[0], "你又在调 UI。");
    HARU_EXPECT_TRUE(fallback.has_value());
    HARU_EXPECT_EQ(fallback->speaker, "Akioki");
}

HARU_TEST(daily_dialogue_script_selects_story_branch_from_resulting_stats) {
    const std::string source =
        "harudlg v1\n"
        "dialogue en-US modding Harufushi\n"
        "branch routine_patch\n"
        "line \"The issue list is still breathing.\"\n"
        "dialogue en-US modding Harufushi\n"
        "branch release_candidate\n"
        "when mod >= 25\n"
        "when energy >= 20\n"
        "line \"The build light finally turned green.\"\n"
        "dialogue en-US harufushi Harufushi\n"
        "branch dependence_lock\n"
        "when dependence >= 8\n"
        "line \"Do not pretend this is just pair programming.\"\n";

    const auto script = haru::game::systems::DailyDialogueScript::parse(source);
    haru::game::systems::DailyStats routineStats;
    routineStats.modProgress = 20;
    routineStats.energy = 70;
    haru::game::systems::DailyStats releaseStats;
    releaseStats.modProgress = 31;
    releaseStats.energy = 35;
    haru::game::systems::DailyStats dependentStats;
    dependentStats.dependence = 10;

    const auto routine =
        script.entryFor("en-US", haru::game::systems::DailyAction::Modding, routineStats);
    const auto release =
        script.entryFor("en-US", haru::game::systems::DailyAction::Modding, releaseStats);
    const auto dependence =
        script.entryFor("en-US",
                        haru::game::systems::DailyAction::SpendTimeWithHarufushi,
                        dependentStats);

    HARU_EXPECT_TRUE(routine.has_value());
    HARU_EXPECT_EQ(routine->branchId, "routine_patch");
    HARU_EXPECT_EQ(routine->lines[0], "The issue list is still breathing.");
    HARU_EXPECT_TRUE(release.has_value());
    HARU_EXPECT_EQ(release->branchId, "release_candidate");
    HARU_EXPECT_EQ(release->lines[0], "The build light finally turned green.");
    HARU_EXPECT_TRUE(dependence.has_value());
    HARU_EXPECT_EQ(dependence->branchId, "dependence_lock");
}

HARU_TEST(daily_dialogue_script_default_resource_contains_four_daily_actions) {
    const auto script = haru::game::systems::DailyDialogueScript::loadDefault();

    HARU_EXPECT_TRUE(
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Study).has_value());
    HARU_EXPECT_TRUE(
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Modding).has_value());
    HARU_EXPECT_TRUE(script
                         .entryFor("zh-CN",
                                   haru::game::systems::DailyAction::SpendTimeWithHarufushi)
                         .has_value());
    HARU_EXPECT_TRUE(
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Rest).has_value());

    haru::game::systems::DailyStats modFocused;
    modFocused.modProgress = 42;
    modFocused.energy = 40;
    const auto modBranch =
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Modding, modFocused);
    HARU_EXPECT_TRUE(modBranch.has_value());
    HARU_EXPECT_EQ(modBranch->branchId, "release_candidate");

    haru::game::systems::DailyStats regressionStats;
    regressionStats.energy = 45;
    regressionStats.studyFocus = 40;
    regressionStats.modProgress = 66;
    regressionStats.dependence = 40;
    const auto regressionBranch =
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Modding, regressionStats);
    HARU_EXPECT_TRUE(regressionBranch.has_value());
    HARU_EXPECT_EQ(regressionBranch->branchId, "regression_night");

    haru::game::systems::DailyStats recoveredAfterLowEnergy;
    recoveredAfterLowEnergy.day = 2;
    recoveredAfterLowEnergy.energy = 55;
    const auto recoveryBranch =
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Rest, recoveredAfterLowEnergy);
    HARU_EXPECT_TRUE(recoveryBranch.has_value());
    HARU_EXPECT_EQ(recoveryBranch->branchId, "recovery_day");
}

HARU_TEST(daily_dialogue_script_default_resource_has_story_scale_chinese_text) {
    const std::string source = defaultDialogueSource();

    HARU_EXPECT_TRUE(countOccurrences(source, "dialogue zh-CN ") >= 28U);
    HARU_EXPECT_TRUE(countOccurrences(source, "dialogue zh-CN ") ==
                     countOccurrences(source, "dialogue en-US "));
    HARU_EXPECT_TRUE(countOccurrences(source, "dialogue zh-CN ") ==
                     countOccurrences(source, "dialogue ja-JP "));
    HARU_EXPECT_TRUE(countOccurrences(source, "line \"") >= 360U);
    HARU_EXPECT_TRUE(countOccurrences(source, "dialogue zh-CN ") * 18U <=
                     countOccurrences(source, "line \""));
    HARU_EXPECT_TRUE(source.find("dialogue zh-CN modding 春伏\nbranch midnight_incident") !=
                     std::string::npos);
    HARU_EXPECT_TRUE(source.find("dialogue zh-CN harufushi 春伏\nbranch boundary_confession") !=
                     std::string::npos);
    HARU_EXPECT_TRUE(source.find("dialogue zh-CN rest 秋起\nbranch clinic_morning") !=
                     std::string::npos);
}

HARU_TEST(daily_dialogue_script_default_resource_selects_story_chapter_branches) {
    const auto script = haru::game::systems::DailyDialogueScript::loadDefault();

    haru::game::systems::DailyStats manifestStats;
    manifestStats.day = 3;
    manifestStats.energy = 65;
    manifestStats.studyFocus = 22;
    manifestStats.modProgress = 28;
    const auto manifestBranch =
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Modding, manifestStats);
    HARU_EXPECT_TRUE(manifestBranch.has_value());
    HARU_EXPECT_EQ(manifestBranch->branchId, "broken_manifest");
    HARU_EXPECT_TRUE(manifestBranch->lines.size() >= 18U);

    haru::game::systems::DailyStats boundaryStats;
    boundaryStats.day = 5;
    boundaryStats.energy = 55;
    boundaryStats.harufushiBond = 58;
    boundaryStats.dependence = 34;
    const auto boundaryBranch =
        script.entryFor("zh-CN",
                        haru::game::systems::DailyAction::SpendTimeWithHarufushi,
                        boundaryStats);
    HARU_EXPECT_TRUE(boundaryBranch.has_value());
    HARU_EXPECT_EQ(boundaryBranch->branchId, "boundary_confession");
    HARU_EXPECT_TRUE(boundaryBranch->lines.size() >= 18U);

    haru::game::systems::DailyStats clinicStats;
    clinicStats.day = 4;
    clinicStats.energy = 40;
    const auto clinicBranch =
        script.entryFor("zh-CN", haru::game::systems::DailyAction::Rest, clinicStats);
    HARU_EXPECT_TRUE(clinicBranch.has_value());
    HARU_EXPECT_EQ(clinicBranch->branchId, "clinic_morning");
    HARU_EXPECT_TRUE(clinicBranch->lines.size() >= 18U);
}
