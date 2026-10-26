#include "Gauntlet.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <unordered_set>

namespace {

const char* kSaveHeader = "QUANTAPIE_GAUNTLET";
const int kSaveVersion = 3;

// One-time tutorial tips, by the bit they use in PlayerStats.
enum Tip {
    kTipHealth,
    kTipHazard,
    kTipShaken,
    kTipDash,
    kTipRefill,
    kTipCollapse
};

std::string amounts(int ei, int r) {
    if (ei > 0 && r > 0) {
        return std::to_string(ei) + " EI and " + std::to_string(r) + " R";
    }
    if (r > 0) {
        return std::to_string(r) + " R";
    }
    return std::to_string(ei) + " EI";
}

std::string kindLabel(EquipmentKind kind) {
    return kind == EquipmentKind::EmotionalIntelligence ? "EI" : "R";
}

long long steadyMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

} // namespace

const std::string Gauntlet::kDefaultDataDirectory = "sql";

Gauntlet::Gauntlet(const std::string& data_directory, const GameOptions& options)
    : Gauntlet(WorldData::loadFromDirectory(data_directory), options) {
}

Gauntlet::Gauntlet(const WorldData& world, const GameOptions& options)
    : world(world), room_count(static_cast<int>(world.rooms.size())), options(options),
      state(GameState::Playing), resume_state(GameState::Playing), view(GameView::Room), ending(Ending::None),
      current_room(1), current_slot(0), dash_cooldown(0), health_change(0), quit_requested(false),
      clock(steadyMilliseconds), last_tick(0) {
    newGame();
}

void Gauntlet::newGame() {
    newGame(options);
}

void Gauntlet::newGame(const GameOptions& new_options) {
    options = new_options;
    environment = std::make_unique<Environment>(world);
    player = std::make_unique<Player>(1, "Integration Specialist", "N/A", environment->getStartingRoom());
    player->setMaxHealth(traitsOf(options.background).max_health);
    stats = PlayerStats();
    log.clear();
    events.clear();
    state = GameState::Playing;
    view = GameView::Room;
    ending = Ending::None;
    current_slot = 0;
    dash_cooldown = 0;
    health_change = 0;
    quit_requested = false;
    last_tick = clock();
    enterRoom(1, environment->getLayout(1).entry);
}

const GameOptions& Gauntlet::getOptions() const {
    return options;
}

bool Gauntlet::handleKey(char key) {
    if (state == GameState::ChoosingSaveSlot || state == GameState::ChoosingLoadSlot) {
        updateTimer();
        int slot = key - '0';
        if (slot >= 1 && slot <= kSaveSlots) {
            return chooseSlot(slot);
        }
        state = resume_state;
        emit(GameEventType::Ignored, "Cancelled.");
        return false;
    }
    return execute(bindings.commandFor(key));
}

