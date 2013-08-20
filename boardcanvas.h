//the board "view"
//a class built around sfg::Canvas

#ifndef BOARDCANVAS_H
#define BOARDCANVAS_H
#include <SFGUI/Canvas.hpp>
#include <boost/signals2.hpp>
#include "piecesbimap.h"
#include "resourcemanager.h"
#include "completemove.h"
#include "position.h"

class boardCanvas
{
private:
    //Canvas onto which board is drawn
    sfg::Canvas::Ptr window;

    //offset used in positioning the pieces to achieve flip
    int flipOffset;

    sf::Texture boardTexture_;
    sf::Sprite boardSprite_;
    sf::Window& bigWindow; //only to know its position

    resourceManager& resources;

    //bimap between board squares and piece sprites
    piecesBimap pieces;

    bool flipped() const;

    //grid of rects to check where a piece sprite is dropped
    std::vector<std::vector<sf::FloatRect> > rectGrid;

    //piece held by the mouse
    piecesBimap::iterator currentPiece;

    bool pieceHeld();
    void releasePiece();

    int idCount; //pieces ids for bimap's use

    void destroy(const int row, const int col); //will destroy the sprite in given location

    sf::Vector2f cellToPosition(const int row, const int col) const;

    sf::Vector2f getMousePosition() const; //mouse position in the canvas' coords

    void sendBack(); //sends the current piece back

    void flipBoard();

    //move was castle with destination the given square
    //so move rook to appropriate square
    void handleCastle(const int row, const int col);

    //move was en passant moving onto given square
    //so destroy the real pawn
    void handleEnPassant(const int row, const int col);

    //signals
    boost::signals2::signal<bool (int, int, int, int)> requestMove;

    //slots
    void slotMouseMove();
    void slotMouseRelease();
    void slotLeftClick();
    void slotEnterCanvas();

    void resetRects();

public:
    boardCanvas(sf::Window& theWindow, resourceManager &theResources);
    void display();
    void moveMake(const completeMove move);

    sfg::Widget::Ptr getBoardWidget() const;
    void setPosition(const position &givenPosition);

    boost::signals2::signal<bool (int, int, int, int)> &getSignal();
};

#endif // BOARDCANVAS_H
