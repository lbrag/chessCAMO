#include "chess.h"

Piece::~Piece() {}
Pawn::~Pawn() {}
Knight::~Knight() {}
Bishop::~Bishop() {}
Rook::~Rook() {}
Queen::~Queen() {}
King::~King() {}
Empty::~Empty() {}

Chess::~Chess()
{
	while(!board.empty())
	{
		board.pop_back(); // destroy the piece
	}
}

bool King::canCastle(int dest)
{
	int increment, src = this->getPieceSquare();

	vector<Piece*> board = chess.getBoard();
	stack<Piece*> stack = chess.getCheckStack();

	if(this->getPieceMoveInfo() || board[dest]->getPieceMoveInfo() || chess.getCheck())
	{
		return false;
	}
	else
	{
		increment = incrementChoice(src, dest);
	    for(auto elem : board)
	    {
	        if(!elem->isEmpty() && !elem->isSameColor(elem->getPieceSquare(), this->getPieceSquare()))
	        {
	            for(int i = src+increment; i<=dest; i+=increment)
	            {	
	                if(elem->isLegalMove(i))
	                {
	                	return false;
		            }
	            }
	        }
	    }

    	return this->isPathFree(src, dest);
	}
}

bool Piece::causeCheck(int dest)
{
	// causes a check if not a king (cannot check opponent with your king)
	if(this->isKing() || !chess.getCheckStack().empty())
	{
		return false;
	}

	stack<Piece*> CheckStack = chess.getCheckStack();
    vector<Piece*> board = chess.getBoard();

    int king_pos;
    for(auto elem : board)
    {
    	if(elem->isKing() && !isSameColor(elem->getPieceSquare(), dest))
    	{
    		king_pos = elem->getPieceSquare();
    		break;
    	}
    }

    if(isPathFree(dest, king_pos) && board[dest]->isLegalMove(king_pos))
    {
        CheckStack.push(board[king_pos]);
        CheckStack.push(board[dest]);
        chess.setCheckStack(CheckStack);
        chess.setCheck(true);
    }

    return chess.getCheck();
}

void Chess::isCheckmate()
{
	stack<Piece*> CheckStack = chess.getCheckStack();
	Piece *king, *piece;
	pieceColor current_turn = chess.getTurn() == 2 ? BLACK : WHITE;

	// checkmate -> checkStack is not empty (a player must be in check!) AND player turn is the player not in check (has pieces not king)
	if(CheckStack.top()->getPieceColor() == current_turn) 
	{
		handleCheckmate();
	}

	else // CheckStack.top()->getPieceColor() != current_turn
	{
		piece = CheckStack.top();
		CheckStack.pop();
		king = CheckStack.top();
		CheckStack.pop();

		// checkmate
		if(pieceIterator(piece->getPieceSquare(), king->getPieceSquare()))
		{
			handleCheckmate();
		}
		else
		{
			CheckStack.push(king);
			CheckStack.push(piece);
			chess.setCheckStack(CheckStack);
    		cout << "Check!" << endl; // was not checkmate so can state that it is check
		}
	}
}

void Chess::handleCheckmate()
{
	// printBoard(board);
	if(chess.getTurn() == 2)
		cout << "White won by Checkmate!" << endl;
	else
		cout << "Black won by Checkmate!" << endl;
	setCheckmate(true);
}

void Piece::enPassantHandling(int src, int dest)
{
	if(this->isPawn())
	{
		this->enPassantHandling(src, dest);
	}
	else
	{
		return;
	}
}

void Pawn::enPassantHandling(int src, int dest)
{
	vector<Piece*> board = chess.getBoard();
	if(abs(src-dest) == 16 && board[dest-1]->isPawn() && !isSameColor(dest-1, dest))
	{
		board[dest-1]->setEnPassant(true);
	}
	else if(abs(src-dest) == 16 && board[dest+1]->isPawn() && !isSameColor(dest+1, dest))
	{
		board[dest+1]->setEnPassant(true);
	}
	else
	{
		// if move was made after en-passant was set, cancel en-passant abilities of all pieces
		for(auto elem : board)
		{
			if(elem->isPawn())
			{
				elem->setEnPassant(false);
			}
		}
	}
}

