#include "game/quickdraw-states.hpp"
#include "game/quickdraw.hpp"

Lose::Lose(Player *player) : State(LOSE) {
    this->player = player;
}

Lose::~Lose() {
    player = nullptr;
}

void Lose::onStateMounted(Device *PDN) {
    auto display = PDN->getDisplay();
    display->invalidateScreen();

    BoldRetroUI::drawHeaderBar(display, "DEFEATED");

    display->drawText("X  X", 54, 25);
    display->drawText("X  X", 54, 30);

    BoldRetroUI::drawBorderedFrame(display, 35, 34, 58, 12);
    display->drawText("ATTEMPTS: 1/3", 42, 42);

    BoldRetroUI::drawCenteredText(display, "TRY AGAIN?", 54);

    display->render();

    loseTimer.setTimer(8000);

    AnimationConfig config;
    config.type = AnimationType::LOSE;
    config.loop = true;
    config.speed = 16;
    config.initialState = LEDState();
    config.loopDelayMs = 0;

    PDN->getLightManager()->startAnimation(config);
}

void Lose::onStateLoop(Device *PDN) {
    loseTimer.updateTime();
    if(loseTimer.expired()) {
        reset = true;
    }
}

void Lose::onStateDismounted(Device *PDN) {
    loseTimer.invalidate();
    reset = false;
}

bool Lose::resetGame() {
    return reset;
}

bool Lose::isTerminalState() {
    return true;
}
