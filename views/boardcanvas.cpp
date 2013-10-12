#include "boardcanvas.h"

const sf::Vector2f boardCanvas::offToCenter(25.f,25.f);

boardCanvas::boardCanvas(sf::Window& theWindow, resourceManager& theResources):
    flipOffset(0),
    window(sfg::Canvas::Create()),
    bigWindow(theWindow),
    resources(theResources),
    currentPiece(&pieces),
    idCount(1)
{
    boardSprite_.setTexture(resources.typeToTexture({Color::None, Piece::None}));

    piecesTexture.loadFromFile("Graphics/Pieces/ArrayBlack.png");

    particle.loadFromFile("Graphics/particle.png");
    system.reset(new thor::ParticleSystem());
    system->setTexture(particle);
    system->addAffector(FireworkAffector());

    rectGrid.resize(8);
    for (int i=0; i<8; ++i){
        rectGrid[i].resize(8);
        for (int j=0; j<8; ++j){
            sf::Vector2f toSet = cellToPosition(i,j);
            rectGrid[i][j].left = toSet.x;
            rectGrid[i][j].top = toSet.y;
            rectGrid[i][j].width = 50.f;
            rectGrid[i][j].height = 50.f;
        }
    }

    window->SetRequisition(sf::Vector2f( 440.f, 440.f ));
    window->GetSignal(sfg::Widget::OnMouseLeftPress).Connect(&boardCanvas::slotLeftClick, this);
    window->GetSignal(sfg::Widget::OnMouseMove).Connect(&boardCanvas::slotMouseMove, this);
    window->GetSignal(sfg::Widget::OnMouseLeftRelease).Connect(&boardCanvas::slotMouseRelease, this);
    window->GetSignal(sfg::Widget::OnMouseEnter).Connect(&boardCanvas::slotEnterCanvas, this);
}

bool boardCanvas::flipped() const
{
    return (flipOffset!=0);
}

sf::Vector2i boardCanvas::toGridPos(const sf::Vector2f &position) const
{
    return sf::Vector2i((position.x-20)/50, ((470 - position.y)/50) - 1);

//    return sf::Vector2i((7*flipOffset-position.y+20)/(2*(flipOffset-25)),
//                        (7*flipOffset+position.x-370)/(2*(flipOffset-25)));
}

bool boardCanvas::pieceHeld()
{
    return (currentPiece.isValid());
}

void boardCanvas::releasePiece()
{
    currentPiece.invalidate();
}

void boardCanvas::destroy(const int row, const int col)
{
    if (pieces[row][col].erase()) //if true then firework
        system->addEmitter(FireworkEmitter(cellToPosition(row,col) + offToCenter), sf::seconds(1.f));
}

void boardCanvas::display()
{
    system->update(frameClock.restart());

    window->Clear();

    window->Draw(boardSprite_);
    window->Draw(*system, sf::BlendAdd);

    vertexArray.clear();
    for (auto& piece : pieces)
        piece.appendQuadTo(vertexArray);

    window->Draw(vertexArray.data(), vertexArray.size(), sf::Quads, &piecesTexture);



    for (auto& arrow : arrows)
    {
        window->Draw(arrow, sf::BlendAdd);
    }

}

void boardCanvas::moveMake(const completeMove move)
{
    const int originRow = move.getRow1();
    const int originCol = move.getCol1();
    const int destRow = move.getRow2();
    const int destCol = move.getCol2();

    destroy(destRow,destCol); //destroy any sprites at destination
    pieces[originRow][originCol].moveTo(destRow, destCol, cellToPosition(destRow, destCol));

    position currentPosition = move.getNewBoard();
    if (currentPosition.wasCastle) handleCastle(destRow,destCol);
    if (currentPosition.wasEnPassant) handleEnPassant(destRow,destCol);
}

sfg::Widget::Ptr boardCanvas::getBoardWidget() const
{
    return window;
}

sf::Vector2f boardCanvas::cellToPosition(const int row, const int col) const
{
    return sf::Vector2f(flipOffset * (7 - 2*col) + 20 + 50 * col, -flipOffset * (7 - 2*row) + 420 - 50 * (row+1));
}

sf::Vector2f boardCanvas::getMousePosition() const
{
    return (static_cast<sf::Vector2f>(sf::Mouse::getPosition(bigWindow)) - window->GetAbsolutePosition());
}

void boardCanvas::sendBack()
{
    BOOST_ASSERT_MSG(pieceHeld(), "No current piece to send back");

    pieces[currentPiece].sendTo(cellToPosition(currentPiece.getRow(),currentPiece.getCol()));

    releasePiece();
}

void boardCanvas::flipBoard()
{
    if (flipped()) flipOffset = 0;
    else flipOffset = 50;

    for (auto &piece : pieces){
        auto toFlip = pieces[piece];
        toFlip.sendTo(cellToPosition(toFlip.getRow(), toFlip.getCol()));
    }

    resetRects();
}

void boardCanvas::reload(const position &givenPosition)
{
    pieces.clear();
    setPosition(givenPosition);
    boardSprite_.setTexture(resources.typeToTexture({Color::None, Piece::None}));
}

