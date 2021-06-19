/* Klassikspiel snake v1.4
 * Eine Schlange zum Essen steuern
 * ohne das diese sich nach der zweiten Mahlzeit in den Schwanz beisst
 *
 * unter Mithilfe von https://www.c-plusplus.net/forum/
 * nach einer Vorlage von https://www.c-howto.de/
 *
 * @author: El Europ alias momefilo
 * Lizenz: GPL
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
//#include <string.h>
//#include <unistd.h>
#include <curses.h>
#include <momefilo_tools.h>

/*Tasten zur Steuerung */
#define MYKEY_UP	'w'
#define MYKEY_DOWN	's'
#define MYKEY_LEFT	'd'
#define MYKEY_RIGHT	'a'

/* Figuren auf dem Spielfeld */
#define BLANK		0 /* Leeres Feld */
#define HEAD		1 /* Kopf der Schlange */
#define TAIL		2 /* Schwanz der Schlange */
#define FOOD		3 /* Essen fuer die Schlange damit der Schwanz waechst */

/* Spielevariablen */
int autoMove	= 0;	/* Beweggung ohne Tastendruck*/
int xoffset		= 0;	/* Zur Spielfeldzentrierung */
int yoffset		= 0;	/* Dito */
int tailcount	= 0;	/* Laenge des Schwanzes */
int points		= 0;	/* Erreichte Punkte */
int width; 				/* Spielfelddimension */
int height; 			/* Dito */
char richtung; 			/* Die aktuelle Richtung der Schlange */

/* Position des HEAD */
struct position{
	int x;
	int y;
} pos; /* Position des HEAD */

/* Initialisiert das Spielfeldarray
 * Dank an @swordfish */
void initalFeld(int **feld, int wert){
	int x;
	int y;
	for(x=0; x<width; x++){
		for(y=0; y<height; y++){
			*(*(feld + x) + y) = wert;
		}
	}
}

/* Initialisiert das Schwanzarray */
void initalTail(struct position *tail){
	int cnt=0;
	struct position myPos;
	myPos.x=-1;
	myPos.y=-1;
	for(cnt=0; cnt <= width * height; cnt++){
		*(tail + cnt) = myPos;
	}
}

/* Setzt pos.x und pos.y Abhängig von der richtung */
void myMove(){
	switch(richtung){
		case MYKEY_DOWN:
			if(pos.y >= height -1) pos.y = 0;
			else ++pos.y;
			break;
		case MYKEY_UP:
			if(pos.y <= 0) pos.y = height -1;
			else --pos.y;
			break;
		case MYKEY_LEFT:
			if(pos.x >= width -1) pos.x = 0;
			else ++pos.x;
			break;
		case MYKEY_RIGHT:
			if(pos.x <= 0) pos.x = width -1;
			else --pos.x;
	}
}

/* Zeichnet den Spielfeldrand und Ueberschrift*/
void printFeld(){
	int x; /* Schleifenzaehler */

	/* Schriftzug mittig ueber Spielfeld */
	int xoff = (width - 10) / 2;
	if(xoff < 0) xoff = 0;

	/* Schriftstil fett */
	attrset(A_BOLD);

	/* Ueberschrift zeichnen */
	mvprintw(0+yoffset, xoff+xoffset, "Snake v%d.%d", 1, 4);
	mvprintw(1+yoffset, xoff+xoffset, "(%d x %d)", width, height);

	/* Ecken Zeichnen */
	mvaddch(yoffset + 3, xoffset, ACS_ULCORNER);
	mvaddch(yoffset + 3, xoffset + width+1, ACS_URCORNER);
	mvaddch(yoffset+height + 4, xoffset, ACS_LLCORNER);
	mvaddch(yoffset+height + 4, xoffset+width + 1, ACS_LRCORNER);

	/* Linke und rechte Kante zeichnen */
	for(x=4; x<=height +3; x++){
		mvaddch(yoffset+x, xoffset, ACS_VLINE);
		mvaddch(yoffset+x, xoffset+width + 1, ACS_VLINE);
	}

	/* Obere und untere Kante zeichnen */
	for(x=1; x<=width; x++){
		mvaddch(yoffset+3, xoffset+x, ACS_HLINE);
		mvaddch(yoffset+height + 4, xoffset+x, ACS_HLINE);
	}
}