bool Gauntlet::execute(Command command) {
    updateTimer();
    if (command == Command::None) {
        return false;
    }
    if (state == GameState::ChoosingSaveSlot || state == GameState::ChoosingLoadSlot) {
        state = resume_state;
        emit(GameEventType::Ignored, "Cancelled.");
        return false;
    }

    // Commands that work in every state and never take a turn
    switch (command) {
        case Command::Quit:
            quit_requested = true;
            return true;
        case Command::Save:
            if (state == GameState::Defeated) {
                emit(GameEventType::Ignored, "There is nothing left to save. Press " + keyHint(Command::NewGame) +
                     " for a new run.");
                return false;
            }
            if (options.hardcore && current_slot > 0) {
                return saveToSlot(current_slot); // A hardcore run keeps one save
            }
            resume_state = state;
            state = GameState::ChoosingSaveSlot;
            emit(GameEventType::Prompt, "Save to which slot? Press 1-" + std::to_string(kSaveSlots) +
                 " (any other key cancels).");
            return true;
        case Command::Load:
            if (options.hardcore) {
                emit(GameEventType::Ignored, "Hardcore runs cannot be reloaded.");
                return false;
            }
            resume_state = state;
            state = GameState::ChoosingLoadSlot;
            emit(GameEventType::Prompt, "Load which slot? Press 1-" + std::to_string(kSaveSlots) +
                 " (any other key cancels).");
            return true;
        case Command::NewGame:
            if (state != GameState::Completed && state != GameState::Defeated) {
                emit(GameEventType::Ignored, "A new run can start once this one is over.");
                return false;
            }
            newGame();
            return true;
        case Command::ShowInventory:
        case Command::ShowStats:
        case Command::ShowBestiary:
        case Command::ShowHelp: {
            GameView target = command == Command::ShowInventory ? GameView::Inventory
                            : command == Command::ShowStats     ? GameView::Stats
                            : command == Command::ShowBestiary  ? GameView::Bestiary
                                                                : GameView::Help;
            view = view == target ? GameView::Room : target;
            return true;
        }
        default:
            break;
    }

    view = GameView::Room;
    health_change = 0;
    Direction direction;
    bool is_move = KeyBindings::directionFor(command, direction);

    switch (state) {
        case GameState::Looking:
            state = GameState::Playing;
            if (is_move) {
                emit(GameEventType::Looked, describe(direction));
                return true;
            }
            emit(GameEventType::Looked, "You stop looking around.");
            return false;
        case GameState::Dashing:
            state = GameState::Playing;
            if (is_move) {
                return dash(direction);
            }
            emit(GameEventType::Ignored, "You decide not to dash.");
            return false;
        case GameState::Paused:
            if (command == Command::Pause) {
                state = GameState::Playing;
                emit(GameEventType::Resumed, "Resumed.");
                return true;
            }
            emit(GameEventType::Ignored, "The game is paused. Press " + keyHint(Command::Pause) + " to resume.");
            return false;
        case GameState::Completed:
            emit(GameEventType::Ignored, "Every obstacle is cleared. Press " + keyHint(Command::NewGame) +
                 " for a new run.");
            return false;
        case GameState::Defeated:
            emit(GameEventType::Ignored, "This run is over. Press " + keyHint(Command::NewGame) + " for a new one.");
            return false;
        case GameState::Playing:
        case GameState::ChoosingSaveSlot:
        case GameState::ChoosingLoadSlot:
            break;
    }

    if (is_move) {
        return step(direction);
    }
    switch (command) {
        case Command::Wait:
            return wait();
        case Command::Look:
            state = GameState::Looking;
            emit(GameEventType::Prompt, "Look which way? Press a direction key.");
            return true;
        case Command::Dash:
            if (dash_cooldown > 0) {
                emit(GameEventType::Ignored, "You need " + std::to_string(dash_cooldown) +
                     " more turn" + (dash_cooldown == 1 ? "" : "s") + " before you can dash again.");
                return false;
            }
            state = GameState::Dashing;
            emit(GameEventType::Prompt, "Dash which way? Press a direction key.");
            return true;
        case Command::Clear:
            return clearObstacle();
        case Command::Pause:
            state = GameState::Paused;
            emit(GameEventType::Paused, "Paused. Press " + keyHint(Command::Pause) + " to resume.");
            return true;
        default:
            return false;
    }
}

bool Gauntlet::step(Direction direction) {
    const RoomLayout& layout = getCurrentLayout();
    Position from = player->getPosition();
    Position target = from.step(direction);

    if (!layout.contains(target)) {
        if (direction == Direction::North && from == layout.door) {
            return changeRoom(Environment::kForward);
        }
        if (direction == Direction::South && from == layout.entry) {
            return changeRoom(Environment::kBack);
        }
        return blocked("A wall blocks the way " + directionName(direction) + ".");
    }
    if (Obstacle* obstacle = obstacleAt(target)) {
        return blocked(obstacle->getName() + " blocks the way. Stand beside it and press " +
                       keyHint(Command::Clear) + " to work through it.");
    }

    payMove();
    stats.recordStep();
    moveTo(target);
    endTurn();
    checkCollapse();
    return true;
}

