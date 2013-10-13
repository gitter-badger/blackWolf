//describes a position
//can be constructed given another position and a move to make on it
//thus it is aware if move just made was castling, en passant or promotion
#ifndef POSITION_H
#define POSITION_H
#include <vector>
#include <array>
#include "../blackWolf.h"

class position{
public:

    position();

    position (int boardArray[8][8]);

    position(const position &givenPos, const Move &move);

    const Unit& operator()(const Square& square) const;


    bool whiteCastleQueen;
    bool whiteCastleKing;
    bool blackCastleQueen;
    bool blackCastleKing;

    bool wasCastle;
    bool wasEnPassant;
    bool wasPromotion;

    void setPromotion(const Square& square, const Unit& chosenPiece);

    Color getTurnColor() const;
    void setTurnColor(Color color);

private:
    Color turnColor;

    std::array<std::array<Unit, 8>, 8> cells;




    void init();


};

#endif // POSITION_H