/* Setzt FOOD zufaellig aufs Spielfeld */
void setFood(int **feld){
	int x;
	int y;
	init_pair(1, COLOR_YELLOW, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);
	do{
		x=rand() % width;
		y=rand() % height;
	}while(*(*(feld + x) + y) != BLANK);
	feld[x][y] = FOOD; 									/* Spielfeld aktualisieren */
	attrset(A_BOLD|COLOR_PAIR(2));						/* Farbe blau fuer FOOD */
	mvaddch(yoffset+y + 4, xoffset+x + 1, ACS_DIAMOND);	/*Figur FOOD zeichnen */
	attrset(A_BOLD|COLOR_PAIR(1));						/* Farbe wieder gelb */
}

/* Zum Start des Spiels. Zeichnet spielfeld neu und setzt HEAD/FOOD/points */
void resetGame(int **feld, struct position *tail){

	clear(); 											/* Bildschirm leeren */
	printFeld(); 										/*Ueberschrift und Rand zeichnen */
	initalFeld(feld, BLANK); 							/* Spielfeldarray initialisieren */
	initalTail(tail); 									/* Schwanz initialisieren */

	/* HEAD platzieren */
	pos.x	= width / 2;
	pos.y	= height / 2;
	feld[pos.x][pos.y] = HEAD; 							/* Feldarray aktualisieren */
	mvaddch(yoffset+pos.y + 4, xoffset+pos.x + 1, '@');	/* Figur zeichnen */

	/* FOOD Platzieren */
	setFood(feld);

	/* Spielvariablen ruecksetzten */
	tailcount = 0;
	points = 0;

	/* Punkte anzeigen */
	mvprintw(yoffset+height+5,xoffset,"Punkte %d",points);

	/* Wenn Option Automove aktiv */
	if(autoMove)nodelay(stdscr, TRUE);
}

/* Fuehrt Spielzug auf feldarray aus aktualisiert das tailarray
 * und zeichnet die Figuren und Punktzahl
 * Beisst sich die Schlange in den Schwanz return 0 (Game over) sonst 1*/
int setFeld(int **feld, struct position *tail, int bevor){

	int ret = 1;									/* Wenn die Schlange in den Schwanz beisst return 0 */
	int figur;										/* Die Figur auf der neuen Position im Feld (BLANK,TAIL,FOOD) */
	static int hasFood	= 0;						/* Wenn die Schlange grad was zu Essen fand */
	int cnt;
	struct position tailPos;						/* Position für tail-Elemente */

	/* Vor dem Zug. Der HEAD verlaesst die Position pos
	 * Den Schwanz verschieben und zeichnen */
	if(bevor){

		/* Die von HEAD verlassende position pos leeren */
		feld[pos.x][pos.y] = BLANK;
		mvaddch(yoffset+pos.y+4, xoffset+pos.x + 1, ' ');

		/* Hat die Schlange ueberhaupt einen Schwanz */
		if(tailcount > 0){

			/* Hat die Schlange zuvor kein FOOD gefunden verschiebe Schwanz */
			if(! hasFood){

				/*Schwanzende loeschen */
				tailPos = tail[0];
				feld[ tailPos.x ][ tailPos.y ] = BLANK;
				mvaddch(yoffset+tailPos.y+4, xoffset+tailPos.x + 1, ' ');

				/* Wenn Schwanzlaenge > 1 verschieben */
				if(tailcount > 1){
					for(cnt=0; cnt<tailcount-1; cnt++)
						tail[cnt] = tail[cnt +1];
					tail[tailcount - 1] = pos; /* Neue Position vor dem HEAD */
				}

				/* Sonst einfach neuen Anfang setzten */
				else tail[0] = pos;

			/* oder FOOD wurde zuvor gefunden und wird jetzt verdaut */
			}else hasFood = 0;

			/* Schwanzanfang im Spielfeld setzen und zeichnen */
			feld[pos.x][pos.y] = TAIL;
			mvaddch(yoffset+pos.y + 4, xoffset+pos.x + 1, '*');
		}

	/* Oder nach dem Zug. Der HEAD besetzt die Position pos
	 * Figuren auswerten und neu Zeichnen*/
	}else{
			figur = feld[pos.x][pos.y];		/* was ist aktuell auf der neuen Position */
			if(figur==TAIL) ret = 0;		/* TAIL: In den Schwanz gebissen - Gameover */
			else if(figur==FOOD){ 			/* FOOD: Was zu Essen gefunden */
				tail[tailcount] = pos;		/* Neues tail-Element einfügen */
				setFood(feld);				/* Frischen Essen servieren :) */
				hasFood = 1;				/* Fuer nachsten Zug zu Verdauung vormerken */
				++tailcount;
				points += 10;
			}
			/* HEAD zeichnen und im Spielfeldarray setzten */
			feld[pos.x][pos.y] = HEAD;
			mvaddch(yoffset+pos.y + 4,xoffset+pos.x+1, '@');

			/* Punke anzeigen */
			mvprintw(yoffset+height+5,xoffset,"Punkte %d",points);
	}
	return ret;
}