bool Chess::pieceIterator(int src, int dest)
{
	// vector<Piece*> board = 
    int increment, original_dest = dest; // original_dest is the king
    int king_moves[8] = {-9, -8, -7, -1, 1, 7, 8, 9};

    // can a piece defend the king from check?
    increment = incrementChoice(src, dest);
    for(auto current_piece : chess.getBoard())
    {
        if(current_piece->isSameColor(current_piece->getPieceSquare(), original_dest) && current_piece->getPieceSquare() != original_dest)
        {
            for(int i = src+increment; i<=dest; i+=increment)
            {	
                if(current_piece->isLegalMove(i))
                {
                	return false;
                }
            }
        }
    }

    // can king move out of check?
    for(auto move : king_moves)
    {
    	if(original_dest+move >= 0 && original_dest + move <=63 && board[original_dest]->isLegalMove(original_dest + move) && !board[original_dest]->movedIntoCheck(original_dest + move))
    	{
    		return false;
    	}
    }
    	
    return true; // no legal moves found ? true : false
}

bool Piece::movedIntoCheck(int dest)
{
	if(this->isKing())
	{
		return this->movedIntoCheck(dest);
	}
	else
	{
		return false;
	}
}

bool King::movedIntoCheck(int dest)
{
	vector<Piece*> board = chess.getBoard();
    for(auto elem : board)
    {
        if(elem->getPieceColor() != NEUTRAL && !isSameColor(this->getPieceSquare(), elem->getPieceSquare()) && elem->isLegalMove(dest) && isPathFree(elem->getPieceSquare(), dest))
        {
            return true;
        }
    }

	return false;
}


bool Piece::canCastle(int dest)
{
	/* TODO
	    check for castling through a check
	*/
	vector<Piece*> board = chess.getBoard();
	if(this->isKing() && board[dest]->isRook() && this->getPieceSquare()/8 == dest/8)
    	return this->canCastle(dest);
    else // not king -> cannot castle
    	return false;
}

void Chess::makeMove(int src, int dest)
{
	vector<Piece*> board = chess.getBoard();
	stack<Piece*> CheckStack = chess.getCheckStack();
	pieceColor current_turn = chess.getTurn();
	Piece *king, *piece, *before_undo_piece; // "before_undo_piece" needed for attacking moves that are illegal (to restore the piece that goes missing)
	bool before_undo_moveInfo;

    if(0 <= dest && dest <= 63 && dest != src && board[src]->isLegalMove(dest) && board[src]->getPieceColor() == current_turn)
    {
        // pawn promotion
        if(board[src]->isPawn() && (dest/8 == 0 || dest/8 == 7))
        {
            board[src]->promotePawn(dest);
        }

		before_undo_piece = board[dest];
		before_undo_moveInfo = board[src]->getPieceMoveInfo();

        makeMoveForType(src, dest);

        if(!CheckStack.empty())
        {
        	piece = CheckStack.top();
			CheckStack.pop();
			king = CheckStack.top();
			CheckStack.pop();

			if(piece->isLegalMove(king->getPieceSquare()) && piece->isPathFree(piece->getPieceSquare(), king->getPieceSquare()))
			{
				makeMoveForType(dest, src); // undo the move
				board = chess.getBoard();
				board[dest] = before_undo_piece;
				board[src]->setPieceMoveInfo(before_undo_moveInfo);
				chess.setBoard(board);
				cout << "You are in check! Try again..." << endl;
				return;
			}
			else
			{
				chess.setCheckStack(CheckStack);
				chess.setCheck(false);
			}
        }

        board = chess.getBoard();
        board[dest]->enPassantHandling(src, dest);

    	if(board[src]->causeCheck(dest)) // did the move cause a check?
    	{
    		chess.isCheckmate(); // check for checkmates
    	}

		chess.setTurn(current_turn == 2 ? BLACK : WHITE);
		printBoard(chess.getBoard());
		if(!chess.getCheckmate())
		{
			cout << "___________________________________________________" << endl;
			if(chess.getTurn() == 2)
			{
				cout << "\nWhite's move" << endl;
			}
			else
			{
				cout << "\nBlack's move" << endl;
			}
		}
  	}
    else
    {
		cout << "Invalid move! Try again..." << endl;
	}
}

