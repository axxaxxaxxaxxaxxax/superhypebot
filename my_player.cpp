#include "my_player.h"
#include <cstdlib>
#include <iostream>




static field_index_t rand_int(field_index_t min, field_index_t max) {
    return min + rand() % (max - min + 1);
}

Point RandomPlayer::play(const GameView& game) {
    Boundary b = game.get_settings().field_size;
    Point result;
    do {
        result = {
            .x = rand_int(b.min.x, b.max.x),
            .y = rand_int(b.min.y, b.max.y),
        };
    } while(game.get_state().field->get_value(result) != Mark::None);
    return result;
}

void BasicObserver::notify(const GameView&, const Event& event) {
    if (event.get_type() == MoveEvent::TYPE) {
        auto &data = get_data<MoveEvent>(event);
        _out << "Move:\tx = " <<  data.point.x 
            << ",\ty = " << data.point.y << ":\t";
        _print_mark(data.mark) << '\n';
        return;
    }
    if (event.get_type() == PlayerJoinedEvent::TYPE) {
        auto &data = get_data<PlayerJoinedEvent>(event);
        _out << "Player '" << data.name << "' joined as ";
        _print_mark(data.mark) << '\n';
        return;
    }
    if (event.get_type() == GameStartedEvent::TYPE) {
        _out << "Game started\n";
        return;
    }
    if (event.get_type() == WinEvent::TYPE) {
        auto &data = get_data<WinEvent>(event);
        _out << "Player playing ";
        _print_mark(data.winner_mark) << " has won\n";
        return;
    }
    if (event.get_type() == DrawEvent::TYPE) {
        auto &data = get_data<DrawEvent>(event);
        _out << "Draw happened, reason: '" << data.reason << "'\n";
        return;
    }
    if (event.get_type() == DisqualificationEvent::TYPE) {
        auto &data = get_data<DisqualificationEvent>(event);
        _out << "Player playing ";
        _print_mark(data.player_mark) << " was disqualified, reason: '" 
            << data.reason << "'\n";
        return;
    }
}

std::ostream& BasicObserver::_print_mark(Mark m) {
    if (m == Mark::Cross) return _out << "X";
    if (m == Mark::Zero) return _out << "O";
    return _out << "?";
}







superhypebot::superhypebot(const std::string& Name) : name(Name) {
    assign_mark(Mark::None);
}

std::string superhypebot::get_name() const {
    return name;
}

void superhypebot::assign_mark(Mark player_mark) {
    mark = player_mark;
}

Point superhypebot::play(const GameView& game) {
    const GameState& state = game.get_state();
    const FixedSizeField& field = static_cast<const FixedSizeField&>(*state.field);

    const Boundary fieldBoundary = state.field->get_current_boundary();
    const size_t fieldWidth = fieldBoundary.get_width();
    const size_t fieldHeight = fieldBoundary.get_height();

    Point bestMove(0, 0);

    bool isFieldEmpty = true;
    auto iterator = field.get_iterator();
    while (iterator->has_value()) {
        if (iterator->get_value() != Mark::None) {
            isFieldEmpty = false;
            break;
        }
        iterator->step();
    }

    if (isFieldEmpty) {
        size_t middle = fieldWidth / 2;
        return Point(middle, middle);
    }

    int bestScore = std::numeric_limits<int>::min();
    Mark playerMark = mark;
    Mark opponentMark = (playerMark == Mark::Cross) ? Mark::Zero : Mark::Cross;

    for (size_t i = 0; i < fieldHeight; ++i) {
        for (size_t j = 0; j < fieldWidth; ++j) {
            Point currentPos(j, i);

            if (field.get_value(currentPos) == Mark::None) {
                
                if (!isNearOccupied(field, currentPos)) {
                    continue;
                }

                if (checkWinningMove(field, currentPos, playerMark)) {
                    return currentPos;
                }

                if (checkWinningMove(field, currentPos, opponentMark)) {
                    return currentPos;
                }

                if (isForkMove(field, currentPos, playerMark)) {
                    return currentPos;
                }

                if (canBlockOpponentFork(field, currentPos, opponentMark)) {
                    return currentPos;
                }

                if (shouldBlockThreeInRow(field, currentPos, opponentMark)) {
                    return currentPos;
                }

                if (shouldBlockFourInRow(field, currentPos, opponentMark)) {
                    return currentPos;
                }

                FixedSizeField tempField = field;
                tempField.set_value(currentPos, playerMark);
                int score = evaluatePosition(tempField, playerMark, fieldWidth, fieldHeight);
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = currentPos;
                }
            }
        }
    }
    return bestMove;
}

