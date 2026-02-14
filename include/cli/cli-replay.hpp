#pragma once

#ifdef NATIVE_BUILD

#include <cstdint>
#include <string>
#include <vector>
#include <deque>

namespace cli {

/**
 * A single recorded input event during gameplay.
 */
struct ReplayEvent {
    uint32_t timestampMs;   // Time offset from game start (milliseconds)
    std::string input;      // The input command or button press ("b1_click", "b2_click", etc.)
};

/**
 * A complete replay record with all metadata and events.
 */
struct ReplayRecord {
    uint32_t replayId;              // Auto-incrementing unique ID
    std::string gameName;            // Which minigame (e.g., "Signal Echo")
    std::string difficulty;          // "easy" or "hard"
    bool playerWon;                  // Game outcome (true = win, false = loss)
    uint32_t durationMs;             // Total game duration in milliseconds
    uint32_t timestamp;              // When the game was played (seconds since session start)
    std::vector<ReplayEvent> events; // Recorded input sequence

    // Optional: store initial game seed for deterministic replay
    uint32_t gameSeed = 0;
};

/**
 * Manages recording and playback of game replays.
 *
 * During gameplay, the manager records all button inputs and their precise
 * timing. After the game ends, it stores the replay (up to MAX_REPLAYS).
 *
 * For playback, the manager feeds recorded inputs back at their original
 * timestamps, allowing faithful recreation of gameplay.
 */
class ReplayManager {
public:
    static const int MAX_REPLAYS = 10;
    static const size_t MAX_EVENTS_PER_REPLAY = 1000;  // Safety cap

    /**
     * Start recording a new game session.
     *
     * @param gameName Name of the minigame (e.g., "Signal Echo")
     * @param difficulty "easy" or "hard"
     * @param seed Optional random seed for deterministic replay
     */
    void startRecording(const std::string& gameName, const std::string& difficulty, uint32_t seed = 0);

    /**
     * Record a player input during gameplay.
     *
     * @param input The input action (e.g., "b1_click", "b2_long")
     * @param gameTimeMs Time since game started (milliseconds)
     */
    void recordInput(const std::string& input, uint32_t gameTimeMs);

    /**
     * Finish recording and save the replay.
     *
     * @param won Whether the player won the game
     * @param durationMs Total game duration in milliseconds
     */
    void finishRecording(bool won, uint32_t durationMs);

    /**
     * Cancel the current recording (game aborted/quit).
     */
    void cancelRecording();

    /**
     * Check if currently recording.
     */
    bool isRecording() const { return recording; }

    /**
     * Start playback of a specific replay.
     *
     * @param replayId The ID of the replay to play
     * @return True if replay was found and started, false otherwise
     */
    bool startPlayback(uint32_t replayId);

    /**
     * Check if currently playing a replay.
     */
    bool isPlaying() const { return playing; }

    /**
     * Get the next input to inject during playback.
     *
     * @param currentPlaybackTimeMs Current playback time (milliseconds since playback started)
     * @return The input string to inject, or empty if no input is due yet
     */
    std::string getNextInput(uint32_t currentPlaybackTimeMs);

    /**
     * Stop current playback.
     */
    void stopPlayback();

    /**
     * Get the currently playing replay (for display purposes).
     * Returns nullptr if not playing.
     */
    const ReplayRecord* getCurrentReplay() const;

    /**
     * List all stored replays (most recent first).
     */
    std::vector<ReplayRecord> listReplays() const;

    /**
     * Get a specific replay by ID.
     *
     * @param replayId The replay ID to fetch
     * @return Pointer to the replay, or nullptr if not found
     */
    const ReplayRecord* getReplay(uint32_t replayId) const;

    /**
     * Clear all stored replays.
     */
    void clearReplays();

    /**
     * Get the ID of the most recent replay, or 0 if none exist.
     */
    uint32_t getLastReplayId() const;

private:
    std::deque<ReplayRecord> replays;  // Stored replays (FIFO queue)
    uint32_t nextId = 1;                // Next replay ID to assign

    // Current recording state
    bool recording = false;
    ReplayRecord currentRecording;
    uint32_t recordingStartTime = 0;    // Session time when recording started

    // Current playback state
    bool playing = false;
    uint32_t playbackReplayId = 0;      // ID of the replay being played
    size_t playbackEventIndex = 0;      // Current event index
    uint32_t playbackStartMs = 0;       // When playback started (session time)

    /**
     * Trim replays to MAX_REPLAYS, removing oldest if needed.
     */
    void trimToMaxReplays();

    /**
     * Find a replay by ID in the deque.
     */
    const ReplayRecord* findReplay(uint32_t replayId) const;
};

} // namespace cli

#endif // NATIVE_BUILD
