## A chess-aware widget
* gtkmm chess widget (C++)
* GTK+ widget (C)
* High performance, highly scalable vectorized pieces.
* Per square colorable (for analysis applications).
* Support for drawing arrows.
* Move indicator.

## Chess board classes (move generation, C++)
* High performance.
* Full control over the chessboard and pieces.
* Ability to run over any combination of pieces on the board.
* Ability to generate all legal (and only legal) moves for a given piece.
* Possibility to check if a given move is legal.
* Full support for FEN codes.
* Allows to query whether there is a check, or double check.
* Keeps track of the number of times each color attacks and/or defends any given square.

Comes with an example program that uses the widgets
and allows one to set up pieces, move pieces and
check for legal moves (by highlighting squares where
a piece can go to, which squares it defends, or which
squares it attacks).

For more information please visit http://carlowood.github.io/cwchessboard/index.html
