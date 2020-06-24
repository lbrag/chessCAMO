#include <SFML/Graphics.hpp>
#include <vector>
#include "chess.h"

using namespace sf;
using namespace std;

void drawPieces(vector<Sprite> & pieces, const vector<Sprite> & pieceType, const vector<Piece*> & board)
{
    FloatRect shape;
    int i = 0;
    for(const auto & elem : board)
    {
        if(elem->isRook())
            pieces[i] = elem->isPieceWhite() ? pieceType[1] : pieceType[7];
        else if(elem->isKnight())
            pieces[i] = elem->isPieceWhite() ? pieceType[2] : pieceType[8];
        else if(elem->isBishop())
            pieces[i] = elem->isPieceWhite() ? pieceType[3] : pieceType[9];
        else if(elem->isQueen())
            pieces[i] = elem->isPieceWhite() ? pieceType[4] : pieceType[10];
        else if(elem->isKing())
            pieces[i] = elem->isPieceWhite() ? pieceType[5] : pieceType[11];
        else if(elem->isPawn())
            pieces[i] = elem->isPieceWhite() ? pieceType[6] : pieceType[12];
        else
            pieces[i] = pieceType[0];

        shape = pieces[i].getGlobalBounds();
        pieces[i].setPosition(shape.width + shape.width*(i%8), shape.height + shape.height*(i/8));
        i++;
    }

}

void storeOrRestore(vector<Piece*> & board, vector<int> & squares_prior, vector<bool> & moved_prior, vector<bool> & enpassant_prior, string type)
{
    int i = 0;
    for(auto & elem : board)
    {
        if(type == "restore")
        {   
            elem->setPieceSquare(squares_prior[i]);
            elem->setPieceMoveInfo(moved_prior[i]);
            elem->setEnPassant(enpassant_prior[i]);
        }
        else // type == "store"
        {
            squares_prior[i] = elem->getPieceSquare();
            moved_prior[i] = elem->getPieceMoveInfo();
            enpassant_prior[i] = elem->getEnPassant();
        }

        i++;
    }
}  

