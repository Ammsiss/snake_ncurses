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

enum class Speed
{
    slow,
    medium,
    insane,
};

enum class SnakeColor
{
    Red,
    Blue,
    Green,
    Yellow,
    Magenta,
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

Speed setUpGameWin(WINDOW* gameW, WINDOW* scoreW, int y, int x)
{
    mvwprintw(gameW, y/2 - 5, x/2 - 10, "CHOOSE A SPEED!!!");
    std::vector<std::string> options{ "SLOW", "MEDIUM", "INSANE"};

    std::size_t selection{0};

    Speed speed{};

    while(true)
    {
        for (std::size_t i{0}; i < options.size(); ++i)
        {
            if (selection == i)
            {
                wattron(gameW, A_BLINK);
                wattron(gameW, A_REVERSE);
                mvwprintw(gameW, ((y/2 - 5) + (2 * (i + 1))), (x/2) - 4, "%s", options[i].c_str());
                wattroff(gameW, A_BLINK);
                wattroff(gameW, A_REVERSE);
            }
            else
            {
                mvwprintw(gameW, ((y/2 - 5) + (2 * (i + 1))), (x/2) - 4, "%s", options[i].c_str());
            }
        }

        int userInput{ wgetch(gameW) };

        switch(userInput)
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

        if (userInput == '\n' && selection == 0)
        {
            speed = Speed::slow;
            break;
        }
        else if (userInput == '\n' && selection == 1)
        {
            speed = Speed::medium;
            break;
        }
        else if (userInput == '\n' && selection == 2)
        {
            speed = Speed::insane;
            break;
        }

        wrefresh(gameW);
    }

    wclear(gameW);
    // creae game box
    box(gameW, 0, 0);
    // create score box
    box(scoreW, 0, 0);
    mvwprintw(scoreW, 1, 1, "SCORE: ");
    wrefresh(gameW);
    wrefresh(scoreW);

    return speed;
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

void updateSnake(std::deque<Snake>& snake, WINDOW* gameW, char gameInput, bool pelletCollected, Pellet pelletCordinates, int y, int x, SnakeColor snakeColor)
{
    start_color();

    init_color(COLOR_BLACK, 0, 0, 0);
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);

    if (pelletCollected)
    {
        snake.push_back({snake[snake.size() - 1].snakeY, snake[snake.size() - 1].snakeX});
    }
    else
    {
        mvwprintw(gameW, snake[snake.size() - 1].snakeY, snake[snake.size() - 1].snakeX, " ");
    }

    for (int iTwo{static_cast<int>(snake.size() - 1)}; iTwo >= 1; --iTwo)
    {
        snake[iTwo].snakeY = snake[iTwo - 1].snakeY;
        snake[iTwo].snakeX = snake[iTwo - 1].snakeX;
    }
    
    switch(snakeColor)
    {
        case SnakeColor::Red:
        wattron(gameW, COLOR_PAIR(1));
        break;
        case SnakeColor::Blue:
        wattron(gameW, COLOR_PAIR(2));
        break;
        case SnakeColor::Green:
        wattron(gameW, COLOR_PAIR(3));
        break;
        case SnakeColor::Yellow:
        wattron(gameW, COLOR_PAIR(4));
        break;
        case SnakeColor::Magenta:
        wattron(gameW, COLOR_PAIR(5));
        break;
        default:
        break;
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

    if (gameInput == 'w')
    {
        mvwprintw(gameW, snake[0].snakeY, snake[0].snakeX, "v");
    }
    else if (gameInput == 'a')
    {
        mvwprintw(gameW, snake[0].snakeY, snake[0].snakeX, ">");
    }
    else if (gameInput == 's')
    {
        mvwprintw(gameW, snake[0].snakeY, snake[0].snakeX, "^");
    }
    else if (gameInput == 'd')
    {
        mvwprintw(gameW, snake[0].snakeY, snake[0].snakeX, "<");
    }

    for (std::size_t i{1}; i < snake.size(); ++i)
    {
        if (i % 2 == 0)
        {
            mvwprintw(gameW, snake[i].snakeY, snake[i].snakeX, "x");
        }
        else
        {
            mvwprintw(gameW, snake[i].snakeY, snake[i].snakeX, "+");

        }
    }

    switch(snakeColor)
    {
        case SnakeColor::Red:
        wattroff(gameW, COLOR_PAIR(1));
        break;
        case SnakeColor::Blue:
        wattroff(gameW, COLOR_PAIR(2));
        break;
        case SnakeColor::Green:
        wattroff(gameW, COLOR_PAIR(3));
        break;
        case SnakeColor::Yellow:
        wattroff(gameW, COLOR_PAIR(4));
        break;
        case SnakeColor::Magenta:
        wattroff(gameW, COLOR_PAIR(5));
        break;
        default:
        break;
    }

    wrefresh(gameW);
}

char userInput(char gameInput, char inputReset)
{
    switch(gameInput)
    {
        case 'w':
        if (inputReset == 's') { break; }
        else { return gameInput; }
        case 'a':
        if (inputReset == 'd') { break; }
        else { return gameInput; }
        case 's':
        if (inputReset == 'w') { break; }
        else { return gameInput; }
        case 'd':
        if (inputReset == 'a') { break; }
        else { return gameInput; }
        default: 
        return inputReset;
    }
    return inputReset;
}

void printScore(WINDOW* scoreW, int pelletCount)
{
    box(scoreW, 0, 0);
    mvwprintw(scoreW, 1, 1, "             ");
    mvwprintw(scoreW, 1, 1, "SCORE: ");
    mvwprintw(scoreW, 1, 9, "%d", pelletCount * 100);
    wrefresh(scoreW);
}

bool checkDie(const std::deque<Snake>& snake)
{
    for (std::size_t i{1}; i < snake.size(); ++i)
    {
        if (snake[0].snakeY == snake[i].snakeY && snake[0].snakeX == snake[i].snakeX)
        {
            return true;
        }    
    }

    return false;
}

void resetScore(WINDOW* scoreW)
{
    mvwprintw(scoreW, 1, 1, "             ");
    mvwprintw(scoreW, 1, 1, "SCORE: ");
}

// bool returns true if user won!
WinCode gameLoop(WINDOW* gameW, WINDOW* scoreW, const int y, const int x, SnakeColor snakeColor)
{
    using namespace std::literals::chrono_literals;

    Speed speed{};

    speed = setUpGameWin(gameW, scoreW, y, x);
    int speedMS{};

    switch(speed)
    {
        case Speed::slow:
        speedMS = 400;
        break;
        case Speed::medium:
        speedMS = 150;
        break;
        case Speed::insane:
        speedMS = 75;
        break;
        default:
        speedMS = 150;
        break;
    }


    // init main snake!
    std::deque<Snake> snake(1);
    snake[0].snakeY = y / 2;
    if ((x / 2) % 2 == 0)
    {
        snake[0].snakeX = (x / 2) + 1;
    }
    else
    {
        snake[0].snakeX = (x / 2);
    }

    // init pellet stuff!
    int pelletCount{0};
    Pellet pelletCordinates{};
    std::uniform_int_distribution<int> gridX{};
    if (x % 2 == 0)
    {
        gridX = std::uniform_int_distribution<int>{ 0, (x - 4) / 2 };
    }
    else if (x % 2 != 0)
    {
        gridX = std::uniform_int_distribution<int>{ 0, (x / 2) - 1};
    }
    std::uniform_int_distribution<int> gridY{ 1, (y - 2)};
    pelletCordinates.pelletY = gridY(Random::mt);
    pelletCordinates.pelletX = ((2 * gridX(Random::mt)) + 1);
    bool spawnFirstPellet{true};
    bool pelletCollected{false};

    // enable no pause in exe with getch()
    nodelay(gameW, TRUE);
    char gameInput{'d'};
    char inputReset{};

    // sets interval that snake moves../n
    auto interval{ std::chrono::milliseconds(speedMS) };
    auto lastTime{ std::chrono::high_resolution_clock::now() };
    while (true)
    {
        // updates current time after every loop (instantly basically)
        auto currentTime{ std::chrono::high_resolution_clock::now() };

        // checks if user went out of bounds
        if (outOfBounds(y, x, snake))
        {
            resetScore(scoreW);
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
                bool insideSnake{true};
                while(insideSnake)
                {
                    insideSnake = false;
                    pelletCordinates.pelletY = gridY(Random::mt);
                    pelletCordinates.pelletX = ((2 * gridX(Random::mt)) + 1);

                    for (std::size_t i{1}; i < snake.size(); ++i)
                    {
                        if (pelletCordinates.pelletY == snake[i].snakeY && pelletCordinates.pelletX == snake[i].snakeX)
                        {
                            insideSnake = true;
                        }
                    }
                }
                ++pelletCount;
                printScore(scoreW, pelletCount);
                mvwprintw(gameW, pelletCordinates.pelletY, pelletCordinates.pelletX, "*");
                wrefresh(gameW);
                pelletCollected = true;
            }

            updateSnake(snake, gameW, gameInput, pelletCollected, pelletCordinates, y, x, snakeColor);
            pelletCollected = false;
            
            if (checkDie(snake))
            {
                resetScore(scoreW);
                return WinCode::Lost;
            }

            // resets timer and lets thread sleep to avoid infinite loop buffer
            lastTime = currentTime;
        }
        std::this_thread::sleep_for(10ms);
    }
}

// returns construction message
SnakeColor settingsLoop(WINDOW* scoreW, WINDOW* gameW, int y, int x)
{
    box(gameW, 0, 0);

    mvwprintw(gameW, y/2 - 5, x/2 - 10, "CHOOSE A COLOR FOR YOUR SNAKE!");
    std::vector<std::string> options{ "1) $5.99$ RED", "2) $8.99$ BLUE", "3) $$15.99$$ GREEN", "4) $$$209.99$$$ BROWN", "5) $.99$ MAGENTA %50 off !!!" };

    std::size_t selection{0};

    while(true)
    {
        for (std::size_t i{0}; i < options.size(); ++i)
        {
            if (selection == i)
            {
                wattron(gameW, A_BLINK);
                wattron(gameW, A_REVERSE);
                mvwprintw(gameW, ((y/2 - 5) + (2 * (i + 1))), (x/2), "%s", options[i].c_str());
                wattroff(gameW, A_BLINK);
                wattroff(gameW, A_REVERSE);
            }
            else
            {
                mvwprintw(gameW, ((y/2 - 5) + (2 * (i + 1))), (x/2), "%s", options[i].c_str());
            }
        }

        int userInput{ wgetch(gameW) };

        switch(userInput)
        {
            case 'w':
            if (selection != 0)
            {
                --selection;
                break;
            }
            break;
            case 's':
            if (selection != 4)
            {
                ++selection;
                break;
            }
            break;
            default:
            break;
        }

        if (userInput == '\n' && selection == 0)
        {
            return SnakeColor::Red;
        }
        else if (userInput == '\n' && selection == 1)
        {
            return SnakeColor::Blue;
        }
        else if (userInput == '\n' && selection == 2)
        {
            return SnakeColor::Green;
        }
        else if (userInput == '\n' && selection == 3)
        {
            return SnakeColor::Yellow;
        }
        else if (userInput == '\n' && selection == 4)
        {
            return SnakeColor::Magenta;
        }

        wrefresh(gameW);
    }
}

void exitGame(WINDOW* scoreW, WINDOW* gameW, int y, int x)
{
    mvwprintw(gameW, y/2, x/2 - 4, "SEE YA!");
    wclear(scoreW);
    box(gameW, 0, 0);
    wrefresh(scoreW);
    wrefresh(gameW);
}

void loseMessage(WINDOW* gameW, int y, int x)
{
    mvwprintw(gameW, y/2, x/2 - 5, "YOU LOSE!");
    wrefresh(gameW);
    getch();
}

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

    MenuCode menuSelection{};

    SnakeColor snakeColor{ SnakeColor::Green };

    // main loop
    for(int i{0}; i < 10; ++i)
    {
        wclear(gameW);

        // gets user menu selection. Play, settings, exit.
        menuSelection = menuLoop(gameW);
        wclear(gameW);

        if (menuSelection == MenuCode::Exit)
        {
            exitGame(scoreW, gameW, y, x);
            break;
        }
        
        if (menuSelection == MenuCode::Settings)
        {
            snakeColor = settingsLoop(scoreW, gameW, y, x);
        }

        if (menuSelection == MenuCode::Play)
        {
            WinCode wonOrLost{ gameLoop(gameW, scoreW, y, x, snakeColor) };

            if (wonOrLost == WinCode::Lost)
            {
                loseMessage(gameW, y, x);
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