void Piece::promotePawn(int dest)
{
	if(this->isPawn())
	{
		this->promotePawn(dest);
	}
	else
	{
		return; // do nothing
	}
}

void Pawn::promotePawn(int dest)
{
	vector<Piece*> board = chess.getBoard();
	char piece;
	int src = this->getPieceSquare();
    while(true)
    {
        cout << "Which Piece: Q/q | R/r | B/b | N/n?";
        cin >> piece;
        if(piece == 'Q' || piece == 'q')
        {
            board[src] = chess.getTurn() == 2 ? new Queen(dest, 9, QUEEN, WHITE) : new Queen(dest, 9, QUEEN, BLACK);
            break;
        }
        if(piece == 'R' || piece == 'r')
        {
            board[src] = chess.getTurn() == 2 ? new Rook(dest, 5, ROOK, WHITE) : new Rook(dest, 5, ROOK, BLACK);
            break;
        }
        if(piece == 'B' || piece == 'b')
        {
            board[src] = chess.getTurn() == 2 ? new Bishop(dest, 3, BISHOP, WHITE) : new Bishop(dest, 3, BISHOP, BLACK);
            break;
        }
        if(piece == 'N' || piece == 'n')
        {
            board[src] = chess.getTurn() == 2 ? new Knight(dest, 3, KNIGHT, WHITE) : new Knight(dest, 3, KNIGHT, BLACK);
            break;
        }

        cout << "Please make sure to pick correct value!" << endl;
    }

    chess.setBoard(board);
}

void Chess::makeMoveForType(int src, int dest)
{
	vector<Piece*> board = chess.getBoard();

	// castling move
	if(board[src]->canCastle(dest) && (abs(src - dest) == 3 || abs(src - dest) == 4))
	{
	    // note that the pieces are moved
	    board[src]->setPieceMoveInfo(true);
	    board[dest]->setPieceMoveInfo(true);

	    if(abs(src - dest) == 3) // king side castle
	    {
            // king move
            board[src]->setPieceSquare(src + 2);
            board[src + 2]->setPieceSquare(src);
            std::swap(board[src], board[src + 2]);

            // rook move
            board[dest]->setPieceSquare(dest - 2);
            board[dest - 2]->setPieceSquare(dest);
            std::swap(board[dest - 2], board[dest]);
	    }

	    else // abs(src-dest) == 4 -> queen side castle
	    {
            // king move
            board[src]->setPieceSquare(src - 2);
            board[src - 2]->setPieceSquare(src);
            std::swap(board[src], board[src - 2]);

            // rook move
            board[dest]->setPieceSquare(dest + 3);
            board[dest + 3]->setPieceSquare(dest);
            std::swap(board[dest + 3], board[dest]);
	    }
	}

	// en-passant move
	else if(board[src]->isPawn() && board[src]->getEnPassant() && src % 8 != dest % 8)
	{
		board[src]->setPieceSquare(dest);
        board[dest]->setPieceSquare(src);
        std::swap(board[src], board[dest]);

        // delete the pawn that caused en-passant (make it empty square)
		if(abs(src-dest) == 7)
		{
			board[src+1] = new Empty(src+1, 0, EMPTY, NEUTRAL);
		}
		else if(abs(src-dest) == 9)
		{
			board[src-1] = new Empty(src-1, 0, EMPTY, NEUTRAL);
		}
	}

	// regular or attacking
	else
	{
	    // note that the piece moved
	    board[src]->setPieceMoveInfo(true);
	    // Regular Move
	    if(board[dest]->isEmpty())
	    {
            board[src]->setPieceSquare(dest);
            board[dest]->setPieceSquare(src);
            std::swap(board[src], board[dest]);
	    }

	    // Attacking Move
	    else
	    {
            // dest
            board[dest] = board[src];
            board[dest]->setPieceSquare(dest);

            // src
            board[src] = new Empty(src, 0, EMPTY, NEUTRAL);
            board[src]->setPieceSquare(src);
	    }
	}

	chess.setBoard(board);
}

bool Chess::isPathFree(int src, int dest)
{
    int increment = incrementChoice(src, dest);

    return increment > 0 ? pathIterator(src, dest, increment) : false;
}

