#include <iostream>

void Rules() {
        std::cout << "\n\n\t\t\t\t\t     ~Rules~\n\n\n";


        std::cout << "\t\t    Chess is a two-player board game using a chessboard.\n"
             << "\t\tOne of them play for White and another for Black.\n"
             << "\t\tWhite makes first move, than then players alternate moves.\n"
             << "\t\t    There are six pieces: Pawn, Knight, Rook, Bishop, Queen and King,\n\t\tthe main figure in game.\n"
             << "\t\t8 Pawns, 2 Knight, 2 Rook, 2 Bishop, 1 Queen and 1 King for each player.\n"
             << "\t\t    In our game we use lower case to mark White and upper for Black:\n"
             << "\t\tPawn - P, p; Knight - G, g; Rook - R, r; Bishop - B, b; Queen - Q, q; King - K, k.\n\n";

        std::cout << "\t\t\tThe chessboard and order of figures look like this:\n"
             << "\t\t\t\t\t-----------------\n"
             << "\t\t\t\t\t|R|G|B|Q|K|B|G|R|\n"
             << "\t\t\t\t\t|P|P|P|P|P|P|P|P|\n"
             << "\t\t\t\t\t|.|.|.|.|.|.|.|.|\n"
             << "\t\t\t\t\t|.|.|.|.|.|.|.|.|\n"
             << "\t\t\t\t\t|.|.|.|.|.|.|.|.|\n"
             << "\t\t\t\t\t|.|.|.|.|.|.|.|.|\n"
             << "\t\t\t\t\t|.|.|.|.|.|.|.|.|\n"
             << "\t\t\t\t\t|p|p|p|p|p|p|p|p|\n"
             << "\t\t\t\t\t|r|g|b|q|k|b|g|r|\n"
             << "\t\t\t\t\t-----------------\n\n";

        std::cout << "\t\t\t\t\tMoves of figures\n"
             << "\tFigures can move to a vacant squares and all beat opponent's figures, expect King.\n\n";

        std::cout << "\t\tQueen\t\t\t\t\t\tBishop\n"
             << "    1. moves any number of vacant squares        |     1. moves any number of vacant squares\n"
             << " horizontally, vertically, or diagonally.        | diagonally.\n\n";

        std::cout << "\t\tKnight\t\t\t\t\t\tRook\n"
             << "    1. moves to the nearest square not on the    |    1. moves any number of vacant squares\n"
             << " same color in an `L` pattern in all directions. | horizontally or vertically.\n"
             << "    2. jumps over figures to the new location,   |    2.is moved when castling.\n"
             << " unlike the rest ones.\n\n";

        std::cout << "\t\tPawn\t\t\t\t\t\tKing\n"
             << "    1. moves straight forward one square and     |    1. moves exactly one square horizontally,\n"
             << " two square from initial position.               | vertically, or diagonally.\n"
             << "    2. cannot move backwards.                    |    2. take part in castling.\n"
             << "    3. capture an enemy piece on either          |    3. can't move to a square, where can be\n"
             << " of the two squares diagonally in front of it.   | in check\n"
             << "    4. has special capture `En passant`.         |\n"
             << "    5. can be converted.                         |\n\n\n";

        std::cout << "\t\t\t\t\tSpecial moves and states\n\n";

        std::cout << "\t\t\t\t\t    Check\n"
             << "\t\t   A check is a condition when a player's king is under threat of\n"
             << "\t\tcapture on their opponent's next turn. A king so threatened is said\n"
             << "\t\tto be in check. You must get out of check, if possible, else the game\n"
             << "\t\tends in checkmate and the player loses. Players cannot make any move\n"
             << "\t\tthat puts their own king in check.\n";

        std::cout << "\t\t\t\t\t    Promotion\n"
             << "\t\t    It is a rule that allow a pawn that reaches a square on the back\n"
             << "\t\trank of the opponent be replaced by Bishop, Knight, Rook, or Queen\n"
             << "\t\tof the same color. Promotion to a queen is the most common -- 97% \n"
             << "\t\tplayers choose the queen, so in our game it is default\n\n";

        std::cout << "\t\t\t\t\t    En passant\n"
             << "\t\t   It is a special pawn capture that can only occur immediately after a\n"
             << "\t\tpawn makes double-step move from its starting square, and it could have\n"
             << "\t\tbeen captured by an enemy pawn had it advanced only one square.\n\n";

        std::cout << "\t\t\t\t\t    Castling\n"
             << "\t\t   It is a move in the game of chess involving a player's king and either\n"
             << "\t\tof the player's original rooks. It is the only move in chess in which a\n"
             << "\t\tplayer moves two pieces in the same move.\n";
    }