bool Gauntlet::dash(Direction direction) {
    const RoomLayout& layout = getCurrentLayout();
    Position at = player->getPosition();
    int moved = 0;
    for (; moved < kDashDistance; ++moved) {
        Position next = at.step(direction);
        if (!layout.contains(next) || obstacleAt(next)) {
            break;
        }
        if (moved == 0) {
            payMove(); // The whole dash costs one move
        }
        at = next;
        stats.recordStep();
        moveTo(at);
        if (player->isDefeated()) {
            moved++;
            break; // Collapsed mid-dash
        }
    }
    if (moved == 0) {
        return blocked("There is no room to dash " + directionName(direction) + ".");
    }
    stats.recordDash();
    emit(GameEventType::Dashed, "You dash " + directionName(direction) + " " + std::to_string(moved) +
         (moved == 1 ? " tile." : " tiles."));
    endTurn();
    dash_cooldown = traitsOf(options.background).dash_cooldown;
    checkCollapse();
    return true;
}

bool Gauntlet::changeRoom(const std::string& exit) {
    if (player->getBlockingObstacle(exit)) {
        return blocked("The way " + exit + " stays shut until every obstacle in this room is cleared.");
    }
    if (!player->move(exit)) {
        return blocked(exit == Environment::kForward ? "There is no way further north."
                                                     : "There is no way back from here.");
    }
    int next_room = environment->getRoomNumber(player->getCurrentRoom());
    payMove();
    if (exit == Environment::kBack) {
        stats.recordBacktrack();
        enterRoom(next_room, environment->getLayout(next_room).door);
    } else {
        enterRoom(next_room, environment->getLayout(next_room).entry);
    }
    endTurn();
    checkCollapse();
    return true;
}

void Gauntlet::moveTo(const Position& position) {
    player->setPosition(position);
    collectAt(position);
    if (const Hazard* hazard = getCurrentLayout().hazardAt(position)) {
        int before = player->getHealth();
        loseHealth(hazard->damage);
        stats.recordHazard();
        emit(GameEventType::HazardHit, "You step into a " + hazard->name + ": -" +
             std::to_string(before - player->getHealth()) + " health.", before - player->getHealth());
        tip(kTipHazard);
    }
}

Obstacle* Gauntlet::obstacleAt(const Position& position) const {
    for (RoomObject* object : getCurrentRoom()->getObjects()) {
        Obstacle* obstacle = dynamic_cast<Obstacle*>(object);
        if (obstacle && !obstacle->isCleared() && obstacle->getPosition() == position) {
            return obstacle;
        }
    }
    return nullptr;
}

bool Gauntlet::blocked(const std::string& message) {
    stats.recordBlockedMove();
    emit(GameEventType::Blocked, message);
    return false;
}

void Gauntlet::collectAt(const Position& position) {
    // Copy first: taking equipment removes it from the room we are iterating.
    std::vector<RoomObject*> objects = getCurrentRoom()->getObjects();
    for (RoomObject* object : objects) {
        Equipment* equipment = dynamic_cast<Equipment*>(object);
        if (!equipment || equipment->getPosition() != position) {
            continue;
        }
        if (player->takeTool(equipment)) {
            stats.recordPickup(equipment->getKind());
            emit(GameEventType::PickedUp, "You pick up the " + equipment->getName() + " (+" +
                 std::to_string(equipment->getPoints()) + " " + kindLabel(equipment->getKind()) + ").");
        } else {
            emit(GameEventType::InventoryFull, "You cannot carry the " + equipment->getName() + ": load " +
                 std::to_string(player->getCarryWeight()) + "/" + std::to_string(player->getCarryCapacity()) + ".");
        }
    }
}

bool Gauntlet::wait() {
    stats.recordWait();
    if (player->hasStatus(StatusEffect::Shaken)) {
        player->removeStatus(StatusEffect::Shaken);
        emit(GameEventType::StatusChanged, "You take a slow breath and feel steady again.");
    } else {
        emit(GameEventType::Waited, "You pause and take a slow breath.");
    }
    endTurn();
    return true;
}

