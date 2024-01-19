#include <SFML/Graphics.hpp>
#define WIN32_LEAN_AND_MEAN
#include <vector>
#include <iostream>
#include <fstream>
#include <windows.h>

using namespace std;

const static int ROWS = 3; //кол-во строк
const static int COLS = 3; //кол-во столбцов
const static int CELL_SIZE = 100; //размер одной клетки

//возможные знаки
enum class Symbol { X, O, N };

//базовый класс игрока
class AbstractPlayer {
protected: //инкапсуляция полей
    Symbol c; //хранит знак текущего игрока
    int row, col; //выбранная клетка для хода

public:

    //конструктор по умолчанию
    AbstractPlayer() : c(Symbol::X), row(0), col(0) {}

    //конструктор с параметром (перегрузка)
    AbstractPlayer(Symbol c) : row(0), col(0) {
        this->c = c;
    }

    //конструктор копирования
    AbstractPlayer(const AbstractPlayer& p) {
        this->c = p.c;
        this->row = p.row;
        this->col = p.col;
    }

    //геттеры - сеттеры
    void setRow(int row) {
        this->row = row;
    }

    void setCol(int col) {
        this->col = col;
    }

    Symbol getSymbol() const {
        return c;
    }

    //текущий игрок делает ход
    virtual void doStep(vector<vector<Symbol>>& board) = 0;

    //дружественная функция вывода информации об объекте в поток
    friend ostream& operator<<(ostream& stream, const AbstractPlayer& p) {
        stream << "{" << p.row << "; " << p.col << "} : " << (int)p.c;
        return stream;
    }
};

//игрок - человек (наследование)
class Human : public AbstractPlayer {
public:

    //конструкторы
    Human() : AbstractPlayer() {}

    Human(Symbol c) : AbstractPlayer(c) {}

    //делает ход
    void doStep(vector<vector<Symbol>>& board) override {
        board[row][col] = c;
    }
};

//игрок компьютер (наследование)
class Ai : public AbstractPlayer {
public:

    //конструкторы
    Ai() : AbstractPlayer() {}

    Ai(Symbol c) : AbstractPlayer(c) {}

    //делает ход
    void doStep(vector<vector<Symbol>>& board) override {

        //опр всех свободных клеток компьютер ходит в любую свободную клетку
        vector<pair<int, int>> freeCells;
        for (int i = 0; i < board.size(); i++) {
            for (int j = 0; j < board[i].size(); j++) {
                if (board[i][j] == Symbol::N) {
                    freeCells.push_back({ i, j });
                }
            }
        }

        //выбираем клетку
        auto idx = freeCells[rand() % freeCells.size()];
        setRow(idx.first);
        setCol(idx.second);
        board[idx.first][idx.second] = c;
    }
};

//интерфейс для отрисовки содержимого игры
class Display {
public:
    virtual void draw(sf::RenderWindow& window) const = 0;
};

//интерфейс пользовательского ввода
class Input {
public:
    virtual void handleInput(sf::Event& event, sf::RenderWindow& window) = 0;
};

//основная игра
template<typename t>
class TicTacToe : public Display, public Input { //мн. наследование
private:
    vector<vector<Symbol>> board; //двумерный массив векторов для хранения состояния игрового поля
    vector<AbstractPlayer*> players; // вектор указателей для  хранения игроков
    int freeCells; //сколько свободных клеток осталось
    t state; //игра идет (true) или уже закончилась (false)

public:

    //конструктор по списку инициализации
    TicTacToe(initializer_list<AbstractPlayer*> list) {
        players = list;
        restart();
    }

    //сброс игры
    void restart() {
        state = true;
        freeCells = ROWS * COLS;
        board = vector<vector<Symbol>>(ROWS, vector<Symbol>(COLS, Symbol::N));
    }

    //проверка кто выиграл
    int winGame(Symbol c) const {

        //главная диагональ
        if (c == board[0][0] && c == board[1][1] && c == board[2][2])
            return true;

        //побочная
        if (c == board[0][2] && c == board[1][1] && c == board[2][0])
            return true;

        //вертикали
        for (int j = 0; j < COLS; j++) {
            if (c == board[0][j] && c == board[1][j] && c == board[2][j])
                return true;
        }

        //горизонтали
        for (int i = 0; i < ROWS; i++) {
            if (c == board[i][0] && c == board[i][1] && c == board[i][2])
                return true;
        }

        //нет выигрыша для знака c
        return false;
    }

