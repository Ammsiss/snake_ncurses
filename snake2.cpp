#include <ncurses.h>
#include <deque>
#include <chrono>
#include <stdio.h>
#include <string>
#include <thread>
#include <random>
#include <vector>

namespace Random
{
    std::mt19937 mt{ std::random_device{}() };
}

struct Snake
{
    int snakeY{};
    int snakeX{};

    Snake() = default;
    Snake (int x, int y) : snakeY(x), snakeX(y) {}
};

struct Pellet
{
    int pelletY{};
    int pelletX{};
};

enum class MenuCode
{
    Play,
    Settings,
    Exit,
};

enum class WinCode
{
    Won,
    Lost,
};

// creates game window to fit stdscr
WINDOW* initGameWindow(int y, int x)
{
    WINDOW* gameW{ newwin(y, x, 0, 0) };
    return gameW;
}

WINDOW* initScoreWindow(int x)
{
    return newwin(3, 15, 0, (x/2) - 7);
}

// gets user menu selection
MenuCode menuLoop(WINDOW* gameW)
{
    std::vector<std::string> choices{"Play Snake!", "Settings!", "Exit Snake!"};
    std::size_t selection{0};
    char input{};

    int x{};
    int y{};
    getmaxyx(stdscr, y, x);

    // create menu box
    box(gameW, 0, 0);
    wrefresh(gameW);

    while(true)
        {
            for (std::size_t index{0}; index < choices.size(); ++index)
            {
                if (index == selection)
                {
                    wattron(gameW, A_BLINK);
                    wattron(gameW, A_REVERSE);
                    mvwprintw(gameW, ((y/2) - 2) + (2 * index), ((x/2) - 5), "%s", choices[index].c_str());
                    wattroff(gameW, A_BLINK);
                    wattroff(gameW, A_REVERSE);
                    continue;
                }
                    
                mvwprintw(gameW, ((y/2) - 2) + (2 * index), ((x/2) - 5), "%s", choices[index].c_str());
                wrefresh(gameW);
            }

            input = wgetch(gameW);

            switch(input)
            {
                case 'w':
                if (selection != 0)
                {
                    --selection;
                    break;
                }
                break;
                case 's':
                if (selection != 2)
                {
                    ++selection;
                    break;
                }
                break;
                default:
                break;
            }

            if (input == '\n' && selection == 0)
            {
                return MenuCode::Play;
            }
            else if (input == '\n' && selection == 1)
            {
                return MenuCode::Settings;
            }
            else if (input == '\n' && selection == 2)
            {
                return MenuCode::Exit;
            }
        }
}

void setUpGameWin(WINDOW* gameW, WINDOW* scoreW, int y, int x)
{
    // creae game box
    box(gameW, 0, 0);
    // create score box
    box(scoreW, 0, 0);
    mvwprintw(scoreW, 1, 1, "SCORE: ");
    wrefresh(gameW);
    wrefresh(scoreW);
}

// bool returns true if went out of bounds
bool outOfBounds(int y, int x, std::deque<Snake> snake)
{
    if (snake[0].snakeY <= 0 || snake[0].snakeY >= (y - 1) || snake[0].snakeX <= 0 || snake[0].snakeX >= (x - 1))
    {
        return true;
    }
    else
    {
        return false;
    }
}



void updateSnake(std::deque<Snake>& snake, WINDOW* gameW, char gameInput, bool pelletCollected)
{
    if (pelletCollected)
    {
        snake.push_back({snake[snake.size() - 1].snakeY, snake[snake.size() - 1].snakeX});
    }
    else
    {
        mvwprintw(gameW, snake[snake.size() - 1].snakeY, snake[snake.size() - 1].snakeX, " ");
    }

    for (int iTwo{1}; iTwo <= snake.size() - 1; ++iTwo)
    {
        snake[iTwo].snakeY = snake[iTwo - 1].snakeY;
        snake[iTwo].snakeX = snake[iTwo - 1].snakeX;
    }
    
    switch(gameInput)
    {
        case 'w':
        --snake[0].snakeY;
        break;
        case 's':
        ++snake[0].snakeY;
        break;
        case 'a':
        snake[0].snakeX -= 2;
        break;
        case 'd':
        snake[0].snakeX += 2;
        break;
        default:
        break;
    }

    mvwprintw(gameW, snake[0].snakeY, snake[0].snakeX, "X");
    wrefresh(gameW);
}

char userInput(char input, char inputReset)
{
    while(true)
    {
        switch(input)
        {
            case 'w':
            case 'a':
            case 's':
            case 'd':
            return input;
            break;
            default: 
            return inputReset;
            break;
        }
    }
}

void printScore(WINDOW* scoreW, int pelletCount)
{
    mvwprintw(scoreW, 1, 9, "%d", pelletCount * 100);
    wrefresh(scoreW);
}