bool Gauntlet::clearObstacle() {
    Obstacle* obstacle = getActiveObstacle();
    if (!obstacle) {
        emit(GameEventType::CannotClear, "Nothing here needs clearing. The way north is open.");
        return false;
    }
    if (player->getPosition().distanceTo(obstacle->getPosition()) > 1) {
        // Say so if the player is beside a later obstacle that is not ready yet.
        for (Obstacle* waiting : environment->getObstacles(current_room)) {
            if (!waiting->isCleared() && player->getPosition().distanceTo(waiting->getPosition()) <= 1) {
                emit(GameEventType::CannotClear, waiting->getName() + " has to wait: work through " +
                     obstacle->getLabel() + " first.");
                return false;
            }
        }
        emit(GameEventType::CannotClear, "Stand next to " + obstacle->getLabel() + " to work through it.");
        return false;
    }
    int ei = obstacle->getRequiredEmotionalIntelligence();
    int r = obstacle->getRequiredResilience();
    if (!player->clearObstacle(obstacle)) {
        // Facing an obstacle unprepared is an attack it answers.
        emit(GameEventType::CannotClear, obstacle->getName() + " needs " + amounts(ei, r) + "; you carry " +
             std::to_string(player->getEmotionalIntelligence()) + " EI and " +
             std::to_string(player->getResilience()) + " R.");
        int before = player->getHealth();
        loseHealth(kStrikePerType * obstacle->getType());
        stats.recordStrike();
        emit(GameEventType::Struck, obstacle->getName() + " strikes back: -" +
             std::to_string(before - player->getHealth()) + " health.", before - player->getHealth());
        endTurn();
        if (!traitsOf(options.background).shaken_immune && !player->isDefeated()) {
            player->addStatus(StatusEffect::Shaken, kShakenTurns);
            emit(GameEventType::StatusChanged, "You feel shaken: moves cost double until you wait (" +
                 keyHint(Command::Wait) + ") to breathe.");
            tip(kTipShaken);
        }
        checkCollapse();
        return false;
    }

    int healed = player->heal(kHealBase + kHealPerType * obstacle->getType());
    stats.recordHealing(healed);
    health_change += healed;
    Obstacle* next = getActiveObstacle();
    emit(GameEventType::Cleared, "You work through " + obstacle->getName() + ", spending " + amounts(ei, r) +
         " (+" + std::to_string(healed) + " health)." +
         (next ? " Next: " + next->getLabel() + "." : " The way north is open."), healed);
    if (next && environment->spawnEquipmentForActiveObstacle(current_room) > 0) {
        emit(GameEventType::EquipmentSpawned, "New equipment appears for " + next->getName() + ".");
        tip(kTipRefill);
    }
    endTurn();
    int total_obstacles = environment->getObstacleCount();
    for (Achievement achievement : stats.recordClear(obstacle->getType(), ei, r, total_obstacles)) {
        emit(GameEventType::AchievementUnlocked, "Achievement unlocked: " + PlayerStats::achievementName(achievement) + ".");
    }
    if (stats.getObstaclesCleared() >= total_obstacles) {
        finish();
    }
    return true;
}

void Gauntlet::payMove() {
    loseHealth(player->hasStatus(StatusEffect::Shaken) ? kShakenStepCost : kStepCost);
    tip(kTipHealth);
}

void Gauntlet::loseHealth(int amount) {
    int lost = player->damage(amount);
    stats.recordDamage(lost);
    health_change -= lost;
}

void Gauntlet::endTurn() {
    stats.recordTurn();
    bool was_shaken = player->hasStatus(StatusEffect::Shaken);
    player->tickStatuses();
    if (was_shaken && !player->hasStatus(StatusEffect::Shaken)) {
        emit(GameEventType::StatusChanged, "The shakiness passes.");
    }
    if (dash_cooldown > 0) {
        dash_cooldown--;
    }
}

void Gauntlet::checkCollapse() {
    if (!player->isDefeated()) {
        return;
    }
    if (options.hardcore) {
        state = GameState::Defeated;
        ending = Ending::Rest;
        std::string lost_save;
        if (current_slot > 0) {
            std::remove(slotPath(current_slot).c_str());
            lost_save = " Its save in slot " + std::to_string(current_slot) + " is gone.";
        }
        emit(GameEventType::Defeated, "You collapse. On hardcore, the run ends here." + lost_save + " " + getEndingText());
        return;
    }
    stats.recordCollapse();
    player->restoreHealth();
    player->removeStatus(StatusEffect::Shaken);
    player->incrementScore(-std::min(kCollapseScorePenalty, player->getScore()));
    player->setPosition(getCurrentLayout().entry);
    emit(GameEventType::Collapsed, "You collapse, and come to at the room's entry with your health restored (-" +
         std::to_string(kCollapseScorePenalty) + " score).");
    tip(kTipCollapse);
}

