#pragma once
#include <ostream>
#include "player.h"

#include "basic_fields.h"

/**
 * This is example player, that plays absolutely randomly.
 */
class RandomPlayer : public Player {
    std::string _name;
public:
    RandomPlayer(const std::string& name): _name(name) {}
    std::string get_name() const override { return _name; }
    Point play(const GameView& game) override;
    void assign_mark(Mark player_mark) override { /*does nothing*/ }
    void notify(const GameView&, const Event&) override { /*does nothing*/ }
};


/**
 * Simple observer, that logs every event in given output stream.
 */
class BasicObserver : public Observer {
    std::ostream& _out;

    std::ostream& _print_mark(Mark mark);
public:
    BasicObserver(std::ostream& out_stream): _out(out_stream) {}
    void notify(const GameView&, const Event& event) override;
};





class superhypebot : public Player {
private:
    Mark mark;
    std::string name;

public:
    superhypebot(const std::string& Name);

    std::string get_name() const override;

    void assign_mark(Mark player_mark) override;

    Point play(const GameView& game) override;

    bool isNearOccupied(const FixedSizeField& field, const Point& pos) const;

    bool checkWinningMove(const FixedSizeField& field, const Point& move, Mark playerMark) const;

    void notify(const GameView& game, const Event& event) override;

    bool shouldBlockThreeInRow(const FixedSizeField& field, const Point& move, Mark opponentMark) const;

    bool shouldBlockFourInRow(const FixedSizeField& field, const Point& move, Mark opponentMark) const;

    bool isForkMove(const FixedSizeField& field, const Point& move, Mark playerMark) const;

    

private:
    bool canBlockOpponentFork(const FixedSizeField& field, const Point& move, Mark opponentMark) const;

    int evaluatePosition(const Field& field, Mark playerMark, size_t fieldWidth, size_t fieldHeight) const;

    int evaluateLine(const Field& field, const Point& startPoint, const Point& direction, Mark playerMark) const;

};