int Chess::incrementChoice(int & src, int & dest)
{
    // make sure 'src' is always lower so can simply analyze one way (same whichever way you choose)
	if(src > dest)
    {
    	std::swap(src, dest);
    }

    if(src/8 == dest/8) // row path
    {
        return 1;
    }
    else if(src%8 == dest%8) // column path
    {
        return 8;
    }
    else if(abs(src/8 - dest/8) == abs(src%8 - dest%8)) // diagonal path
    {
        return abs(src - dest) % 7 == 0 ? 7 : 9;
    }
    else
    {
    	return 0;
    }
}

bool Chess::pathIterator(int src, int dest, int increment)
{
	vector<Piece*> board = chess.getBoard();
	for(int i = src+increment; i<dest; i+=increment)
    {
        if(!board[i]->isEmpty())
            return false;
    }
    return true;
}

bool Piece::isSameColor(int src, int dest)
{
	vector<Piece*> board = chess.getBoard();
	return board[src]->getPieceColor() == board[dest]->getPieceColor();
}

bool Piece::isLegalMove(int dest)
{
	if(this->isEmpty())
	{
		return false;
	}	
	else
	{
		return this->isLegalMove(dest);
	}	
}

bool Pawn::isLegalMove(int dest)
{
	vector<Piece*> board = chess.getBoard();

	bool legal = false;
	int src = this->getPieceSquare();

	// on attack, it can move sideways & first move can be 2 squares forward
    if(this->getPieceMoveInfo())
    {
        if(this->isPieceWhite()) // goes up
            legal = (src-dest == 8 && board[dest]->isEmpty()) || ((src-dest == 7 || src-dest == 9) && (!board[dest]->isEmpty() || getEnPassant()));
        else // black, goes down
            legal = (dest-src == 8 && board[dest]->isEmpty()) || ((dest-src == 7 || dest-src == 9) && (!board[dest]->isEmpty() || getEnPassant()));
    }
    else // cannot en-passant if you have not moved yet
    {
        if(this->isPieceWhite()) // goes up
            legal = ((src-dest == 8 || src-dest == 16) && board[dest]->isEmpty()) || ((src-dest == 7 || src-dest == 9) && !board[dest]->isEmpty());
        else // black, goes down
            legal = ((dest-src == 8 || dest-src == 16) && board[dest]->isEmpty()) || ((dest-src == 7 || dest-src == 9) && !board[dest]->isEmpty());
    }

    return legal && isPathFree(src, dest) && !isSameColor(src, dest);
}

bool Rook::isLegalMove(int dest)
{
	int src = this->getPieceSquare();
	int src_row = src/8, src_col = src%8, dest_row = dest/8, dest_col = dest%8;
	return (src_row == dest_row || src_col == dest_col) && isPathFree(src, dest) && !isSameColor(src, dest);
}

bool Knight::isLegalMove(int dest)
{
	vector<Piece*> board = chess.getBoard();

	int src = this->getPieceSquare();
	int src_row = src/8, src_col = src%8, dest_row = dest/8, dest_col = dest%8;
	int diff = abs(src - dest), diff_row = abs(src_row - dest_row), diff_col = abs(src_col - dest_col);
	return diff_row <= 2 && diff_col <= 2 && (diff == 6 || diff == 10 || diff == 15 || diff == 17) && !isSameColor(src, dest);
}

bool Bishop::isLegalMove(int dest)
{
	int src = this->getPieceSquare();
	int src_row = src/8, src_col = src%8, dest_row = dest/8, dest_col = dest%8;
	int diff = abs(src - dest), diff_row = abs(src_row - dest_row), diff_col = abs(src_col - dest_col);
	return (diff % 7 == 0 || diff % 9 == 0) && diff_row == diff_col && isPathFree(src, dest) && !isSameColor(src, dest);
}

bool Queen::isLegalMove(int dest)
{
	vector<Piece*> board = chess.getBoard();

	int src = this->getPieceSquare();
	int src_row = src/8, src_col = src%8, dest_row = dest/8, dest_col = dest%8;
	int diff = abs(src - dest), diff_row = abs(src_row - dest_row), diff_col = abs(src_col - dest_col);
	return (((diff % 7 == 0 || diff % 9 == 0) && diff_row == diff_col) || (src_row == dest_row || src_col == dest_col)) && isPathFree(src, dest) && !isSameColor(src, dest);
}