void Gauntlet::finish() {
    state = GameState::Completed;
    ending = stats.getCollapses() == 0 ? Ending::Unbroken : Ending::RisingAgain;
    emit(GameEventType::Completed, "You have cleared all " + std::to_string(room_count) + " rooms. Ending: " +
         endingTitle(ending) + ". " + getEndingText());
}

bool Gauntlet::chooseSlot(int slot) {
    GameState choosing = state;
    state = resume_state;
    return choosing == GameState::ChoosingSaveSlot ? saveToSlot(slot) : loadFromSlot(slot);
}

void Gauntlet::enterRoom(int room_number, const Position& arrival) {
    current_room = room_number;
    player->setCurrentRoom(environment->getRoom(room_number));
    player->setPosition(arrival);
    stats.recordRoomReached(room_number);

    int remaining = 0;
    for (Obstacle* obstacle : environment->getObstacles(room_number)) {
        stats.recordSeen(obstacle->getType());
        remaining += obstacle->isCleared() ? 0 : 1;
    }
    std::string message = "You enter " + getRoomTitle(room_number) + ".";
    if (remaining == 1) {
        message += " " + getActiveObstacle()->getLabel() + " blocks the north door.";
    } else if (remaining > 1) {
        message += " " + std::to_string(remaining) + " obstacles block the north door.";
    }
    emit(GameEventType::RoomEntered, message);
    collectAt(arrival);
    if (room_number > 1) {
        tip(kTipDash);
    }
}

void Gauntlet::tip(int which) {
    static const char* const kTips[] = {
        "Tip: every move costs health. Clearing an obstacle restores 10, plus 3 per type, so plan short routes.",
        "Tip: ':' tiles are hazards that cost extra health. Walk around them when you can.",
        "Tip: while shaken, moves cost double. Wait to breathe and steady yourself.",
        "Tip: press 'r' and a direction to dash two tiles for the cost of one move.",
        "Tip: each cleared obstacle brings out the equipment for the next one on the same tiles.",
        "Tip: collapsing sends you back to the room's entry and costs score. On hardcore it ends the run.",
    };
    if (options.tutorial && stats.markTipShown(which)) {
        emit(GameEventType::Tip, kTips[which]);
    }
}

bool Gauntlet::timerRunning() const {
    return state != GameState::Paused && state != GameState::Completed && state != GameState::Defeated;
}

void Gauntlet::updateTimer() {
    long long now = clock();
    if (timerRunning() && now > last_tick) {
        stats.addElapsed(now - last_tick);
    }
    last_tick = now;
}

void Gauntlet::setClock(Clock new_clock) {
    clock = std::move(new_clock);
    last_tick = clock();
}

long long Gauntlet::getElapsedMilliseconds() const {
    long long running = timerRunning() ? std::max(0LL, clock() - last_tick) : 0;
    return stats.getElapsedMilliseconds() + running;
}

void Gauntlet::emit(GameEventType type, const std::string& message, int amount) {
    MessageTone tone = MessageTone::Neutral;
    switch (type) {
        case GameEventType::Struck:
        case GameEventType::HazardHit:
        case GameEventType::Collapsed:
        case GameEventType::Defeated:
        case GameEventType::CannotClear:
        case GameEventType::InventoryFull:
        case GameEventType::SaveFailed:
        case GameEventType::LoadFailed:
            tone = MessageTone::Bad;
            break;
        case GameEventType::Cleared:
        case GameEventType::Healed:
        case GameEventType::PickedUp:
        case GameEventType::AchievementUnlocked:
        case GameEventType::Completed:
        case GameEventType::Saved:
        case GameEventType::Loaded:
            tone = MessageTone::Good;
            break;
        case GameEventType::Tip:
            tone = MessageTone::Tip;
            break;
        default:
            break;
    }
    log.add(message, tone);
    events.push_back({type, message, amount});
    while (events.size() > kMaxPendingEvents) {
        events.pop_front();
    }
}

bool Gauntlet::pollEvent(GameEvent& event) {
    if (events.empty()) {
        return false;
    }
    event = events.front();
    events.pop_front();
    return true;
}