    //обработка пользовательского ввода
    void handleInput(sf::Event& event, sf::RenderWindow& window) override {

        //нажали клавишу мыши
        if (event.type == sf::Event::MouseButtonPressed && state) {

            //получаем позицию мыши на клетке
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            int row = mousePos.y / CELL_SIZE;
            int col = mousePos.x / CELL_SIZE;

            //если клетка валидная
            if (row >= 0 && row < ROWS && col >= 0 && col < COLS && board[row][col] == Symbol::N) {

                //устанавливаем значение на поле (ход игрока)
                players[0]->setRow(row);
                players[0]->setCol(col);
                players[0]->doStep(board);
                cout << *players[0] << endl;
                freeCells--;

                //игрок выиграл?
                if (winGame(players[0]->getSymbol())) {
                    state = false;
                    MessageBoxA(NULL, "win krestiki", "result", MB_OK | MB_ICONQUESTION);
                }
                else {

                    //ничья
                    if (state && freeCells == 0) {
                        state = false;
                        MessageBoxA(NULL, "draw!", "result", MB_OK | MB_ICONQUESTION);
                    }
                    else {

                        //ход компьютера
                        players[1]->doStep(board);
                        cout << *players[1] << endl;
                        freeCells--;

                        //компьютер выиграл?
                        if (winGame(players[1]->getSymbol())) {
                            state = false;
                            MessageBoxA(NULL, "win noloki!", "result", MB_OK | MB_ICONQUESTION);
                        }
                    }
                }

                //ничья
                if (state && freeCells == 0) {
                    state = false;
                    MessageBoxA(NULL, "draw!", "result", MB_OK | MB_ICONQUESTION);
                }
            }
        }

        //нажали на кнопку Enter
        if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::Enter) {
                restart(); //сброс игры
            }
        }
    }

    //отрисовка игрового поля
    void draw(sf::RenderWindow& window) const override {
        window.clear(); //очистка окна

        //по всем клеткам игрового поля
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {

                //рисуем квадрат
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setFillColor({ 180,180,180 });
                cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);
                cell.setOutlineThickness(2);

                //отрисовка клетки
                window.draw(cell);

                //это крестик
                if (board[i][j] == Symbol::X) {
                    sf::VertexArray cross(sf::Lines, 4);
                    cross[0].position = sf::Vector2f(j * CELL_SIZE + 10, i * CELL_SIZE + 10);
                    cross[1].position = sf::Vector2f((j + 1) * CELL_SIZE - 10, (i + 1) * CELL_SIZE - 10);
                    cross[2].position = sf::Vector2f(j * CELL_SIZE + 10, (i + 1) * CELL_SIZE - 10);
                    cross[3].position = sf::Vector2f((j + 1) * CELL_SIZE - 10, i * CELL_SIZE + 10);
                    for (int k = 0; k < 4; k++) {
                        cross[k].color = { 255,0,0 };
                    }
                    window.draw(cross);
                }

                //это круг
                else if (board[i][j] == Symbol::O) {
                    sf::CircleShape circle(CELL_SIZE / 2 - 10);
                    circle.setOutlineThickness(1);
                    circle.setPosition(j * CELL_SIZE + 10, i * CELL_SIZE + 10);
                    circle.setFillColor({ 0,0,255 });
                    window.draw(circle);
                }
            }
        }

        //обновляем экран
        window.display();
    }

    //деструктор
    ~TicTacToe() {
        for (int i = 0; i < players.size(); i++) {
            delete players[i];
        }
    }
};

//главная функция
int main() {
    srand(time(0)); //инициализация счетчика сл. чисел

    //создаем игру
    sf::RenderWindow window(sf::VideoMode(COLS * CELL_SIZE, ROWS * CELL_SIZE), "Tic Tac Toe");
    TicTacToe<bool> game({ new Human(Symbol::X), new Ai(Symbol::O) });

    //пока игра активна
    while (window.isOpen()) {

        //оьбработка событий
        sf::Event event;
        while (window.pollEvent(event)) {

            //закрыли окно с игрой
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            //обработка пользовательского ввода
            game.handleInput(event, window);
        }

        //отрисовка игры
        game.draw(window);
    }

    //успешное завершение программы
    return 0;
}
