// Sebastian Dante Corno
// Ottimizzato da: Dilan Bertini

///---------------------------!ATTENZIONE!-------------------------------///
/// Non avviare il file dal progetto, ma dalla cartella dell'eseguibile. ///
///----------------------------------------------------------------------///
/// La struttura della cartella e dell'eseguibile è:                     ///
///                                                                      ///
/// GermanInvaders.exe                                                   ///
/// songs                                                                ///
/// |_-song.mp3                                                          ///
/// |                                                                    ///
/// levels                                                               ///
/// |_-level_1.txt                                                       ///
///   -level_2.txt                                                       ///
///   -level_3.txt                                                       ///
///   -level_n.txt                                                       ///
///   -default.txt                                                       ///
///----------------------------------------------------------------------///

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <conio.h>
#include <fstream>

using namespace std;

const int MAP_SIZE = 20;
char Map[MAP_SIZE][MAP_SIZE] = {}; // Mappa di gioco

// Variabili di gioco
bool gameOver = false, victory = false;
enum ACTION {STOP = 0, UP, DOWN, RIGHT, LEFT, ATTACK};
int level = 1;

// Variabili del player
int pX, pY;
int life, nEnemy;
ACTION pDir;

/// ANTI FLICKERING System

void cls() {
    // Get the Win32 handle representing standard output.
    // This generally only has to be done once, so we make it static.
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD topLeft = { 0, 0 };

    // std::cout uses a buffer to batch writes to the underlying console.
    // We need to flush that to the console because we're circumventing
    // std::cout entirely; after we clear the console, we don't want
    // stale buffered text to randomly be written out.
    cout.flush();

    // Figure out the current width and height of the console window
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
        // TODO: Handle failure!
        abort();
    }
    DWORD length = csbi.dwSize.X * csbi.dwSize.Y;

    DWORD written;

    // Flood-fill the console with spaces to clear it
    FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

    // Reset the attributes of every character to the default.
    // This clears all background color formatting, if any.
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, length, topLeft, &written);

    // Move the cursor back to the top left for the next sequence of writes
    SetConsoleCursorPosition(hOut, topLeft);
}

void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}

void setCursorPosition(int x, int y)
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
    ShowConsoleCursor(false);
}

/// ANTI FLICKERING System

// Aggiornamento di un singolo carattere
void update(int x, int y, char ch) {
    setCursorPosition(x,y);
    cout << ch;
}

// Valida il livello prima che venga caricato nella matrice
bool levelValidator() {
    // Bool for validator
    bool sizeW = true;
    bool sizeH = false;
    bool enemy = false;
    bool spawnPoint = false;

    int spawnPointCounter = 0;
    int lines = 0;
    int enemyCounter = 0;
    string line;
    ifstream inFile;
    inFile.open("levels\\level_" + to_string(level) + ".txt"); // Apre il file
    if (!inFile) {
        cls();
        cout << "Imposibile caricare/verficare il livello " << level << endl;
        system("PAUSE");
        exit(1);
    }

    // Controllo e Conteggio delle varie entita'
    while (getline(inFile, line)) {
        lines++;
        if (line.size() != 20 && sizeW) {
            sizeW = false;
        }
        for (unsigned int i = 0; i < line.size(); i++) {
            if (line[i] == 'H') {
                enemyCounter++;
            }
            if (line[i] == 'X') {
                spawnPointCounter++;
                if (spawnPointCounter == 1) {
                    pY = lines-1;
                    pX = i;
                }
            }

        }
    }
    inFile.close();

    // Controllo finale
    if (spawnPointCounter == 1) {
        spawnPoint = true;
    }

    if (lines == 20) {
        sizeH = true;
    }

    if (enemyCounter > 0) {
        enemy = true;
    }

    if (sizeH && sizeW && enemy && spawnPoint) {
        nEnemy = enemyCounter;
    }

    // Report per l'utente
    cout << "Report: " << endl;
    if (!sizeH || !sizeW) {
        cout << "Non e' un 20x20"  << endl;
    }

    if (!enemy) {
        cout << "Non e' presente alcun nemico" << endl;
    }

    if (!spawnPoint) {
        cout << "Non e' presente uno spawnPoint o sono più di uno" << endl;
    }

    cout << "SpawnPoint X: " << pX << endl;
    cout << "SpawnPoint Y: " << pY << endl;
    cout << "Enemy: " << enemyCounter << endl;
    Sleep(2500);
    cls();
    return sizeH && sizeW && enemy && spawnPoint;
}