GameState Gauntlet::getState() const { return state; }
GameView Gauntlet::getView() const { return view; }
bool Gauntlet::isQuitRequested() const { return quit_requested; }
Ending Gauntlet::getEnding() const { return ending; }
const Environment& Gauntlet::getEnvironment() const { return *environment; }
const Player& Gauntlet::getPlayer() const { return *player; }
const PlayerStats& Gauntlet::getStats() const { return stats; }
const MessageLog& Gauntlet::getMessageLog() const { return log; }
KeyBindings& Gauntlet::getKeyBindings() { return bindings; }
const KeyBindings& Gauntlet::getKeyBindings() const { return bindings; }
int Gauntlet::getRoomCount() const { return room_count; }
int Gauntlet::getCurrentRoomNumber() const { return current_room; }
int Gauntlet::getDashCooldown() const { return dash_cooldown; }
int Gauntlet::getLastHealthChange() const { return health_change; }
int Gauntlet::getCurrentSlot() const { return current_slot; }

std::string Gauntlet::endingTitle(Ending ending) {
    switch (ending) {
        case Ending::Unbroken:    return "Unbroken";
        case Ending::RisingAgain: return "Rising Again";
        case Ending::Rest:        return "Rest";
        case Ending::None:        break;
    }
    return "";
}

std::string Gauntlet::getEndingText() const {
    switch (ending) {
        case Ending::Unbroken:
            return "You crossed every room without once losing your footing.";
        case Ending::RisingAgain:
            return "You fell " + std::to_string(stats.getCollapses()) +
                   (stats.getCollapses() == 1 ? " time" : " times") +
                   " and got back up every time. That is what resilience means.";
        case Ending::Rest:
            return "Your strength ran out. Rest, and begin again when you are ready.";
        case Ending::None:
            break;
    }
    return "";
}

std::string Gauntlet::getRoomTitle(int room_number) const {
    return "Room " + std::to_string(room_number) + ": " + environment->getRoomName(room_number);
}
Room* Gauntlet::getCurrentRoom() const { return environment->getRoom(current_room); }
const RoomLayout& Gauntlet::getCurrentLayout() const { return environment->getLayout(current_room); }
Obstacle* Gauntlet::getActiveObstacle() const { return environment->getActiveObstacle(current_room); }

std::string Gauntlet::keyHint(Command command) const {
    std::vector<char> keys = bindings.keysFor(command);
    if (keys.empty()) {
        return "(unbound: " + KeyBindings::commandName(command) + ")";
    }
    return std::string("'") + keys.front() + "'";
}

std::string Gauntlet::describe(Direction direction) const {
    const RoomLayout& layout = getCurrentLayout();
    Position from = player->getPosition();
    Position target = from.step(direction);
    std::string where = "To the " + directionName(direction) + ": ";

    if (!layout.contains(target)) {
        bool at_exit = (direction == Direction::North && from == layout.door) ||
                       (direction == Direction::South && from == layout.entry);
        std::string exit = environment->getExitDescription(current_room, directionName(direction));
        if (at_exit && !exit.empty()) {
            return where + exit;
        }
        return where + (direction == Direction::North && from == layout.door ? "the end of your journey." : "a wall.");
    }
    for (RoomObject* object : getCurrentRoom()->getObjects()) {
        if (object->getPosition() != target) {
            continue;
        }
        if (Obstacle* obstacle = dynamic_cast<Obstacle*>(object)) {
            Obstacle* active = getActiveObstacle();
            return where + obstacle->getLabel() + " (type " + std::to_string(obstacle->getType()) + ", needs " +
                   amounts(obstacle->getRequiredEmotionalIntelligence(), obstacle->getRequiredResilience()) +
                   "). " + obstacle->getDescription() +
                   (obstacle != active ? " It waits behind " + active->getLabel() + "." : "");
        }
        if (Equipment* equipment = dynamic_cast<Equipment*>(object)) {
            return where + equipment->getName() + " (+" + std::to_string(equipment->getPoints()) + " " +
                   kindLabel(equipment->getKind()) + "). " + equipment->getDescription();
        }
    }
    if (const Hazard* hazard = layout.hazardAt(target)) {
        return where + hazard->name + " (-" + std::to_string(hazard->damage) + " health). " + hazard->description;
    }
    return where + "open floor.";
}

