#include "chess.h"

Chess chess; // global object call
bool valid_test = true; // global variable call

int main()
{
	// 'src' -> start square index, 'dest' -> end square index, 'turn' -> 1 (white) or -1 (black)
	int src, dest;

	// Create 8x8 default board
	boardInit(chess);
	vector<Piece*> board = chess.getBoard();

	// userEnded(turn);

    while(!chess.getCheckmate()) //while the end of file is NOT reached or game is not finished
    {	
    	cout << endl << "Enter a source AND destination square in [0, 63]: ";
        cin >> src >> dest;
        chess.makeMove(src, dest);
    }

	return 0;
}

/* TODO: 
	1. Avoid castling:
		- into check (king is in check after castling)
		- through check
	2. En-passant
	3. Double Checks, stalemate, knight check
	4. Pinned Piece
	5. Three move repetition (draw) & 50 move rule
	6. Draw offer / Resign
*/

		