void boardCanvas::setResult(Color result)
{
    if (result == Color::White){
        for (auto& piece : pieces){
            if (piece.getColor() == Color::Black)
            {
                system->addEmitter(FireworkEmitter(piece.getPosition() + offToCenter), sf::seconds(1.f));
                //pieces.erase(piece);
            }


        }
    }else if (result == Color::Black)
    {
        for (auto& piece : pieces){
            if (piece.getColor() == Color::White)
            {
                system->addEmitter(FireworkEmitter(piece.getPosition() + offToCenter), sf::seconds(1.f));
                //pieces.erase(piece);
            }
        }
    }
}

void boardCanvas::setArrow(int row1, int col1, int row2, int col2)
{
    sf::Vector2f point1 = cellToPosition(row1,col1) + offToCenter;
    sf::Vector2f point2 = cellToPosition(row2,col2) + offToCenter;

    arrows.emplace_back(point1,point2-point1,sf::Color(0,100,0,125), 5.f);
}

void boardCanvas::clearArrows()
{
    arrows.clear();
}

void boardCanvas::handleCastle(const int row, const int col)
{
    if (row==0){
        if (col==2){
            pieces[0][0].moveTo(0,3,cellToPosition(0,3));
        }else{
            BOOST_ASSERT_MSG(col==6, "Invalid Castle");
            pieces[0][7].moveTo(0,5,cellToPosition(0,5));
        }
    }else{
        if (col==2){
            BOOST_ASSERT_MSG(row==7, "Invalid Castle");
            pieces[7][0].moveTo(7,3,cellToPosition(7,3));
        }else{
            BOOST_ASSERT_MSG(col==6, "Invalid Castle");
            pieces[7][7].moveTo(7,5,cellToPosition(7,5));
        }
    }
}

void boardCanvas::handleEnPassant(const int row, const int col)
{
    if (row==5){
        destroy(4,col);
    }else{
        BOOST_ASSERT_MSG(row==2, "Invalid en passant");
        destroy(3,col);
    }
}

void boardCanvas::slotLeftClick()
{
    //if (gameEnded) return; - it should be disabled from outside
    //if (humanColor != getTurnColor()) return; - boardMaster will decide that

    sf::Vector2f clickedPoint = getMousePosition();

    //const int whoseTurn = getTurnColor();

    for (auto &piece : pieces){
        //if (piece.getSide()!=whoseTurn) continue;
        if (piece.contains(clickedPoint)){
            currentPiece = pieces.spriteToIt(piece);
            break;
        }
    }
}

void boardCanvas::slotMouseMove()
{
    if (pieceHeld()){
        pieces[currentPiece].sendTo(getMousePosition()-offToCenter);
    }
}

void boardCanvas::slotMouseRelease()
{
    if (pieceHeld()){
        sf::Vector2f centrePos = pieces[currentPiece].getPosition() + offToCenter;
//        for (int i=0; i<8; ++i){
//            for (int j=0; j<8; ++j){
//                if (rectGrid[i][j].contains(centrePos)){
                    const int originRow = currentPiece.getRow();
                    const int originCol = currentPiece.getCol();
                    sf::Vector2i gridPos = toGridPos(centrePos);
                    //send request move signal to controller
                    std::cout << gridPos.x << "," << gridPos.y << std::endl;
                    if (!(*requestMove(originRow, originCol, gridPos.y, gridPos.x)))
                        sendBack(); //if rejected send piece back
                    return;
//                }
//            }
       // }
        sendBack();
    }
}

void boardCanvas::slotEnterCanvas()
{
    if (pieceHeld()){
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) sendBack();
    }
}

void boardCanvas::setPosition(const position& givenPosition)
{
    for (int i=0; i<8; ++i){
        for (int j=0; j<8; ++j){
            const Unit pieceId = givenPosition(i, j);
            if ((pieceId.piece == Piece::None)||(pieceId.piece == Piece::Shadow)) continue;
            pieceSprite toAdd(cellToPosition(i,j),pieceId, idCount++);
            pieces[i][j].insert(toAdd);            
        }
    }

}

void boardCanvas::resetFor(Color whoFaceUp)
{
    if (flipped()){
        if (whoFaceUp == Color::White) flipBoard();
    }else{
        if (whoFaceUp != Color::White) flipBoard();
    }

    resetRects();

    idCount = 1;
    pieces.clear();
}

boost::signals2::signal<bool (int, int, int, int)>& boardCanvas::getSignal()
{
    return requestMove;
}

void boardCanvas::setPromotion(int row, int col, Piece piece)
{
    const Color whichSide = pieces[row][col].getColor();
    destroy(row,col);
    pieceSprite toAdd(cellToPosition(row,col),{whichSide, piece},idCount++);
    pieces[row][col].insert(toAdd);
}

void boardCanvas::resetRects()
{
    for (int i=0; i<8; ++i){
        for (int j=0; j<8; ++j){
            sf::Vector2f toSet = cellToPosition(i,j);
            rectGrid[i][j].left = toSet.x;
            rectGrid[i][j].top = toSet.y;
        }
    }
}
