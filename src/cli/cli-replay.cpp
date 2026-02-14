#ifdef NATIVE_BUILD

#include "cli/cli-replay.hpp"
#include <algorithm>

namespace cli {

void ReplayManager::startRecording(const std::string& gameName, const std::string& difficulty, uint32_t seed) {
    if (recording) {
        // Already recording - cancel previous
        cancelRecording();
    }

    recording = true;
    currentRecording = ReplayRecord{};
    currentRecording.replayId = nextId;
    currentRecording.gameName = gameName;
    currentRecording.difficulty = difficulty;
    currentRecording.gameSeed = seed;
    currentRecording.timestamp = 0;  // Will be set on finish
    currentRecording.playerWon = false;
    currentRecording.durationMs = 0;
    currentRecording.events.clear();
    currentRecording.events.reserve(100);  // Pre-allocate some space
}

void ReplayManager::recordInput(const std::string& input, uint32_t gameTimeMs) {
    if (!recording) {
        return;
    }

    // Safety cap - prevent memory bloat
    if (currentRecording.events.size() >= MAX_EVENTS_PER_REPLAY) {
        return;
    }

    ReplayEvent event;
    event.timestampMs = gameTimeMs;
    event.input = input;
    currentRecording.events.push_back(event);
}

void ReplayManager::finishRecording(bool won, uint32_t durationMs) {
    if (!recording) {
        return;
    }

    currentRecording.playerWon = won;
    currentRecording.durationMs = durationMs;
    currentRecording.timestamp = 0;  // Session timestamp (could be set from global clock)

    // Add to replay buffer
    replays.push_back(currentRecording);

    // Increment ID for next recording
    nextId++;

    // Trim to max capacity
    trimToMaxReplays();

    // Reset recording state
    recording = false;
    currentRecording = ReplayRecord{};
}

void ReplayManager::cancelRecording() {
    recording = false;
    currentRecording = ReplayRecord{};
}

bool ReplayManager::startPlayback(uint32_t replayId) {
    const ReplayRecord* replay = findReplay(replayId);
    if (!replay) {
        return false;
    }

    playing = true;
    playbackReplayId = replayId;
    playbackEventIndex = 0;
    playbackStartMs = 0;  // Will be set by caller

    return true;
}

std::string ReplayManager::getNextInput(uint32_t currentPlaybackTimeMs) {
    if (!playing) {
        return "";
    }

    const ReplayRecord* replay = findReplay(playbackReplayId);
    if (!replay) {
        // Replay was deleted somehow
        stopPlayback();
        return "";
    }

    // Check if we've reached the end
    if (playbackEventIndex >= replay->events.size()) {
        // End of replay
        stopPlayback();
        return "";
    }

    // Check if the next event is due
    const ReplayEvent& nextEvent = replay->events[playbackEventIndex];
    if (currentPlaybackTimeMs >= nextEvent.timestampMs) {
        // Time to inject this input
        playbackEventIndex++;
        return nextEvent.input;
    }

    // Not time yet
    return "";
}

void ReplayManager::stopPlayback() {
    playing = false;
    playbackReplayId = 0;
    playbackEventIndex = 0;
    playbackStartMs = 0;
}

const ReplayRecord* ReplayManager::getCurrentReplay() const {
    if (!playing) {
        return nullptr;
    }
    return findReplay(playbackReplayId);
}

std::vector<ReplayRecord> ReplayManager::listReplays() const {
    // Return as vector, most recent first
    std::vector<ReplayRecord> result;
    result.reserve(replays.size());

    // Deque is in chronological order, so reverse it
    for (auto it = replays.rbegin(); it != replays.rend(); ++it) {
        result.push_back(*it);
    }

    return result;
}

const ReplayRecord* ReplayManager::getReplay(uint32_t replayId) const {
    return findReplay(replayId);
}

void ReplayManager::clearReplays() {
    replays.clear();
}

uint32_t ReplayManager::getLastReplayId() const {
    if (replays.empty()) {
        return 0;
    }
    return replays.back().replayId;
}

void ReplayManager::trimToMaxReplays() {
    while (replays.size() > MAX_REPLAYS) {
        // Remove oldest (front of deque)
        replays.pop_front();
    }
}

const ReplayRecord* ReplayManager::findReplay(uint32_t replayId) const {
    for (const auto& replay : replays) {
        if (replay.replayId == replayId) {
            return &replay;
        }
    }
    return nullptr;
}

} // namespace cli

#endif // NATIVE_BUILD
