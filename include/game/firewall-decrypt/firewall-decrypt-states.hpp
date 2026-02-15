#pragma once

#include "state/state.hpp"
#include "utils/simple-timer.hpp"
#include "game/base-states/base-intro-state.hpp"
#include "game/base-states/base-win-state.hpp"
#include "game/base-states/base-lose-state.hpp"

class FirewallDecrypt;

/*
 * Firewall Decrypt state IDs — offset to 200+ to avoid collisions.
 */
enum FirewallDecryptStateId {
    DECRYPT_INTRO = 200,
    DECRYPT_SCAN = 201,
    DECRYPT_EVALUATE = 202,
    DECRYPT_WIN = 203,
    DECRYPT_LOSE = 204,
};

/*
 * DecryptIntro — Title screen. Shows "FIREWALL DECRYPT" for 2 seconds.
 * Seeds RNG and sets up the first round.
 */
class DecryptIntro : public BaseIntroState<FirewallDecrypt, DECRYPT_SCAN> {
public:
    explicit DecryptIntro(FirewallDecrypt* game);

    bool transitionToScan() const { return this->transitionCondition(); }

protected:
    const char* introTitle() const override { return "FIREWALL"; }
    const char* introSubtext() const override { return "DECRYPT"; }
    LEDState getIdleLedState() const override;

    int getTitleY() const override { return 20; }
    int getSubtextY() const override { return 40; }
    int getTitleX() const override { return 20; }
    int getSubtextX() const override { return 25; }

    void onIntroSetup(Device* PDN) override;
};

/*
 * DecryptScan — Main gameplay state.
 * Shows target address at top, scrollable candidate list below.
 * Primary = scroll, Secondary = confirm selection.
 * Optional time limit per round (hard mode).
 */
class DecryptScan : public State {
public:
    explicit DecryptScan(FirewallDecrypt* game);
    ~DecryptScan() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToEvaluate();

    int getSelectedIndex() const { return cursorIndex; }

private:
    FirewallDecrypt* game;
    SimpleTimer roundTimer;
    int cursorIndex = 0;
    bool transitionToEvaluateState = false;
    bool displayIsDirty = false;
    bool timedOut = false;

    void renderUi(Device* PDN);
};

/*
 * DecryptEvaluate — Checks the player's selection.
 * Correct: advance round or win.
 * Wrong/timeout: lose.
 */
class DecryptEvaluate : public State {
public:
    explicit DecryptEvaluate(FirewallDecrypt* game);
    ~DecryptEvaluate() override;

    void onStateMounted(Device* PDN) override;
    void onStateLoop(Device* PDN) override;
    void onStateDismounted(Device* PDN) override;
    bool transitionToScan();
    bool transitionToWin();
    bool transitionToLose();

private:
    FirewallDecrypt* game;
    bool transitionToScanState = false;
    bool transitionToWinState = false;
    bool transitionToLoseState = false;
};

/*
 * DecryptWin — Victory screen. Shows "DECRYPTED!" + score.
 * In managed mode, calls returnToPreviousApp.
 */
class DecryptWin : public BaseWinState<FirewallDecrypt, DECRYPT_INTRO> {
public:
    explicit DecryptWin(FirewallDecrypt* game);

protected:
    const char* victoryText() const override { return "DECRYPTED!"; }
    LEDState getWinLedState() const override;
    bool computeHardMode() const override;

    int getVictoryTextX() const override { return 15; }

    void logVictory(int score, bool isHard) const override;
};

/*
 * DecryptLose — Defeat screen. Shows "FIREWALL INTACT".
 * In managed mode, calls returnToPreviousApp.
 */
class DecryptLose : public BaseLoseState<FirewallDecrypt, DECRYPT_INTRO> {
public:
    explicit DecryptLose(FirewallDecrypt* game);

protected:
    const char* defeatText() const override { return "FIREWALL"; }
    LEDState getLoseLedState() const override;

    // Two-line defeat text: "FIREWALL" / "INTACT"
    void getDefeatTextLines(const char*& line1, const char*& line2) const override {
        line1 = "FIREWALL";
        line2 = "INTACT";
    }

    int getDefeatTextX() const override { return 20; }
    int getDefeatTextY() const override { return 20; }
    int getDefeatText2X() const override { return 30; }
    int getDefeatText2Y() const override { return 40; }

    bool showScoreOnLose() const override { return true; }

    void logDefeat(int score) const override;
};