std::string Gauntlet::getObjective() const {
    if (state == GameState::Defeated) {
        return "The run is over. Press " + keyHint(Command::NewGame) + " to begin a new one.";
    }
    if (state == GameState::Completed) {
        return "Every obstacle is cleared. You have finished all " + std::to_string(room_count) + " rooms.";
    }
    Obstacle* obstacle = getActiveObstacle();
    if (!obstacle) {
        int next_room = environment->getRoomNumber(getCurrentRoom()->getExit(Environment::kForward));
        return next_room > 0 ? "Head north through the door to Room " + std::to_string(next_room) + "."
                             : "Head north through the door.";
    }
    std::string clear_hint = "stand next to " + obstacle->getLabel() + " and press " + keyHint(Command::Clear);
    int missing_ei = std::max(0, obstacle->getRequiredEmotionalIntelligence() - player->getEmotionalIntelligence());
    int missing_r = std::max(0, obstacle->getRequiredResilience() - player->getResilience());
    if (missing_ei > 0 || missing_r > 0) {
        return "Collect " + amounts(missing_ei, missing_r) + " from the E and R tiles, then " + clear_hint + ".";
    }
    return "Clear " + obstacle->getLabel() + ": " + clear_hint + ".";
}

std::string Gauntlet::slotPath(int slot) const {
    return options.save_directory + "/quantapie_slot" + std::to_string(slot) + ".sav";
}

bool Gauntlet::saveToSlot(int slot) {
    if (slot < 1 || slot > kSaveSlots) {
        return false;
    }
    bool saved = saveToFile(slotPath(slot));
    if (saved) {
        current_slot = slot;
    }
    emit(saved ? GameEventType::Saved : GameEventType::SaveFailed,
         saved ? "Game saved to slot " + std::to_string(slot) + "." : "Could not save to " + slotPath(slot) + ".");
    return saved;
}

bool Gauntlet::loadFromSlot(int slot) {
    bool loaded = slot >= 1 && slot <= kSaveSlots && loadFromFile(slotPath(slot));
    if (loaded) {
        current_slot = slot;
    }
    emit(loaded ? GameEventType::Loaded : GameEventType::LoadFailed,
         loaded ? "Slot " + std::to_string(slot) + " loaded. You are in " + getRoomTitle(current_room) + "."
                : "Slot " + std::to_string(slot) + " has no saved game that can be loaded.");
    return loaded;
}

bool Gauntlet::save(std::ostream& out) const {
    std::unordered_set<const Tool*> carried(player->getTools().begin(), player->getTools().end());
    Position position = player->getPosition();

    out << kSaveHeader << ' ' << kSaveVersion << '\n'
        << "rooms " << room_count << '\n'
        << "room " << current_room << '\n'
        << "position " << position.x << ' ' << position.y << '\n'
        << "score " << player->getScore() << '\n'
        << "background " << traitsOf(options.background).name << '\n'
        << "hardcore " << (options.hardcore ? 1 : 0) << '\n'
        << "health " << player->getHealth() << '\n'
        << "shaken " << player->getStatusTurns(StatusEffect::Shaken) << '\n'
        << "dash " << dash_cooldown << '\n'
        << "obstacles ";
    for (const auto& obstacle : environment->getAllObstacles()) {
        out << (obstacle->isCleared() ? '1' : '0');
    }
    // Each piece of equipment is 0 = waiting to spawn, 1 = on its spawn point, 2 = carried, 3 = consumed.
    out << "\nequipment ";
    for (const auto& equipment : environment->getEquipment()) {
        out << (!equipment->isSpawned()                ? '0'
              : equipment->isConsumed()                ? '3'
              : carried.count(equipment.get()) ? '2' : '1');
    }
    out << "\nstats ";
    PlayerStats snapshot = stats; // Include the play time since the last key press
    snapshot.addElapsed(getElapsedMilliseconds() - stats.getElapsedMilliseconds());
    snapshot.write(out);
    out << "\nend\n";
    return static_cast<bool>(out);
}