void superhypebot::notify(const GameView& game, const Event& event) {}


bool superhypebot::isNearOccupied(const FixedSizeField& field, const Point& p) const {
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dy = -2; dy <= 2; ++dy) {
            if (dx == 0 && dy == 0) continue;
            Point neighbor(p.x + dx, p.y + dy);
            if (neighbor.x >= 0 && neighbor.x < field.get_current_boundary().get_width() &&
                neighbor.y >= 0 && neighbor.y < field.get_current_boundary().get_height()) {
                if (field.get_value(neighbor) != Mark::None) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool superhypebot::checkWinningMove(const FixedSizeField& field, const Point& move, Mark playerMark) const {
    return evaluateLine(field, move, Point(1, 0), playerMark) >= 3 ||
        evaluateLine(field, move, Point(0, 1), playerMark) >= 3 ||
        evaluateLine(field, move, Point(1, 1), playerMark) >= 3 ||
        evaluateLine(field, move, Point(1, -1), playerMark) >= 3;
}


bool superhypebot::isForkMove(const FixedSizeField& field, const Point& move, Mark playerMark) const {
    int forks = 0;

    // Check all directions
    forks += evaluateLine(field, move, Point(1, 0), playerMark) >= 2 ? 1 : 0;
    forks += evaluateLine(field, move, Point(0, 1), playerMark) >= 2 ? 1 : 0;
    forks += evaluateLine(field, move, Point(1, 1), playerMark) >= 2 ? 1 : 0;
    forks += evaluateLine(field, move, Point(1, -1), playerMark) >= 2 ? 1 : 0;

    return forks >= 2;
}

bool superhypebot::canBlockOpponentFork(const FixedSizeField& field, const Point& move, Mark opponentMark) const {
    return isForkMove(field, move, opponentMark);
}


bool superhypebot::shouldBlockThreeInRow(const FixedSizeField& field, const Point& move, Mark opponentMark) const {
    return (
        evaluateLine(field, move, Point(1, 0), opponentMark) == 3 || 
        evaluateLine(field, move, Point(0, 1), opponentMark) == 3 || 
        evaluateLine(field, move, Point(1, 1), opponentMark) == 3 || 
        evaluateLine(field, move, Point(1, -1), opponentMark) == 3 );
}

bool superhypebot::shouldBlockFourInRow(const FixedSizeField& field, const Point& move, Mark opponentMark) const {
    return (
        evaluateLine(field, move, Point(1, 0), opponentMark) == 4 || 
        evaluateLine(field, move, Point(0, 1), opponentMark) == 4 || 
        evaluateLine(field, move, Point(1, 1), opponentMark) == 4 || 
        evaluateLine(field, move, Point(1, -1), opponentMark) == 4 
            );
}


int superhypebot::evaluatePosition(const Field& field, Mark playerMark, size_t fieldWidth, size_t fieldHeight) const {
    int score = 0;

    for (size_t i = 0; i < fieldHeight; ++i) {
        for (size_t j = 0; j < fieldWidth; ++j) {
            Point currentPos(j, i);

            if (field.get_value(currentPos) == playerMark) {
                score += evaluateLine(field, currentPos, Point(1, 0), playerMark); 
                score += evaluateLine(field, currentPos, Point(0, 1), playerMark); 
                score += evaluateLine(field, currentPos, Point(1, 1), playerMark); 
                score += evaluateLine(field, currentPos, Point(1, -1), playerMark); 

                if (isForkMove(static_cast<const FixedSizeField&>(field), currentPos, playerMark)) {
                    score += 4; 
                }
            }
        }
    }

    return score;
}


int superhypebot::evaluateLine(const Field& field, const Point& startPoint, const Point& direction, Mark playerMark) const {
    int consecutiveMarks = 0;
    Point currentPoint = startPoint;

    while (currentPoint.x >= 0 && currentPoint.x < field.get_current_boundary().get_width() &&
        currentPoint.y >= 0 && currentPoint.y < field.get_current_boundary().get_height()) {
        if (field.get_value(currentPoint) == playerMark) {
            consecutiveMarks++;
        }
        else {
            break;
        }

        currentPoint.x += direction.x;
        currentPoint.y += direction.y;
    }

    return consecutiveMarks;
}

