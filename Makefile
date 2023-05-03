tetrashell: tetrashell.c tetris
	gcc tetrashell.c -lncurses -o ./tetrashell -g

tetris: tetris.c
	gcc tetris.c -lncurses -o ./tetris -g