/* Beendet curses */
void quit(){endwin();}

int main(int argc, char *argv[]){

	int **feld;								/* Spielfeld */
	struct position *tail; 					/* Schwanz */
	int cnt;								/* Schleifenvariable */
	int myx; 								/* Zur Spielfeldzentrierung */
	int myy; 								/* Dito */
	srand( (unsigned int) time(NULL) );		/* Zufall initialisieren */

	/* Bei unkorrekten Komandozeilenargumenten erfolgt Abbruch */
	if(argc > 2){
		width=atoi(argv[1]);
		height=atoi(argv[2]);
		if(width > 1 && height > 1){
			feld = malloc(width * sizeof(int *));
			for(cnt=0; cnt<width; cnt++)feld[cnt]=malloc(height * sizeof(int));
			tail = (struct position *)malloc((width * height +1) * (sizeof(struct position)));
		}else{
			printf("Aufruf mit 'snake breite hoehe [option a=Automove]' min: 'snake 2 2'\n");
			printf("Steuerung mit 'w' 'a' 's' 'd' Ende 'x'\n");
			return 0;
		}
	}else{
		printf("Aufruf mit 'snake breite hoehe [option a=Automove]' min: 'snake 2 2'\n");
		printf("Steuerung mit 'w' 'a' 's' 'd' Ende 'x'\n");
		return 0;
	}

	/* Ist Automove aktiviert ?*/
	if(argc>3)autoMove = 1;

	/* curses initialisieren */
	initscr();
	atexit(quit);
	curs_set(0);
	noecho();
	start_color();
	init_pair(1, COLOR_YELLOW, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);
	bkgd(COLOR_PAIR(1));
	cbreak();
	keypad(stdscr, TRUE);

	/*Spielfeld zentrieren */
	getmaxyx(stdscr ,myy, myx);
	if(width>myx-4 || height>myy-4){ /* Spielfeld zu gross fuer Screen */
		attrset(A_BOLD);
		mvprintw(LINES/2,0,"Das Spielfeld ist zu gross max:%d x %d",myx-4,myy-4);
		mvprintw(LINES/2+1,0,"mit taste beenden und mit max neu aufrufen");
		getch();
		return 0;
	}
	xoffset = myx/2 - width/2 -1;
	yoffset = myy/2 - (height+6)/2;

	/* Spiel starten */
	resetGame(feld, tail);
	while(richtung != 'x'){

		/* Wenn Automove aktiv sleep() aktivieren*/
		if(autoMove){
			if(richtung == 'a' || richtung == 'd')mmillisleep(100);
			else if(richtung == 'w' || richtung == 's')mmillisleep(150);
		}

		/* Neue Richtung einlesen */
		char key = getch();
		if(key=='w' || key=='a' || key=='s' || key=='d' || key=='x')
			richtung = key;

		/* Spielfeld vor dem Zug setzten. HEAD verlaesst die pos */
		setFeld(feld, tail, 1);

		/* Zug Ausführen */
		myMove();

		/* Spielfeld nach dem Zug setzten
		 * gewonnen und verloren auswerten */
		if(! setFeld(feld, tail, 0) || points > width*height*10 -30){

			/* damit auf getch() gewartet wird */
			nodelay(stdscr, FALSE);

			/* Schriftfarbe aendern */
			attrset(A_BOLD|COLOR_PAIR(2));

			/* Gewonnen ?*/
			if(points > width*height*10 -30)
				mvprintw(yoffset+height/2,xoffset+2,"Sie haben gewonnen! Nochmal j");

			/* oder verloren */
			else
				mvprintw(yoffset+height/2,xoffset+2,"Sie haben verloren! Nochmal j");
			attrset(A_BOLD|COLOR_PAIR(1));
			key = getch();
			if(key != 'j') break; /*Spiel verlassen */
			else resetGame(feld,tail); /*Spiel neu starten */
		}

	/* Naechster/neuer zug beginnt */
	}
	quit(); /* curses beenden */
	free(tail); /* Speicher wieder frei geben */
	free(feld); /* dito */
}