// Caricamento del livello all'interno della matrice
void loadLevel() {
    int iY = 0;
    string line;
    ifstream inFile;
    inFile.open("levels\\level_" + to_string(level) + ".txt");
    if (!inFile) {
        cls();
        cout << "Imposibile caricare/verficare il livello " << level << endl;
        system("PAUSE");
        exit(1);
    }
    while (getline(inFile, line)) {
        for (unsigned int i = 0; i < line.size(); i++) {
            if (line[i] == '#' || line[i] == 'H' || line[i] == '/') {
                Map[iY][i] = line[i];
            } else {
                Map[iY][i] = ' ';
            }
        }
        iY++;
    }
}

// Inizializzazione o Reset per ogni livello
void setUp() {
    srand(time(NULL));
    gameOver = false;
    victory = false;

    // Validazione del livello
    if (levelValidator()) {
        loadLevel();
    } else {
        cout << "Il livello " << level << " non e' valido" << endl;
        system("PAUSE");
        exit(1);
    }
    life = 1;

    // Prima stampa della mappa
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (x == pX && y == pY) {
                cout << "w";
            } else {
                cout << Map[y][x];
            }
        }
        cout << endl;
    }
}

// Input
void Input(){
    if(_kbhit()){
        switch (_getch())
        {
        case 'w':
            pDir = UP;
            break;
        case 'a':
            pDir = LEFT;
            break;
        case 'd':
            pDir = RIGHT;
            break;
        case 's':
            pDir = DOWN;
            break;
        case ' ':
            pDir = ATTACK;
            break;
        }

    }
}

// Meccaniche di gioco
void Logic() {
    bool canItShoot;
    int i;

    // Movimento e generazione dei proiettili
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            canItShoot = false;
            switch (Map[y][x]) {
            case '^':
                Map[y][x] = ' ';
                update(x,y,' ');
                y--;
                if (Map[y][x] != '#' && Map[y][x] != 'H' && Map[y][x] != 'I') {
                    Map[y][x] = '^';
                    update(x,y, '^');
                }

                if (Map[y][x] == 'H') {
                    nEnemy--;
                    if (nEnemy <= 0) {
                        victory = true;
                    }
                }

                if (Map[y][x] == 'H' || Map[y][x] == 'I') {
                        Map[y][x] = ' ';
                        update(x,y,' ');
                }
                break;
            case 'H':
                i = y+1;
                canItShoot = true;
                while (i < MAP_SIZE && canItShoot == true) {
                    if (Map[i][x] == 'H')
                        canItShoot = false;
                    i++;
                }

                if (canItShoot) {
                    if ((rand() % 10) + 1 == 1) {
                        y++;
                        Map[y][x] = 'I';
                        update(x,y,'I');
                    }
                }

                break;
            case 'I':
                Map[y][x] = ' ';
                update(x,y,' ');
                y++;
                if (Map[y][x] != '#' && (x != pX || y != pY) && Map[y][x] != '^') {
                    Map[y][x] = 'I';
                    update(x,y,'I');
                }
                if (x == pX && y == pY) {
                    life--;
                    if (life <= 0)
                        gameOver = true;
                }
                break;
             default:
                break;
            }
        }
    }

    // Movimenti del player
    switch (pDir) {
    case UP:
        if (Map[pY-1][pX] == ' ') {
            update(pX,pY, ' ');
            pY--;
            update(pX,pY, 'w');
        }
        break;
    case DOWN:
        if (Map[pY+1][pX] == ' ') {
            update(pX,pY, ' ');
            pY++;
            update(pX,pY, 'w');
        }
        break;
    case RIGHT:
        if (Map[pY][pX+1] == ' ') {
            update(pX,pY, ' ');
            pX++;
            update(pX,pY,'w');
        }
        break;
    case LEFT:
        if (Map[pY][pX-1] == ' ') {
            update(pX,pY, ' ');
            pX--;
            update(pX,pY,'w');
        }
        break;
    case ATTACK:
        if (Map[pY-1][pX] != '#') {
            Map[pY-1][pX] = '^';
            update(pX,pY-1,'^');
        }
        break;
    default:
        break;
    }
    pDir = STOP;
}

int main() {
    //system("start /min songs\\song.mp3"); /// Avvia la "canzone"
    while (true){
        setUp();
        while (!gameOver && !victory) {
            Sleep(100);
            Input();
            Logic();
        }
        cls();
        if (victory) {
            cout << "Hai vinto premi invio per passare al prossimo livello" << endl;
            level++;
        } else {
            cout << "Hai perso premi invio per ricominciare il livello" << endl;
        }
        system("PAUSE");
    }
    return 0;
}