bool Gauntlet::load(std::istream& in) {
    std::string header;
    int version = 0;
    if (!(in >> header >> version) || header != kSaveHeader || version != kSaveVersion) {
        return false;
    }
    auto expect = [&in](const char* label) {
        std::string word;
        return (in >> word) && word == label;
    };
    int rooms = 0;
    int room = 0;
    int score = 0;
    int hardcore = 0;
    int health = 0;
    int shaken = 0;
    int dash = 0;
    Position position;
    std::string background_name;
    std::string obstacle_states;
    std::string equipment_states;
    PlayerStats loaded_stats;
    if (!expect("rooms") || !(in >> rooms) || rooms != room_count ||
        !expect("room") || !(in >> room) ||
        !expect("position") || !(in >> position.x >> position.y) ||
        !expect("score") || !(in >> score) ||
        !expect("background") || !(in >> background_name) ||
        !expect("hardcore") || !(in >> hardcore) ||
        !expect("health") || !(in >> health) ||
        !expect("shaken") || !(in >> shaken) ||
        !expect("dash") || !(in >> dash) ||
        !expect("obstacles") || !(in >> obstacle_states) ||
        !expect("equipment") || !(in >> equipment_states) ||
        !expect("stats") || !loaded_stats.read(in) ||
        !expect("end")) {
        return false;
    }
    Background background;
    if (room < 1 || room > rooms || !backgroundFromName(background_name, background) ||
        health < 1 || health > traitsOf(background).max_health || shaken < 0 || dash < 0 ||
        (hardcore != 0 && hardcore != 1)) {
        return false;
    }

    // Rebuild the world from the loaded data, then replay the saved state onto it.
    auto loaded_environment = std::make_unique<Environment>(world);
    const auto& all_obstacles = loaded_environment->getAllObstacles();
    const auto& all_equipment = loaded_environment->getEquipment();
    if (obstacle_states.size() != all_obstacles.size() || equipment_states.size() != all_equipment.size() ||
        !loaded_environment->getLayout(room).contains(position)) {
        return false;
    }
    auto loaded_player = std::make_unique<Player>(1, "Integration Specialist", "N/A", loaded_environment->getRoom(room));
    loaded_player->setPosition(position);
    loaded_player->incrementScore(score);
    loaded_player->setMaxHealth(traitsOf(background).max_health);
    loaded_player->damage(traitsOf(background).max_health - health);
    loaded_player->addStatus(StatusEffect::Shaken, shaken);

    for (size_t i = 0; i < all_obstacles.size(); ++i) {
        Obstacle* obstacle = all_obstacles[i].get();
        if (obstacle_states[i] == '1') {
            obstacle->clear();
            loaded_environment->getRoom(obstacle->getRoomId())->removeObject(obstacle);
        } else if (obstacle_states[i] != '0') {
            return false;
        } else if (obstacle->getRoomId() == room && obstacle->getPosition() == position) {
            return false; // The player cannot stand on an obstacle
        }
    }
    for (size_t i = 0; i < all_equipment.size(); ++i) {
        Equipment* equipment = all_equipment[i].get();
        Room* origin = loaded_environment->getRoom(equipment->getRoomId());
        char saved = equipment_states[i];
        if (saved < '0' || saved > '3' || (saved == '0' && equipment->isSpawned())) {
            return false; // Unknown state, or a first wave (spawned when a room is built) marked as waiting
        }
        origin->removeObject(equipment);
        if (saved != '0' && !equipment->isSpawned()) {
            equipment->spawn();
        }
        if (saved == '1') {
            origin->addObject(equipment);
        } else if (saved == '2' && !loaded_player->takeTool(equipment)) {
            return false;
        } else if (saved == '3') {
            equipment->consume();
        }
    }

    environment = std::move(loaded_environment);
    player = std::move(loaded_player);
    stats = loaded_stats;
    options.background = background;
    options.hardcore = hardcore == 1;
    current_room = room;
    dash_cooldown = dash;
    health_change = 0;
    state = GameState::Playing;
    ending = Ending::None;
    if (environment->isComplete()) {
        state = GameState::Completed;
        ending = stats.getCollapses() == 0 ? Ending::Unbroken : Ending::RisingAgain;
    }
    view = GameView::Room;
    last_tick = clock();
    return true;
}

bool Gauntlet::saveToFile(const std::string& path) const {
    std::ofstream out(path);
    return out && save(out) && out.flush();
}

bool Gauntlet::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    return in && load(in);
}