int main()
{
    // cout.setstate(std::ios_base::failbit); // surpress output

    Chess *chess = new Chess;

    // Create 8x8 default board
    chess->boardInit();

    stack<vector<Piece*>> board_positions;
    vector<int> squares_prior(64);
    vector<bool> moved_prior(64), enpassant_prior(64);

    board_positions.push(chess->getBoard());

    RenderWindow window(VideoMode(600,600), "chessCAMO");

    // pieces
    Texture blank, w1, w2, w3, w4, w5, w6, b1, b2, b3, b4, b5, b6;
    blank.loadFromFile("images/blank.png");   
    w1.loadFromFile("images/wr.png");
    w2.loadFromFile("images/wn.png");
    w3.loadFromFile("images/wb.png");
    w4.loadFromFile("images/wq.png");
    w5.loadFromFile("images/wk.png");
    w6.loadFromFile("images/wp.png");
    b1.loadFromFile("images/br.png");
    b2.loadFromFile("images/bn.png");
    b3.loadFromFile("images/bb.png");
    b4.loadFromFile("images/bq.png");
    b5.loadFromFile("images/bk.png");
    b6.loadFromFile("images/bp.png");

    Sprite sBlank(blank), sWr(w1), sWn(w2), sWb(w3), sWq(w4), sWk(w5), sWp(w6), 
                          sBr(b1), sBn(b2), sBb(b3), sBq(b4), sBk(b5), sBp(b6);
    
    vector<Sprite> pieceType = {sBlank, sWr, sWn, sWb, sWq, sWk, sWp, sBr, sBn, sBb, sBq, sBk, sBp};
    vector<Sprite> pieces(64); 

    drawPieces(pieces, pieceType, chess->getBoard());

    Vector2f size_rect(60.0, 60.0);
    Vector2f lr_rect(size_rect.x/2, size_rect.y);
    Vector2f tb_rect(size_rect.x, size_rect.y/2);
    RectangleShape rect(size_rect);
    Text text;
    Font font;
    const int numRows = 10; 
    const int numCols = 10; 
    const int distance = 60; //distance between squuares
    Color color_dark(211, 211, 211, 255); 
    Color color_yellow(255, 255, 153, 255); 
    Color color_orange(255, 204, 153, 255); 
    Color color_red(255, 204, 204, 255); 
    bool clicked = false;

    while(window.isOpen())
    {   
        Vector2i pos = Mouse::getPosition(window);
        int src, dest;
        vector<Sprite>::const_iterator iter;

        Event e;
        while (window.pollEvent(e))
        {
            if(e.type == Event::Closed)
                window.close();   

            if(e.type == sf::Event::KeyPressed)
            {
                if(e.key.code == sf::Keyboard::Space && board_positions.size() > 1)
                {
                    board_positions.pop();
                    chess->setTurn(chess->getTurn() == WHITE ? BLACK : WHITE);
                    chess->setBoard(board_positions.top());

                    storeOrRestore(board_positions.top(), squares_prior, moved_prior, enpassant_prior, "restore");
                    drawPieces(pieces, pieceType, chess->getBoard());
                }
            }

            if(e.type == Event::MouseButtonPressed)
            {
                if(e.mouseButton.button == Mouse::Left)
                {
                    clicked = true;
                    for(iter = pieces.begin(); iter != pieces.end(); iter++)
                    {
                        if(iter->getGlobalBounds().contains(pos.x, pos.y))
                        {
                            src = iter - pieces.begin();
                        }
                    }
                }
            }
                
            if(e.type == Event::MouseButtonReleased)
            {
                if(e.mouseButton.button == Mouse::Left)
                {
                    clicked = false;
                    dest = int((pos.x / size_rect.x) - 1) +  8*int((pos.y / size_rect.y) - 1) ;
                }

                // here means that you pressed and released the mousebutton,
                // so can make a move
                storeOrRestore(board_positions.top(), squares_prior, moved_prior, enpassant_prior, "store");
                chess->makeMove(src, dest, cin);

                drawPieces(pieces, pieceType, chess->getBoard());

                if(chess->getBoard() != board_positions.top())
                    board_positions.push(chess->getBoard());         
            }   

            if(clicked)
                pieces[src].setPosition( pos.x - size_rect.x/2, pos.y - size_rect.y/2 ); 
        }

        window.clear();

        // board
        if(font.loadFromFile("font/arial.ttf"))
        {
            text.setCharacterSize(30);
            text.setFont(font);
            text.setFillColor(Color::Black);

            string piece_file[8] = {"A", "B", "C", "D", "E", "F", "G", "H"};
            string piece_rank[8] = {"1", "2", "3", "4", "5", "6", "7", "8"};

            for(int i=0; i < numRows; i++)
            {
                for(int j=0; j < numCols; j++)
                {
                    // top and bottom rows
                    if((i == 0 || i == 9) && (1 <= j && j <= 8))
                    {
                        i == 0 ? text.setPosition(j*distance + size_rect.x/2, size_rect.y/2) : text.setPosition(j*distance + size_rect.x/2, size_rect.y*(numRows-1));
                        text.setString(piece_file[j-1]);
                        if(i == 0)
                            rect.setFillColor((i+j)%2 != 0 ? color_yellow : color_orange);
                        else
                            rect.setFillColor((i+j)%2 == 0 ? color_yellow : color_orange);
                    }

                    // left and right columns
                    else if((j == 0 || j == 9) && (1 <= i && i <= 8))
                    {
                        j == 0 ? text.setPosition(size_rect.x/2, i*distance + size_rect.y/2) : text.setPosition(size_rect.x*(numCols-1), i*distance + size_rect.y/2);
                        text.setString(piece_rank[i-1]);
                        if(j == 0)
                            rect.setFillColor((i+j)%2 != 0 ? color_yellow : color_orange);
                        else
                            rect.setFillColor((i+j)%2 == 0 ? color_yellow : color_orange);
                    }

                    // corners
                    else if((i == 0 && j == 0) || (i == 9 && j == 0) || (i == 0 && j == 9) || (i == 9 && j == 9))
                    {
                        rect.setFillColor(color_red);
                    }

                    // middle squares
                    else
                    {
                        rect.setFillColor((i+j)%2 != 0 ? color_dark : Color::White);
                    }

                    rect.setPosition(j*distance, i*distance);
                    window.draw(rect);  
                    window.draw(text);
                }
            } 
        }
        else
        {
            cout << "failed to load font file" << endl;
        }
        

        // pieces
        for(const auto & piece : pieces)
            window.draw(piece);

        window.display();
    }

    delete chess; // destroy the dynamic object to free its memory allocation
    cout.clear();

    return 0; 
}
