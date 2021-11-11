#include<bits/stdc++.h>
#include "rules.h"
using namespace std;

class Chess {//основной класс игры
  private:
    enum {
        EMPTY,
        PAWN, QUEEN, KING,
        KNIGHT, BISHOP, ROOK,
        BARRIER
    };//enumeration фигур и гранц и пустой клетки
    enum {
        ANGLE0,//вверх, вниз
        ANGLE45,//угол 45 градусов
        ANGLE90,//вправо, влево
        ANGLE135//угол 135 градусов
    };//enumeration векторов направления

    typedef struct {
        char figure;//переменная для хранения значения фигуры
        char mobility;//направление передвижения
        char color;//цвет клетки
        char attacked;//угроза для короля
        char special;//ходы под связкой
    } cell_t;//переменная структуры

    cell_t **desk;//выделение памяти на структуру
    char **move_table;//доска ходов
    int moveptr_list[6] = { 0, 0, 0, 0, 0, 0 };//масив строк с номерами ходами для каждой фигуры
    struct {
        int fromX, fromY, toX, toY, figure;//координаты хода и фигуры, которая уже совершила ход
    } current_move;//структура хранящая координаты текущего хода
    int side;//1 - белые или 0 - чёрные
    int castling_active[2][3];//масив для совершения рокировок
    //2*3 = 6 фигур, которые берут учавстивие в рокировке: 2 короля и 4 ладьи

    struct {
        int exist, X, Y;
    } extra;//структура для взятия на проходе

    typedef void (Chess::*line_handler_t)(int, int, int, int);//указатель на функцию генерации ходов
    typedef void(Chess::*cell_handler_t)(int, int);//указатель на функцию обработчиков

    //установка начальных позиций доски, состояний рокировок и первого хода
    const char *starting =
        "rgbqkbgr"//white figures
        "pppppppp"
        "........"
        "........"
        "........"
        "........"
        "PPPPPPPP"
        "RGBQKBGR"//black figures
        "000000e8e8k";//000000 - состояние рокировок, e8e8k - последний ход

    vector<tuple<int, int, int, int>> moves;//вектор з коретежів типу інт

    int white_moves = 0, black_moves = 0;//счётчики шагов игроков

  public:

    void clear_desk() {//очистка доски
        for (int i = 1; i <= 8; ++i) {
            for (int j = 1; j <= 8; ++j) {
                desk[i][j].figure = EMPTY; //во всех клетках установка пустого состояния
            }
        }
    }
    void clear_desk_state() {//состояние доски анулируется
        for (int i = 1; i <= 8; ++i) {
            for (int j = 1; j <= 8; ++j) {
                desk[i][j].attacked = 0;//нет атак на короля
                desk[i][j].special = 0;//нет специальных ходов
            }
        }
    }
    void init_desk() {//инициализация доски
        desk = new cell_t* [10];//выделение памяти под указатели на структуру
        for (int i = 0; i < 10; ++i) {
            desk[i] = new cell_t [10];//выделение помяти под доску
        }
        for (int i = 0; i < 10; ++i) {//назначение границ доски
            desk[i][0].figure = BARRIER;
            desk[i][9].figure = BARRIER;
            desk[0][i].figure = BARRIER;
            desk[9][i].figure = BARRIER;
        }
        clear_desk();//очистка доски
        clear_desk_state();//очистка состояния доски
    }
    void clear_move_table() {//очистка строк ходов
        for (int i = 0; i < 6; ++i) {
            moveptr_list[i] = 0;
        }
    }
    void init_move_table() {//инициализация доски ходов
        move_table = new char* [6];//выделение памяти под 6 строк
        for (int i = 0; i < 6; ++i)
            move_table[i] = new char[1024];//выделение памяти под масив с ходами
        clear_move_table();//очистка доски ходов
    }
    void destroy_move_table() {//очистить доски ходов
        for (int i = 0; i < 6; ++i) {
            delete move_table[i];//очистка масива с ходами
        }
        delete move_table;
    }
    void add_move(int fromX, int fromY, int toX, int toY) {//функция добавления ходов
        char *move_list = move_table[desk[fromX][fromY].figure - 1];//запись ходов в список
        int moveptr = moveptr_list[desk[fromX][fromY].figure - 1];///////////////////////////////
        move_list[moveptr++] = fromX + 'a' - 1;//запись в список ходов с конвертацией в char
        move_list[moveptr++] = fromY + '0';
        move_list[moveptr++] = toX + 'a' - 1;
        move_list[moveptr++] = toY + '0';
        moveptr_list[desk[fromX][fromY].figure - 1] = moveptr;
    }
    int is_any_move_exist() {//проверка есть ли ходы
        for (int i = 0; i < 6; ++i) {
            if (moveptr_list[i])//если в строке что-то есть тогда ход существует
                return 1;
        }
        return 0;
    }
    void set_current_move(int fromX, int fromY, int toX, int toY, int figure) {//установка текущего хода
        current_move.fromX = fromX;
        current_move.fromY = fromY;
        current_move.toX = toX;
        current_move.toY = toY;
        current_move.figure = figure;
    }
    void release_current_move() {//осуществление выбраного хода
        int fromX = current_move.fromX,//запись координат текущей позиции в переменые
            fromY = current_move.fromY,
            toX = current_move.toX,
            toY = current_move.toY,
            figure = current_move.figure;//запись текущей фигуры в переменую, которая сделала ход
        side = !desk[fromX][fromY].color;//переход на другую сторону (с белой на чёрную,или на оборот)
        if (fromX == toX && fromY == toY) {//если выбран ход н атекущую позицию
            return;
        }
        update_castling();//рокировка
        extra_check();//взятие на проходе

        if (desk[fromX][fromY].figure == KING && abs(fromX - toX) > 1) {//при рокировке
            int _fromX = fromX < toX ? 8 : 1;//если fromX < toX тогда рокировка с ладьей h1(h8), если нет, то с - a1(а8)
            int _toX = (fromX + toX) / 2;//координата ладьи после рокировки
            desk[_toX][toY] = desk[_fromX][fromY];//передвижение ладьи
            desk[_fromX][fromY].figure = EMPTY;//место ладьи стает пустым
        }
        if (desk[fromX][fromY].figure == PAWN) {//если ходит пешка
            if (toY == (side ? 1 : 8)) {//определяем напрвление, если белая 1 линия, если чёрная - 8 линия
                desk[fromX][fromY].figure = figure - 1;
            }
            if (fromX != toX && desk[toX][toY].figure == EMPTY) {//замена при взятии на проходе
                desk[toX][fromY].figure = EMPTY;
            }
        }
        desk[toX][toY] = desk[fromX][fromY];//передвижение
        desk[fromX][fromY].figure = EMPTY;//клетка, с которой походила становиться пустой
        side ? (++white_moves) : (++black_moves);
    }
    void update_castling() {//рокировка
        int fromX = current_move.fromX,//запись координат текущей позиции в переменные
            fromY = current_move.fromY,
            toX = current_move.toX,
            toY = current_move.toY;
        for (int i = 1; i <= 8; i += 7) {//проход по 1 и 8 ряду
            for (int j = 1; j <= 8; j += 3 + (j == 1)) { //позиции 1, 4 и 8: [# . . . # . . #]
                castling_active[i < 8][j / 4] |= (fromX == j && fromY == i);//
                castling_active[i < 8][j / 4] |= (toX == j && toY == i);
            }
        }
    }
    int extra_figure(const cell_t *cell) {
        if (cell->figure == EMPTY) {//если клетка пустая
            return 0;
        }//проверяем дальше

        if ((cell->color == side && cell->figure == KING) ||
                (cell->color != side && (cell->figure == ROOK || cell->figure == QUEEN))) {
            return cell->figure;//взятие на проходе(возврат клетки с фигурой на ней)
        } else {//
            return -1;//возврат ошибки в случае несоответствия условию
        }
    }
    void extra_check() {//проверка можно ли осуществить взятие на проходе
        int fromX = current_move.fromX,//запись координат
            fromY = current_move.fromY,
            toX = current_move.toX,//ход который может быть осуществлен на 2 клетки
            toY = current_move.toY;
        extra.exist = 0;

        //проверка правильности хода
        //проверка существует ли ход для взятия на проходе: fromY - toY = 2
        if (desk[fromX][fromY].figure != PAWN || abs(fromY - toY) < 2) {
            return;//если фигура, которой ходят не пешка или ход не на две клетки вперед
        }
        int X, Y;//координаты пешки, которая может осуществить взятие на проходе
        //если был ход пешкой на 2 вперед и клетка, через которую перелетели соответствует
        //цвету фигуры, какая ходит

        //проверка влево на наличие вражеской пешки(ход белых - белой), которая может взять на проходе
        if (desk[toX - 1][toY].figure == PAWN && desk[toX - 1][toY].color == side) {
            X = toX - 1;//стает ниже на одну чем битая пешка
            Y = toY;
            //проверка вправо на наличие вражеской пешки, которая может взять на проходе
        } else if (desk[toX + 1][toY].figure == PAWN && desk[toX + 1][toY].color == side) {
            X = toX + 1;//стает выше на одну чем битая пешка
            Y = toY;
        } else {//если пешки нет, взятия на проходе не будет
            return;
        }

        extra.exist = 1;//взятие на проходе может быть осуществлено: есть пешка,к оторая может это сделать
        extra.X = toX;//координаты на которые станет пешка при взятии
        extra.Y = toY + (side ? 1 : -1);//опредиление направления хода, если белые + 1, ход вверх по доске
        int dec = 0, inc = 0;

        for (int i = X - 1; i > 0; --i) {//проверка границы для черной пешки
            int t = extra_figure(&desk[i][toY]);//передаем переменной клетку с фигурой
            //если t = 0 проверяем дальше
            if (t > 0) {//если есть фигура
                dec = t;//запоминание фигуры
                break;//выход из условия
            } else if (t < 0) {//не проходит проверку
                return;//ничего не возвращаем
            }
        }
        for (int i = X + 1; i <= 8; ++i) {//проверка границы для белой пешки
            int t = extra_figure(&desk[i][toY]);//передаем переменной клетку с фигурой
            if(t > 0) {//если есть фигура
                inc = t;//запоминание фигуры
                break;//выход из условия
            } else if (t < 0) {//не проходит проверку
                return;//ничего не возвращаем
            }
        }
        if (inc && dec && (dec == KING || inc == KING)) {//если стоит король, то невозможно осуществить из-за шаха
            extra.exist = 0;//скидываем на default значение взятия
        }
    }
    char get_figure_symbol(int figure) {//конвертация enum в char
        return (figure == PAWN) * 'p' + (figure == QUEEN) * 'q' + (figure == KING) * 'k'
               + (figure == KNIGHT) * 'g' + (figure == BISHOP) * 'b' + (figure == ROOK) * 'r'
               + (figure == EMPTY) * '.';
    }
    int get_figure_number(char figure) {//конвертация char в enum
        figure = tolower(figure);
        return (figure == 'p') * PAWN + (figure == 'q') * QUEEN + (figure == 'k') * KING
               + (figure == 'g') * KNIGHT + (figure == 'b') * BISHOP + (figure == 'r') * ROOK
               + (figure == '.') * EMPTY;
    }
    void unpack_desk(const char *position) {//распаковка доски
        init_desk();//инициализация доски: выделение памяти, выставление границ
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                const char c = position[i * 8 + j];//получение фигур(символов) из строки задающей начальние состояние
                //инициализация структуры для каждой клетки
                desk[j + 1][i + 1] = (cell_t) {
                    static_cast<char>(get_figure_number(c)), 0, (c >= 'a' && c <= 'z'), 0, 0
                };
            }
        }
    }
    void unpack_castling(const char *position) {//задание начального состояни рокировок 000000
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 3; ++j) {
                castling_active[i][j] = position[i * 3 + j] - '0';//начальное состояние 000000
            }
        }
    }
    void unpack_move(const char *position) {//установка первого хода, в данном случае e8e8k - то есть ход не осуществлесться
        set_current_move(position[0] - 'a' + 1, position[1] - '0',
                         position[2] - 'a' + 1, position[3] - '0',
                         get_figure_number(position[4]));
    }
    void unpack_position(const char *position) {//установка начальных позиций доски
        unpack_desk(position);//установка фигур
        unpack_move(position + 70);//все фигуры и состояние рокировок
        unpack_castling(position + 64);
    }
    int atacks_on_king = 0;//король не атакован

    int get_mobility(int dx, int dy) {//определяем направление хода
        return abs(dx) == abs(dy) ? (dx == dy ? ANGLE45 : ANGLE135) : (dx ? ANGLE0 : ANGLE90);
    }
    int is_check() {//король атакован
        return atacks_on_king == 1;
    }
    int is_multicheck() {//елси не сколько фигур одновременно атакуют короля
        return atacks_on_king > 1;
    }
    int is_correct(int x, int y) {//проверка на выход за границы доски
        return x > 0 && x < 9 && y > 0 && y < 9;
    }
    int is_figure(const cell_t *cell) {//проверка существуует ли фугура
        return cell->figure != EMPTY && cell->figure != BARRIER;//не является ли пустой клеткой или границой
    }
    int is_ally(const cell_t *cell) {//есть ли союзной фигурой
        return is_figure(cell) && cell->color == side;//существует ли фигура,соответстивует
    }
    int is_enemy(const cell_t *cell) {//если фигура существует и ее цвет не соответсвует цвету
        return is_figure(cell) && cell->color != side;//фигуры которая ходит
    }
    int is_direction_correct(const cell_t *cell, int dx, int dy) {//проверка на правильность направления хода
        //если фигура на клетке не король, если направление правильное, клетка не под связкой
        return (cell->figure != KNIGHT && get_mobility(dx, dy) == cell->mobility) || !cell->special;
    }
    int is_moveable(int x, int y, int dx, int dy) {//проверка на правильность хода, можно ли походить
        if (!is_correct(x + dx, y + dy) || is_ally(&desk[x + dx][y + dy]) || !is_direction_correct(&desk[x][y], dx, dy))
            return 0;//если выход за границы, союзная фигура или неправильное направление ход невозможен
        return !is_check() || desk[x + dx][y + dy].special;//проверка не есть есть ли шах, либо защита од шаха
    }
    void atack_handler(int x, int y, int dx, int dy) {//генерация всех ходов, проверка каждой клетки, на которую может походить вражеская фигура
        if (is_correct(x + dx, y + dy)) {//проверка правильности передвижения
            cell_t *cell = &desk[x + dx][y + dy];//указатель на клетку перемещения
            cell->attacked = 1;//клетка на которой вражеская фигура может быть атакована(король может получить шах)
            if (is_ally(cell) && cell->figure == KING) {//если на клетке стоит король, то он атакован
                desk[x][y].special = 1;//клетка с которой идет атака
                ++atacks_on_king;
            }
        }
    }
    void pierce_block(int x, int y, int dx, int dy) {//блокировка ходов для клеток под связкой+
        desk[x][y].special = 1;//фигура на клетке - фигура под связкой
        desk[x][y].mobility = get_mobility(dx, dy);//вектор, котором фигура под связкой
    }
    void ally_line_handler(int x, int y, int dx, int dy) {//генерация ходов, которые могут быть побиты для линии союзных фигур и клеток под их прикрытием
        if(!is_direction_correct(&desk[x][y], dx, dy))//если направление неправильное
            return;
        for(int i = dx, j = dy; is_correct(x + i, y + j); i += dx, j += dy) {
            if(is_moveable(x, y, i, j)) {
                add_move(x, y, x + i, y + j);//передвинуться на позицию
            }
            if(is_figure(&desk[x + i][y + j])) { //если натыкнулось на свою фигуру
                break;
            }
        }
    }
    int pierce_detector(int x, int y, int dx, int dy, int cnt) {//функция блокировки невозможных ходов
        cell_t *cell = &desk[x + dx][y + dy];//указатель на клетку
        if (!cnt) {//cnt = count
            atack_handler(x, y, dx, dy);//если нет шаха, генерировать ходы
        }
        if (cell->figure == EMPTY) {//если клетка пустая продолжаем проверять в этом направлении
            return cnt;//сnt = 0
        }
        ++cnt;
        cnt *= (cell->figure == KING ? -1 : 1);//если atack_handler обнаружил шах
        if (cell->figure == BARRIER || is_enemy(cell) || cnt == 2) {//если вражеская натыкаеться на граниу или свою фигуру
            return -3;//если натыкается на границу или враескую фигуру, или
        } else { //могут взаимно побить друг друга(2), шах
            return cnt;
        }
    }
    void enemy_line_handler(int x, int y, int dx, int dy) {//обработчик возможных ходов вражеских фигур
        int cnt = 0, i = 0, j = 0;
        do {//перебор всех возможных ходов, для нахождения клеток, где
            i += dx;
            j += dy;;
            cnt = pierce_detector(x, y, i, j, cnt);
        } while (cnt >= 0);
        if (cnt == -1) {//при шахе
            atack_handler(x, y, i + dx, j + dy);
            for (i = x, j = y; desk[i][j].figure != KING; i += dx, j += dy) {//проверка на вражеского короля
                desk[i][j].special = 1;//клетки стают под связкой
            }
        } else if (cnt == -2) {//побить друг друга
            for (i = x, j = y; !is_ally(&desk[i][j]); i += dx, j += dy);
            pierce_block(i, j, dx, dy);//
        }
    }
    void X_handler(int x, int y, int vh, int d, line_handler_t handl) {//проверка направления
        if (vh) {//если ход по горизонтале или вертикали
            (this->*handl)(x, y, 1, 0);//враво
            (this->*handl)(x, y, -1, 0);//влево
            (this->*handl)(x, y, 0, 1);//вверх
            (this->*handl)(x, y, 0, -1);//вниз
        }
        if (d) {//если ход по горизонтали
            (this->*handl)(x, y, 1, 1);//вверх-вправо
            (this->*handl)(x, y, 1, -1);//вниз-вправо
            (this->*handl)(x, y, -1, 1);//влево-вверх
            (this->*handl)(x, y, -1, -1);//влево-вниз
        }
    }
    void ally_knight_handler(int x, int y) {//генерация ходов союзного коня
        for(int i = 1; i <= 2; ++i) {//проход на 2 линии по х
            int j = 3 - i;//позиция по у
            if(is_moveable(x, y, i, j)) {//если ход возможен, то сделать ход
                add_move(x, y, x + i, y + j);//ход вверх и вправо
            }
            if(is_moveable(x, y, i, -j)) {
                add_move(x, y, x + i, y - j);//ход вправо и вниз
            }
            if(is_moveable(x, y, -i, j)) {
                add_move(x, y, x - i, y + j);//ход влево и вверх
            }
            if(is_moveable(x, y, -i, -j)) {
                add_move(x, y, x - i, y - j);//ход влево и вниз
            }
        }
    }
    void enemy_knight_handler(int x, int y) {//вражеский король
        for (int i = 1; i <= 2; ++i) {//проход по 2 рядам, координата по х
            int j = 3 - i;//координата по у
            atack_handler(x, y, i, j);
            atack_handler(x, y, i, -j);
            atack_handler(x, y, -i, j);
            atack_handler(x, y, -i, -j);
        }
    }
    void ally_king_handler(int x, int y) {//генерация ходов для союзного короля
        for (int i = -1; i <= 1; ++i) {//во всех направлениях по горизонтали
            for (int j = -1; j <= 1; ++j) {//во всех направлениях по вертикали
                if (is_correct(x + i, y + j) && !desk[x + i][y + j].attacked &&
                        !is_ally(&desk[x + i][y + j])) {
                    add_move(x, y, x + i, y + j);
                }
            }
        }
    }
    void enemy_king_handler(int x, int y) {//генерация возможных ходов для вражеского короля
        for (int i = -1; i <= 1; ++i) {//во всех направлениях по горизонтали
            for (int j = -1; j <= 1; ++j) {//во всех направлениях по вертикали
                if (desk[x + i][y + j].figure != KING) {
                    atack_handler(x, y, i, j);
                }
            }
        }
    }
    void pawn_kill_move(int x, int y, int dx, int dy) {//генератор ходов, при которых пешка может побить вражескую фигуру: f2->g3, f2->e3
        if ((is_enemy(&desk[x + dx][y + dy]) && is_moveable(x, y, dx, dy))
                || (extra.exist && x + dx == extra.X && y + dy == extra.Y
                    && (is_moveable(x, y, dx, dy) || is_moveable(x, y, dx, 0)))) {
            add_move(x, y, x + dx, y + dy);
        }
    }
    void ally_pawn_handler(int x, int y) {//генерация ходов для союзной пешки
        int t = (side ? 1 : -1);
        //t = 1 - белые, t = -1 - чёрные, определяет направление, если белые то вверх, если чёрные, то вниз по доске
        pawn_kill_move(x, y, 1, t);
        pawn_kill_move(x, y, -1, t);

        if (is_figure(&desk[x][y + t])) {//невозможно походить на клетку, где уже есть фигура
            return;
        }
        if (is_moveable(x, y, 0, t)) {//генерация хода на одну клетку вверх/вниз
            add_move(x, y, x, y + t);
        }
        //генерация хода на 2 клетки вверх/вниз для первого хода пешки
        if (y == 7 - 5 * side && !is_figure(&desk[x][y + 2 * t]) && is_moveable(x, y, 0, 2 * t)) {
            add_move(x, y, x, y + 2 * t);
        }
    }
    void enemy_pawn_handler(int x, int y) {//генерация ходов, которые может сделать вражеская пешка, клеток, которые она может побить
        //для черных side = -1, белых 1, если ход белых, генерация хода вниз, для чёрных
        atack_handler(x, y, 1, -(side ? 1 : -1));//генерация хода вправо
        atack_handler(x, y, -1, -(side ? 1 : -1));//генерация хода влево
    }
    void ally_rook_handler(int x, int y) {//проверка союзной ладьи
        X_handler(x, y, 1, 0, ally_line_handler);//проверка идет по горизонтали и вертикали
    }
    void enemy_rook_handler(int x, int y) {//проверка вражеской ладьи
        X_handler(x, y, 1, 0, enemy_line_handler);//проверка идет по горизонтали и вертикали
    }
    void ally_bishop_handler(int x, int y) {//проверка союзного слона
        X_handler(x, y, 0, 1, ally_line_handler);//проверка идет по диагоналям
    }
    void enemy_bishop_handler(int x, int y) {//проверка вражеского слона
        X_handler(x, y, 0, 1, enemy_line_handler);//проверка идет по диагоналям
    }

    void ally_queen_handler(int x, int y) {//проверка союзной королевы
        X_handler(x, y, 1, 1, ally_line_handler);//проверка идет и по горизонтали/вертикали и по диагоналям
    }
    void enemy_queen_handler(int x, int y) {//проверка вражеской королевы
        X_handler(x, y, 1, 1, enemy_line_handler);//проверка идет и по горизонтали/вертикали и по диагоналям
    }
    void ally_figure_handler(int x, int y) {//поиск фигуры и опредиление генератора ходов для фигур
        const static cell_handler_t handlers[] = {//статичиский массив с функциями генерации ходов фигур
            ally_pawn_handler, ally_queen_handler,
            ally_king_handler, ally_knight_handler,
            ally_bishop_handler, ally_rook_handler
        };
        if(!is_figure(&desk[x][y]) || is_enemy(&desk[x][y])//выключить все клетки не с союзными фигурами
                || (is_multicheck() && desk[x][y].figure != KING)) {//
            return;//если на клетке не стоит союзная фигура, либо стоит вражеская, либо когда королю поставлен мультишах
        }
        (this->*handlers[desk[x][y].figure - 1])(x, y);
    }
    void enemy_figure_handler(int x, int y) {//функция выхова обработчиков
        const static cell_handler_t handlers[] = {
            enemy_pawn_handler, enemy_queen_handler,
            enemy_king_handler, enemy_knight_handler,
            enemy_bishop_handler, enemy_rook_handler
        };
        //если фигура не союзная для вражеской или пустая клетка
        if (!is_figure(&desk[x][y]) || is_ally(&desk[x][y])) {
            return;
        }
        (this->*handlers[desk[x][y].figure - 1])(x, y);//
    }
    void desk_handler(cell_handler_t handl) {//обработчик доски
        for (int i = 1; i <= 8; ++i) {
            for (int j = 1; j <= 8; ++j) {
                (this->*handl)(i, j);
            }
        }
    }
    void castle_handler() {//обработчик рокировок
        if (is_check() || is_multicheck() || castling_active[side][1]) {//
            return;//если осуществлен шах,
        }
        int sideline = 8 - side * 7;//первая линия или восьмая
        //если король не атакован и стоит на клетке, если клетка на которую может стать
        //король не атакован
        if (!castling_active[side][0]
                && !desk[4][sideline].attacked && !is_figure(&desk[4][sideline])
                && !desk[3][sideline].attacked && !is_figure(&desk[3][sideline])) {
            add_move(5, sideline, 3, sideline);//ход e1->c1, e8->c8
        }
        //если клетка, на которую перевигаеться король не атакована и там нет фигуры
        if (!castling_active[side][2] && !desk[6][sideline].attacked && !is_figure(&desk[6][sideline])
                && !desk[7][sideline].attacked && !is_figure(&desk[7][sideline])) {
            add_move(5, sideline, 7, sideline);//ход e1->g1, e8->g8
        }
    }
    void position_handler() {//обработчик состояний
        atacks_on_king = 0;//нет атак на короля
        clear_move_table();//очистка доски ходов
        clear_desk_state();//очистка доски состояний
        release_current_move();//осуществление хода
        desk_handler(enemy_figure_handler);//обработка вражеской стороны
        desk_handler(ally_figure_handler);//обработка союзной стороны
        castle_handler();//обработчик рокировок
    }
    void update_moves() {//функция записи ходов
        moves.clear();//очистка таблицы с ранее сгенерироваными ходами
        for (int i = 0; i < 6; ++i) {//6 - количевство фигур, размер доски с ходами
            for (int j = 0; j < moveptr_list[i]; j += 4) {//////////////////////////////////
                moves.push_back(make_tuple(
                                    move_table[i][j] - 'a' + 1,
                                    move_table[i][j + 1] - '0',
                                    move_table[i][j + 2] - 'a' + 1,
                                    move_table[i][j + 3] - '0'
                                ));//запись обьектов кортежей в вектор
            }
        }
        destroy_move_table();//очистка доски ходов
        init_move_table();//инициализация доски ходов
    }
    void output_desk() {//функция вывода доски
        system("cls");
        cout << "\n\t\t\t\t        Chess game\n\n";
        cout << "\t\t\t  ----------------------------------------" << endl;
        for (int i = 8; i >= 1; --i) {
            cout << "\t\t\t" << i << " | ";
            for (int j = 1; j <= 8; ++j) {
                cout << static_cast<char>(get_figure_symbol(desk[j][i].figure) +
                        (desk[j][i].color || desk[j][i].figure == EMPTY ? 0 : 'A' - 'a'));
                if (j < 8)
                    cout << " || ";
            }
            cout << " |\n\t\t\t  ----------------------------------------\n";
        }
        cout << " \t\t\t    a    b    c    d    e    f    g    h   \n\n";
    }


    clock_t start;

    Chess() {//коструктор
        init_move_table();//инициализация доски ходов
        unpack_position(starting);//распаковка первого хода
        position_handler();//обработчик позиций
        update_moves();//запись ходов
    }
    void start_timer() {//начало отсчета времени
        start = clock();
    }
    void game_info() {
        double time = (clock() - start) / CLOCKS_PER_SEC / 60.;
        cout << "\n\t\t\t\t\t" << (side ? "White won\n" : "Black won\n");
        cout << "\t\t\tTime of the game: " << (int)time << " minutes, " << setprecision(2)<< (time - (int)time)*100 << " seconds\n";
        cout << "\t\t\t\tTotal moves: " << white_moves + black_moves << endl;
        cout << "\t\t\tWhite_moves -- " << white_moves << ", black_moves -- " << black_moves << endl;
    }
    bool turn(int number) {//функция осуществления хода
        //проверка выбора правильного хода, вызов обработчика
        if (number < 0 || number >= moves.size()) {//проверка корректности введеного номера
            return false;
        }
        //распаковка, возвращение значений кортежа, лежащего за номером в векторе и запись в значения ходов
        tie(current_move.fromX, current_move.fromY, current_move.toX, current_move.toY) = moves[number];
        position_handler();//генерация для измененной доски и позиций
        update_moves();
        return true;
    }
    void play() {//функция начала игры, вывод доски ивозможных ходов
        start_timer();
        while (!moves.empty()) {//пока не будут выведены все хвозможные ходы
            output_desk();//вывод доски
            sort(moves.begin(), moves.end());
            cout << setw(52) << (side ? "White's turn\n" : "Black's turn\n");
            cout << setw(54) << "Possible moves\n\n";
            for (int i = 0; i < moves.size(); ++i) {
                if (i % 2 == 0) cout << "\t\t\t\t";
                cout << setw(2) << i + 1 << ". " << char(get<0>(moves[i]) + 96) << get<1>(moves[i]) << "->"
                     << char(get<2>(moves[i]) + 96) << get<3>(moves[i]) << "   ";
                if(i % 2 != 0)  cout << endl;
            }
            cout << "\n\t\t\t\t\t   ";
            int number;
            cin >> number;//ввод номеры хода
            turn(number - 1);
        }
        game_info();
    }
    void setup() {//меню игры
        string answer;//ответ пользователя
        cout << "\n\t\t\t\t\tWelcome to Chess game!\n\n";
        cout << setw(65) << "Choose option and enter:\n";
        cout << setw(73) << "<play>, <rules>, <exit>, <again>\n\t\t\t\t\t\t";
        cin >> answer;
        if (answer == "play") {
            system("cls");
            play();
        } else if(answer == "rules") {
            system("cls");
            Rules();
        } else if (answer == "again") {
            system("cls");
            play();
        } else if (answer == "exit") {
            system("cls");
            cout << "\n\n\t\t\t\tThank you for the game! Hope to see you later.\n\n";
            exit(0);
        } else {
            cout << "\n\n\t\t\t\tYour answer is incorrect.\n";
        }
    }
};

class ChessGame {
  public:
    void StartGame() {
        while(true) {
            Chess game;
            game.setup();
        }
    }
};

int main() {

    ChessGame game;
    game.StartGame();

    return 0;
}