bool King::isLegalMove(int dest)
{
	vector<Piece*> board = chess.getBoard();

	int src = this->getPieceSquare();
	int diff = abs(src - dest);
	return (((diff == 1 || diff == 7 || diff == 8 || diff == 9) && !isSameColor(src, dest)) || ((diff == 3 || diff == 4) && this->canCastle(dest) && isSameColor(src, dest))) && !this->movedIntoCheck(dest);
}

// Board intialization
void boardInit(Chess & chess)
{
	int board_size = 64;
	vector<Piece*> board;

	// initialize the board
	for(int i = 0; i < board_size; i++)
	{
		if(i < board_size/4) // black
		{
			if(i == 0 || i == 7) // rook
			{
				board.push_back(new Rook(i, 5, ROOK, BLACK));
			}
			else if(i == 1 || i == 6) // knight
			{
				board.push_back(new Knight(i, 3, KNIGHT, BLACK)); 
			}
			else if(i == 2 || i == 5) // bishop
			{
				board.push_back(new Bishop(i, 3, BISHOP, BLACK)); 
			}
			else if(i == 3) // queen
			{
				board.push_back(new Queen(i, 9, QUEEN, BLACK));
			}
			else if(i == 4) // king
			{
				board.push_back(new King(i, 10, KING, BLACK));
			}
			else // pawn
			{
				board.push_back(new Pawn(i, 1, PAWN, BLACK));
			}
		}
		else if(board_size/4 <= i && i < board_size*3/4) // blank squares
		{	
			board.push_back(new Empty(i, 0, EMPTY, NEUTRAL));
		}
		else // white
		{
			if(i == 56 || i == 63) // rook
			{
				board.push_back(new Rook(i, 5, ROOK, WHITE));
			}
			else if(i == 57 || i == 62) // knight
			{
				board.push_back(new Knight(i, 3, KNIGHT, WHITE)); 
			}
			else if(i == 58 || i == 61) // bishop
			{
				board.push_back(new Bishop(i, 3, BISHOP, WHITE));
			}
			else if(i == 59) // queen
			{
				board.push_back(new Queen(i, 9, QUEEN, WHITE));
			}
			else if(i == 60) // king
			{
				board.push_back(new King(i, 10, KING, WHITE));
			}
			else // pawn
			{
				board.push_back(new Pawn(i, 1, PAWN, WHITE));
			}
		}
	}

	chess.setBoard(board);
	cout << "\nWhite to move first (as always)! \n" << endl;
	printBoard(board);
}

// Print the current board position
void printBoard(const vector<Piece*> & board)
{
	char piece_char;
	char ranks[8] = {'H', 'G', 'F', 'E', 'D', 'C', 'B', 'A'};
	
	int count = 0;
	for(auto elem : board)
	{
		if(count == 0)
		{
			cout << "  +---+---+---+---+---+---+---+---+" << endl;
			cout << ranks[count/8] << " | ";
		}
		else if(count % 8 == 0 && count > 0)
		{
			cout << "  +---+---+---+---+---+---+---+---+" << endl;
			cout << ranks[count/8] << " | ";
		}

		switch(elem->getPieceType())
		{
			case ROOK:
				piece_char = elem->isPieceWhite() ? 'R' : 'r';
				break;
			case KNIGHT:
				piece_char = elem->isPieceWhite() ? 'N' : 'n';
				break;
			case BISHOP:
				piece_char = elem->isPieceWhite() ? 'B' : 'b';
				break;
			case KING:
				piece_char = elem->isPieceWhite() ? 'K' : 'k';
				break;
			case QUEEN:
				piece_char = elem->isPieceWhite() ? 'Q' : 'q';
				break;
			case PAWN:
				piece_char = elem->isPieceWhite() ? 'P' : 'p';
				break;
			default:
				piece_char = ' ';
		}

		cout << std::left << std::setw(2) << piece_char;
		cout << "| ";

		// go to next row if reached last column
		if(count % 8 == 7)
			cout << endl;
		if(count == 63)
		{
			cout << "  +---+---+---+---+---+---+---+---+" << endl;
			cout << "    1   2   3   4   5   6   7   8" << endl;
		}
		count++;
	}
}