// bool returns true if user won!
WinCode gameLoop(WINDOW* gameW, WINDOW* scoreW, int y, int x)
{
    using namespace std::literals::chrono_literals;

    setUpGameWin(gameW, scoreW, y, x);

    // enable no pause in exe with getch()
    nodelay(gameW, TRUE);
    char gameInput{'d'};
    char inputReset{'d'};

    // init main snake!
    std::deque<Snake> snake(1);
    snake[0].snakeY = y/2;
    snake[0].snakeX = x/2;

    // init pellet stuff!
    int pelletCount{0};
    Pellet pelletCordinates{};
    std::uniform_int_distribution<int> gridX{};
    if (x % 2 == 0)
    {
        gridX = std::uniform_int_distribution<int>{ 0, (x - 2) / 2 };
    }
    else
    {
        gridX = std::uniform_int_distribution<int>{ 0, x / 2};
    }
    std::uniform_int_distribution<int> gridY{ 1, (y - 2)};
    pelletCordinates.pelletY = gridY(Random::mt);
    pelletCordinates.pelletX = (2 * gridX(Random::mt) + 1);
    bool spawnFirstPellet{true};
    bool pelletCollected{false};

    // sets interval that snake moves../n
    auto interval{ std::chrono::milliseconds(50) };
    auto lastTime{ std::chrono::high_resolution_clock::now() };
    while (true)
    {
        // updates current time after every loop (instantly basically)
        auto currentTime{ std::chrono::high_resolution_clock::now() };

        // checks if user went out of bounds
        if (outOfBounds(y, x, snake))
        {
            return WinCode::Lost;
        }

        inputReset = gameInput;
        gameInput = wgetch(gameW);
        gameInput = userInput(gameInput, inputReset);

        if (spawnFirstPellet)
        {
            mvwprintw(gameW, pelletCordinates.pelletY, pelletCordinates.pelletX, "*");
            spawnFirstPellet = false;
        }



        if (currentTime - lastTime >= interval)
        {
            if (snake[0].snakeY == pelletCordinates.pelletY && snake[0].snakeX == pelletCordinates.pelletX)
            {
                // randomizes pellet position and increases pelletCount and prints score.
                pelletCordinates.pelletY = gridY(Random::mt);
                pelletCordinates.pelletX = (2 * gridX(Random::mt) + 1);
                ++pelletCount;
                printScore(scoreW, pelletCount);
                mvwprintw(gameW, pelletCordinates.pelletY, pelletCordinates.pelletX, "*");
                wrefresh(gameW);
                pelletCollected = true;
            }

            updateSnake(snake, gameW, gameInput, pelletCollected);
            pelletCollected = false;

            // resets timer and lets thread sleep to avoid infinite loop buffer
            lastTime = currentTime;
            std::this_thread::sleep_for(50ms);
        }
    }
}

// returns construction message
void settingsLoop(WINDOW* scoreW, WINDOW* gameW, int y, int x)
{
    mvwprintw(gameW, y/2, x/2 - 10, "UNDER CONSTSTRUCTION!");
    wclear(scoreW);
    box(gameW, 0, 0);
    wrefresh(scoreW);
    wrefresh(gameW);
    wgetch(gameW);
    wclear(gameW);
}


void exitGame(WINDOW* scoreW, WINDOW* gameW, int y, int x)
{
    mvwprintw(gameW, y/2, x/2 - 4, "SEE YA!");
    wclear(scoreW);
    box(gameW, 0, 0);
    wrefresh(scoreW);
    wrefresh(gameW);
}

/* void loseMessage()
{
    
}
*/

/* void winMessage()
{

}
*/

int main()
{
    // set up ncurses
    initscr();
    noecho();
    cbreak();
    // hide cursor
    curs_set(0);
    // set up subwindows
    refresh();

    // create window dimension variables
    int y{};
    int x{};
    getmaxyx(stdscr, y, x);

    // initialize windows
    WINDOW* gameW{ initGameWindow(y, x)};
    WINDOW* scoreW{ initScoreWindow(x)};

    // main loop
    for(int i{0}; i < 10; ++i)
    {
        // gets user menu selection. Play, settings, exit.
        MenuCode menuSelection{ menuLoop(gameW) };
        wclear(gameW);

        if (menuSelection == MenuCode::Exit)
        {
            exitGame(scoreW, gameW, y, x);
            break;
        }
        
        if (menuSelection == MenuCode::Settings)
        {
            settingsLoop(scoreW, gameW, y, x);
        }

        if (menuSelection == MenuCode::Play)
        {
            WinCode wonOrLost{ gameLoop(gameW, scoreW, y, x) };

            if (wonOrLost == WinCode::Won) 
            {
                // winMessage();
            }
            else if (wonOrLost == WinCode::Lost)
            {
                // loseMessage();
            }
        }
    }
    
    // wait for input before exiting...
    getch();
    // de allocates windows
    delwin(gameW);
    delwin(scoreW);
    endwin();
    return 0;
    
}


// this is a update