#include<bits/stdc++.h>
#include "rules.h"
using namespace std;

class Chess {//�������� ����� ����
  private:
    enum {
        EMPTY,
        PAWN, QUEEN, KING,
        KNIGHT, BISHOP, ROOK,
        BARRIER
    };//enumeration ����� � ����� � ������ ������
    enum {
        ANGLE0,//�����, ����
        ANGLE45,//���� 45 ��������
        ANGLE90,//������, �����
        ANGLE135//���� 135 ��������
    };//enumeration �������� �����������

    typedef struct {
        char figure;//���������� ��� �������� �������� ������
        char mobility;//����������� ������������
        char color;//���� ������
        char attacked;//������ ��� ������
        char special;//���� ��� �������
    } cell_t;//���������� ���������

    cell_t **desk;//��������� ������ �� ���������
    char **move_table;//����� �����
    int moveptr_list[6] = { 0, 0, 0, 0, 0, 0 };//����� ����� � �������� ������ ��� ������ ������
    struct {
        int fromX, fromY, toX, toY, figure;//���������� ���� � ������, ������� ��� ��������� ���
    } current_move;//��������� �������� ���������� �������� ����
    int side;//1 - ����� ��� 0 - ������
    int castling_active[2][3];//����� ��� ���������� ���������
    //2*3 = 6 �����, ������� ����� ���������� � ���������: 2 ������ � 4 �����

    struct {
        int exist, X, Y;
    } extra;//��������� ��� ������ �� �������

    typedef void (Chess::*line_handler_t)(int, int, int, int);//��������� �� ������� ��������� �����
    typedef void(Chess::*cell_handler_t)(int, int);//��������� �� ������� ������������

    //��������� ��������� ������� �����, ��������� ��������� � ������� ����
    const char *starting =
        "rgbqkbgr"//white figures
        "pppppppp"
        "........"
        "........"
        "........"
        "........"
        "PPPPPPPP"
        "RGBQKBGR"//black figures
        "000000e8e8k";//000000 - ��������� ���������, e8e8k - ��������� ���

    vector<tuple<int, int, int, int>> moves;//������ � �������� ���� ���

    int white_moves = 0, black_moves = 0;//�������� ����� �������

  public:

    void clear_desk() {//������� �����
        for (int i = 1; i <= 8; ++i) {
            for (int j = 1; j <= 8; ++j) {
                desk[i][j].figure = EMPTY; //�� ���� ������� ��������� ������� ���������
            }
        }
    }
    void clear_desk_state() {//��������� ����� �����������
        for (int i = 1; i <= 8; ++i) {
            for (int j = 1; j <= 8; ++j) {
                desk[i][j].attacked = 0;//��� ���� �� ������
                desk[i][j].special = 0;//��� ����������� �����
            }
        }
    }
    void init_desk() {//������������� �����
        desk = new cell_t* [10];//��������� ������ ��� ��������� �� ���������
        for (int i = 0; i < 10; ++i) {
            desk[i] = new cell_t [10];//��������� ������ ��� �����
        }
        for (int i = 0; i < 10; ++i) {//���������� ������ �����
            desk[i][0].figure = BARRIER;
            desk[i][9].figure = BARRIER;
            desk[0][i].figure = BARRIER;
            desk[9][i].figure = BARRIER;
        }
        clear_desk();//������� �����
        clear_desk_state();//������� ��������� �����
    }
    void clear_move_table() {//������� ����� �����
        for (int i = 0; i < 6; ++i) {
            moveptr_list[i] = 0;
        }
    }
    void init_move_table() {//������������� ����� �����
        move_table = new char* [6];//��������� ������ ��� 6 �����
        for (int i = 0; i < 6; ++i)
            move_table[i] = new char[1024];//��������� ������ ��� ����� � ������
        clear_move_table();//������� ����� �����
    }
    void destroy_move_table() {//�������� ����� �����
        for (int i = 0; i < 6; ++i) {
            delete move_table[i];//������� ������ � ������
        }
        delete move_table;
    }
    void add_move(int fromX, int fromY, int toX, int toY) {//������� ���������� �����
        char *move_list = move_table[desk[fromX][fromY].figure - 1];//������ ����� � ������
        int moveptr = moveptr_list[desk[fromX][fromY].figure - 1];///////////////////////////////
        move_list[moveptr++] = fromX + 'a' - 1;//������ � ������ ����� � ������������ � char
        move_list[moveptr++] = fromY + '0';
        move_list[moveptr++] = toX + 'a' - 1;
        move_list[moveptr++] = toY + '0';
        moveptr_list[desk[fromX][fromY].figure - 1] = moveptr;
    }
    int is_any_move_exist() {//�������� ���� �� ����
        for (int i = 0; i < 6; ++i) {
            if (moveptr_list[i])//���� � ������ ���-�� ���� ����� ��� ����������
                return 1;
        }
        return 0;
    }
    void set_current_move(int fromX, int fromY, int toX, int toY, int figure) {//��������� �������� ����
        current_move.fromX = fromX;
        current_move.fromY = fromY;
        current_move.toX = toX;
        current_move.toY = toY;
        current_move.figure = figure;
    }
    void release_current_move() {//������������� ��������� ����
        int fromX = current_move.fromX,//������ ��������� ������� ������� � ���������
            fromY = current_move.fromY,
            toX = current_move.toX,
            toY = current_move.toY,
            figure = current_move.figure;//������ ������� ������ � ���������, ������� ������� ���
        side = !desk[fromX][fromY].color;//������� �� ������ ������� (� ����� �� ������,��� �� ������)
        if (fromX == toX && fromY == toY) {//���� ������ ��� � �������� �������
            return;
        }
        update_castling();//���������
        extra_check();//������ �� �������

        if (desk[fromX][fromY].figure == KING && abs(fromX - toX) > 1) {//��� ���������
            int _fromX = fromX < toX ? 8 : 1;//���� fromX < toX ����� ��������� � ������ h1(h8), ���� ���, �� � - a1(�8)
            int _toX = (fromX + toX) / 2;//���������� ����� ����� ���������
            desk[_toX][toY] = desk[_fromX][fromY];//������������ �����
            desk[_fromX][fromY].figure = EMPTY;//����� ����� ����� ������
        }
        if (desk[fromX][fromY].figure == PAWN) {//���� ����� �����
            if (toY == (side ? 1 : 8)) {//���������� ����������, ���� ����� 1 �����, ���� ������ - 8 �����
                desk[fromX][fromY].figure = figure - 1;
            }
            if (fromX != toX && desk[toX][toY].figure == EMPTY) {//������ ��� ������ �� �������
                desk[toX][fromY].figure = EMPTY;
            }
        }
        desk[toX][toY] = desk[fromX][fromY];//������������
        desk[fromX][fromY].figure = EMPTY;//������, � ������� �������� ����������� ������
        side ? (++white_moves) : (++black_moves);
    }
    void update_castling() {//���������
        int fromX = current_move.fromX,//������ ��������� ������� ������� � ����������
            fromY = current_move.fromY,
            toX = current_move.toX,
            toY = current_move.toY;
        for (int i = 1; i <= 8; i += 7) {//������ �� 1 � 8 ����
            for (int j = 1; j <= 8; j += 3 + (j == 1)) { //������� 1, 4 � 8: [# . . . # . . #]
                castling_active[i < 8][j / 4] |= (fromX == j && fromY == i);//
                castling_active[i < 8][j / 4] |= (toX == j && toY == i);
            }
        }
    }
    int extra_figure(const cell_t *cell) {
        if (cell->figure == EMPTY) {//���� ������ ������
            return 0;
        }//��������� ������

        if ((cell->color == side && cell->figure == KING) ||
                (cell->color != side && (cell->figure == ROOK || cell->figure == QUEEN))) {
            return cell->figure;//������ �� �������(������� ������ � ������� �� ���)
        } else {//
            return -1;//������� ������ � ������ �������������� �������
        }
    }
    void extra_check() {//�������� ����� �� ����������� ������ �� �������
        int fromX = current_move.fromX,//������ ���������
            fromY = current_move.fromY,
            toX = current_move.toX,//��� ������� ����� ���� ����������� �� 2 ������
            toY = current_move.toY;
        extra.exist = 0;

        //�������� ������������ ����
        //�������� ���������� �� ��� ��� ������ �� �������: fromY - toY = 2
        if (desk[fromX][fromY].figure != PAWN || abs(fromY - toY) < 2) {
            return;//���� ������, ������� ����� �� ����� ��� ��� �� �� ��� ������ ������
        }
        int X, Y;//���������� �����, ������� ����� ����������� ������ �� �������
        //���� ��� ��� ������ �� 2 ������ � ������, ����� ������� ���������� �������������
        //����� ������, ����� �����

        //�������� ����� �� ������� ��������� �����(��� ����� - �����), ������� ����� ����� �� �������
        if (desk[toX - 1][toY].figure == PAWN && desk[toX - 1][toY].color == side) {
            X = toX - 1;//����� ���� �� ���� ��� ����� �����
            Y = toY;
            //�������� ������ �� ������� ��������� �����, ������� ����� ����� �� �������
        } else if (desk[toX + 1][toY].figure == PAWN && desk[toX + 1][toY].color == side) {
            X = toX + 1;//����� ���� �� ���� ��� ����� �����
            Y = toY;
        } else {//���� ����� ���, ������ �� ������� �� �����
            return;
        }

        extra.exist = 1;//������ �� ������� ����� ���� ������������: ���� �����,� ������ ����� ��� �������
        extra.X = toX;//���������� �� ������� ������ ����� ��� ������
        extra.Y = toY + (side ? 1 : -1);//����������� ����������� ����, ���� ����� + 1, ��� ����� �� �����
        int dec = 0, inc = 0;

        for (int i = X - 1; i > 0; --i) {//�������� ������� ��� ������ �����
            int t = extra_figure(&desk[i][toY]);//�������� ���������� ������ � �������
            //���� t = 0 ��������� ������
            if (t > 0) {//���� ���� ������
                dec = t;//����������� ������
                break;//����� �� �������
            } else if (t < 0) {//�� �������� ��������
                return;//������ �� ����������
            }
        }
        for (int i = X + 1; i <= 8; ++i) {//�������� ������� ��� ����� �����
            int t = extra_figure(&desk[i][toY]);//�������� ���������� ������ � �������
            if(t > 0) {//���� ���� ������
                inc = t;//����������� ������
                break;//����� �� �������
            } else if (t < 0) {//�� �������� ��������
                return;//������ �� ����������
            }
        }
        if (inc && dec && (dec == KING || inc == KING)) {//���� ����� ������, �� ���������� ����������� ��-�� ����
            extra.exist = 0;//��������� �� default �������� ������
        }
    }
    char get_figure_symbol(int figure) {//����������� enum � char
        return (figure == PAWN) * 'p' + (figure == QUEEN) * 'q' + (figure == KING) * 'k'
               + (figure == KNIGHT) * 'g' + (figure == BISHOP) * 'b' + (figure == ROOK) * 'r'
               + (figure == EMPTY) * '.';
    }
    int get_figure_number(char figure) {//����������� char � enum
        figure = tolower(figure);
        return (figure == 'p') * PAWN + (figure == 'q') * QUEEN + (figure == 'k') * KING
               + (figure == 'g') * KNIGHT + (figure == 'b') * BISHOP + (figure == 'r') * ROOK
               + (figure == '.') * EMPTY;
    }
    void unpack_desk(const char *position) {//���������� �����
        init_desk();//������������� �����: ��������� ������, ����������� ������
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                const char c = position[i * 8 + j];//��������� �����(��������) �� ������ �������� ��������� ���������
                //������������� ��������� ��� ������ ������
                desk[j + 1][i + 1] = (cell_t) {
                    static_cast<char>(get_figure_number(c)), 0, (c >= 'a' && c <= 'z'), 0, 0
                };
            }
        }
    }
    void unpack_castling(const char *position) {//������� ���������� �������� ��������� 000000
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 3; ++j) {
                castling_active[i][j] = position[i * 3 + j] - '0';//��������� ��������� 000000
            }
        }
    }
    void unpack_move(const char *position) {//��������� ������� ����, � ������ ������ e8e8k - �� ���� ��� �� ���������������
        set_current_move(position[0] - 'a' + 1, position[1] - '0',
                         position[2] - 'a' + 1, position[3] - '0',
                         get_figure_number(position[4]));
    }
    void unpack_position(const char *position) {//��������� ��������� ������� �����
        unpack_desk(position);//��������� �����
        unpack_move(position + 70);//��� ������ � ��������� ���������
        unpack_castling(position + 64);
    }
    int atacks_on_king = 0;//������ �� ��������

    int get_mobility(int dx, int dy) {//���������� ����������� ����
        return abs(dx) == abs(dy) ? (dx == dy ? ANGLE45 : ANGLE135) : (dx ? ANGLE0 : ANGLE90);
    }
    int is_check() {//������ ��������
        return atacks_on_king == 1;
    }
    int is_multicheck() {//���� �� ������� ����� ������������ ������� ������
        return atacks_on_king > 1;
    }
    int is_correct(int x, int y) {//�������� �� ����� �� ������� �����
        return x > 0 && x < 9 && y > 0 && y < 9;
    }
    int is_figure(const cell_t *cell) {//�������� ����������� �� ������
        return cell->figure != EMPTY && cell->figure != BARRIER;//�� �������� �� ������ ������� ��� ��������
    }
    int is_ally(const cell_t *cell) {//���� �� ������� �������
        return is_figure(cell) && cell->color == side;//���������� �� ������,��������������
    }
    int is_enemy(const cell_t *cell) {//���� ������ ���������� � �� ���� �� ������������ �����
        return is_figure(cell) && cell->color != side;//������ ������� �����
    }
    int is_direction_correct(const cell_t *cell, int dx, int dy) {//�������� �� ������������ ����������� ����
        //���� ������ �� ������ �� ������, ���� ����������� ����������, ������ �� ��� �������
        return (cell->figure != KNIGHT && get_mobility(dx, dy) == cell->mobility) || !cell->special;
    }
    int is_moveable(int x, int y, int dx, int dy) {//�������� �� ������������ ����, ����� �� ��������
        if (!is_correct(x + dx, y + dy) || is_ally(&desk[x + dx][y + dy]) || !is_direction_correct(&desk[x][y], dx, dy))
            return 0;//���� ����� �� �������, ������� ������ ��� ������������ ����������� ��� ����������
        return !is_check() || desk[x + dx][y + dy].special;//�������� �� ���� ���� �� ���, ���� ������ �� ����
    }
    void atack_handler(int x, int y, int dx, int dy) {//��������� ���� �����, �������� ������ ������, �� ������� ����� �������� ��������� ������
        if (is_correct(x + dx, y + dy)) {//�������� ������������ ������������
            cell_t *cell = &desk[x + dx][y + dy];//��������� �� ������ �����������
            cell->attacked = 1;//������ �� ������� ��������� ������ ����� ���� ���������(������ ����� �������� ���)
            if (is_ally(cell) && cell->figure == KING) {//���� �� ������ ����� ������, �� �� ��������
                desk[x][y].special = 1;//������ � ������� ���� �����
                ++atacks_on_king;
            }
        }
    }
    void pierce_block(int x, int y, int dx, int dy) {//���������� ����� ��� ������ ��� �������+
        desk[x][y].special = 1;//������ �� ������ - ������ ��� �������
        desk[x][y].mobility = get_mobility(dx, dy);//������, ������� ������ ��� �������
    }
    void ally_line_handler(int x, int y, int dx, int dy) {//��������� �����, ������� ����� ���� ������ ��� ����� ������� ����� � ������ ��� �� ����������
        if(!is_direction_correct(&desk[x][y], dx, dy))//���� ����������� ������������
            return;
        for(int i = dx, j = dy; is_correct(x + i, y + j); i += dx, j += dy) {
            if(is_moveable(x, y, i, j)) {
                add_move(x, y, x + i, y + j);//������������� �� �������
            }
            if(is_figure(&desk[x + i][y + j])) { //���� ����������� �� ���� ������
                break;
            }
        }
    }
    int pierce_detector(int x, int y, int dx, int dy, int cnt) {//������� ���������� ����������� �����
        cell_t *cell = &desk[x + dx][y + dy];//��������� �� ������
        if (!cnt) {//cnt = count
            atack_handler(x, y, dx, dy);//���� ��� ����, ������������ ����
        }
        if (cell->figure == EMPTY) {//���� ������ ������ ���������� ��������� � ���� �����������
            return cnt;//�nt = 0
        }
        ++cnt;
        cnt *= (cell->figure == KING ? -1 : 1);//���� atack_handler ��������� ���
        if (cell->figure == BARRIER || is_enemy(cell) || cnt == 2) {//���� ��������� ����������� �� ������ ��� ���� ������
            return -3;//���� ���������� �� ������� ��� �������� ������, ���
        } else { //����� ������� ������ ���� �����(2), ���
            return cnt;
        }
    }
    void enemy_line_handler(int x, int y, int dx, int dy) {//���������� ��������� ����� ��������� �����
        int cnt = 0, i = 0, j = 0;
        do {//������� ���� ��������� �����, ��� ���������� ������, ���
            i += dx;
            j += dy;;
            cnt = pierce_detector(x, y, i, j, cnt);
        } while (cnt >= 0);
        if (cnt == -1) {//��� ����
            atack_handler(x, y, i + dx, j + dy);
            for (i = x, j = y; desk[i][j].figure != KING; i += dx, j += dy) {//�������� �� ���������� ������
                desk[i][j].special = 1;//������ ����� ��� �������
            }
        } else if (cnt == -2) {//������ ���� �����
            for (i = x, j = y; !is_ally(&desk[i][j]); i += dx, j += dy);
            pierce_block(i, j, dx, dy);//
        }
    }
    void X_handler(int x, int y, int vh, int d, line_handler_t handl) {//�������� �����������
        if (vh) {//���� ��� �� ����������� ��� ���������
            (this->*handl)(x, y, 1, 0);//�����
            (this->*handl)(x, y, -1, 0);//�����
            (this->*handl)(x, y, 0, 1);//�����
            (this->*handl)(x, y, 0, -1);//����
        }
        if (d) {//���� ��� �� �����������
            (this->*handl)(x, y, 1, 1);//�����-������
            (this->*handl)(x, y, 1, -1);//����-������
            (this->*handl)(x, y, -1, 1);//�����-�����
            (this->*handl)(x, y, -1, -1);//�����-����
        }
    }
    void ally_knight_handler(int x, int y) {//��������� ����� �������� ����
        for(int i = 1; i <= 2; ++i) {//������ �� 2 ����� �� �
            int j = 3 - i;//������� �� �
            if(is_moveable(x, y, i, j)) {//���� ��� ��������, �� ������� ���
                add_move(x, y, x + i, y + j);//��� ����� � ������
            }
            if(is_moveable(x, y, i, -j)) {
                add_move(x, y, x + i, y - j);//��� ������ � ����
            }
            if(is_moveable(x, y, -i, j)) {
                add_move(x, y, x - i, y + j);//��� ����� � �����
            }
            if(is_moveable(x, y, -i, -j)) {
                add_move(x, y, x - i, y - j);//��� ����� � ����
            }
        }
    }
    void enemy_knight_handler(int x, int y) {//��������� ������
        for (int i = 1; i <= 2; ++i) {//������ �� 2 �����, ���������� �� �
            int j = 3 - i;//���������� �� �
            atack_handler(x, y, i, j);
            atack_handler(x, y, i, -j);
            atack_handler(x, y, -i, j);
            atack_handler(x, y, -i, -j);
        }
    }
    void ally_king_handler(int x, int y) {//��������� ����� ��� �������� ������
        for (int i = -1; i <= 1; ++i) {//�� ���� ������������ �� �����������
            for (int j = -1; j <= 1; ++j) {//�� ���� ������������ �� ���������
                if (is_correct(x + i, y + j) && !desk[x + i][y + j].attacked &&
                        !is_ally(&desk[x + i][y + j])) {
                    add_move(x, y, x + i, y + j);
                }
            }
        }
    }
    void enemy_king_handler(int x, int y) {//��������� ��������� ����� ��� ���������� ������
        for (int i = -1; i <= 1; ++i) {//�� ���� ������������ �� �����������
            for (int j = -1; j <= 1; ++j) {//�� ���� ������������ �� ���������
                if (desk[x + i][y + j].figure != KING) {
                    atack_handler(x, y, i, j);
                }
            }
        }
    }
    void pawn_kill_move(int x, int y, int dx, int dy) {//��������� �����, ��� ������� ����� ����� ������ ��������� ������: f2->g3, f2->e3
        if ((is_enemy(&desk[x + dx][y + dy]) && is_moveable(x, y, dx, dy))
                || (extra.exist && x + dx == extra.X && y + dy == extra.Y
                    && (is_moveable(x, y, dx, dy) || is_moveable(x, y, dx, 0)))) {
            add_move(x, y, x + dx, y + dy);
        }
    }
    void ally_pawn_handler(int x, int y) {//��������� ����� ��� ������� �����
        int t = (side ? 1 : -1);
        //t = 1 - �����, t = -1 - ������, ���������� �����������, ���� ����� �� �����, ���� ������, �� ���� �� �����
        pawn_kill_move(x, y, 1, t);
        pawn_kill_move(x, y, -1, t);

        if (is_figure(&desk[x][y + t])) {//���������� �������� �� ������, ��� ��� ���� ������
            return;
        }
        if (is_moveable(x, y, 0, t)) {//��������� ���� �� ���� ������ �����/����
            add_move(x, y, x, y + t);
        }
        //��������� ���� �� 2 ������ �����/���� ��� ������� ���� �����
        if (y == 7 - 5 * side && !is_figure(&desk[x][y + 2 * t]) && is_moveable(x, y, 0, 2 * t)) {
            add_move(x, y, x, y + 2 * t);
        }
    }
    void enemy_pawn_handler(int x, int y) {//��������� �����, ������� ����� ������� ��������� �����, ������, ������� ��� ����� ������
        //��� ������ side = -1, ����� 1, ���� ��� �����, ��������� ���� ����, ��� ������
        atack_handler(x, y, 1, -(side ? 1 : -1));//��������� ���� ������
        atack_handler(x, y, -1, -(side ? 1 : -1));//��������� ���� �����
    }
    void ally_rook_handler(int x, int y) {//�������� ������� �����
        X_handler(x, y, 1, 0, ally_line_handler);//�������� ���� �� ����������� � ���������
    }
    void enemy_rook_handler(int x, int y) {//�������� ��������� �����
        X_handler(x, y, 1, 0, enemy_line_handler);//�������� ���� �� ����������� � ���������
    }
    void ally_bishop_handler(int x, int y) {//�������� �������� �����
        X_handler(x, y, 0, 1, ally_line_handler);//�������� ���� �� ����������
    }
    void enemy_bishop_handler(int x, int y) {//�������� ���������� �����
        X_handler(x, y, 0, 1, enemy_line_handler);//�������� ���� �� ����������
    }

    void ally_queen_handler(int x, int y) {//�������� ������� ��������
        X_handler(x, y, 1, 1, ally_line_handler);//�������� ���� � �� �����������/��������� � �� ����������
    }
    void enemy_queen_handler(int x, int y) {//�������� ��������� ��������
        X_handler(x, y, 1, 1, enemy_line_handler);//�������� ���� � �� �����������/��������� � �� ����������
    }
    void ally_figure_handler(int x, int y) {//����� ������ � ����������� ���������� ����� ��� �����
        const static cell_handler_t handlers[] = {//����������� ������ � ��������� ��������� ����� �����
            ally_pawn_handler, ally_queen_handler,
            ally_king_handler, ally_knight_handler,
            ally_bishop_handler, ally_rook_handler
        };
        if(!is_figure(&desk[x][y]) || is_enemy(&desk[x][y])//��������� ��� ������ �� � �������� ��������
                || (is_multicheck() && desk[x][y].figure != KING)) {//
            return;//���� �� ������ �� ����� ������� ������, ���� ����� ���������, ���� ����� ������ ��������� ���������
        }
        (this->*handlers[desk[x][y].figure - 1])(x, y);
    }
    void enemy_figure_handler(int x, int y) {//������� ������ ������������
        const static cell_handler_t handlers[] = {
            enemy_pawn_handler, enemy_queen_handler,
            enemy_king_handler, enemy_knight_handler,
            enemy_bishop_handler, enemy_rook_handler
        };
        //���� ������ �� ������� ��� ��������� ��� ������ ������
        if (!is_figure(&desk[x][y]) || is_ally(&desk[x][y])) {
            return;
        }
        (this->*handlers[desk[x][y].figure - 1])(x, y);//
    }
    void desk_handler(cell_handler_t handl) {//���������� �����
        for (int i = 1; i <= 8; ++i) {
            for (int j = 1; j <= 8; ++j) {
                (this->*handl)(i, j);
            }
        }
    }
    void castle_handler() {//���������� ���������
        if (is_check() || is_multicheck() || castling_active[side][1]) {//
            return;//���� ����������� ���,
        }
        int sideline = 8 - side * 7;//������ ����� ��� �������
        //���� ������ �� �������� � ����� �� ������, ���� ������ �� ������� ����� �����
        //������ �� ��������
        if (!castling_active[side][0]
                && !desk[4][sideline].attacked && !is_figure(&desk[4][sideline])
                && !desk[3][sideline].attacked && !is_figure(&desk[3][sideline])) {
            add_move(5, sideline, 3, sideline);//��� e1->c1, e8->c8
        }
        //���� ������, �� ������� ������������� ������ �� ��������� � ��� ��� ������
        if (!castling_active[side][2] && !desk[6][sideline].attacked && !is_figure(&desk[6][sideline])
                && !desk[7][sideline].attacked && !is_figure(&desk[7][sideline])) {
            add_move(5, sideline, 7, sideline);//��� e1->g1, e8->g8
        }
    }
    void position_handler() {//���������� ���������
        atacks_on_king = 0;//��� ���� �� ������
        clear_move_table();//������� ����� �����
        clear_desk_state();//������� ����� ���������
        release_current_move();//������������� ����
        desk_handler(enemy_figure_handler);//��������� ��������� �������
        desk_handler(ally_figure_handler);//��������� ������� �������
        castle_handler();//���������� ���������
    }
    void update_moves() {//������� ������ �����
        moves.clear();//������� ������� � ����� ��������������� ������
        for (int i = 0; i < 6; ++i) {//6 - ����������� �����, ������ ����� � ������
            for (int j = 0; j < moveptr_list[i]; j += 4) {//////////////////////////////////
                moves.push_back(make_tuple(
                                    move_table[i][j] - 'a' + 1,
                                    move_table[i][j + 1] - '0',
                                    move_table[i][j + 2] - 'a' + 1,
                                    move_table[i][j + 3] - '0'
                                ));//������ �������� �������� � ������
            }
        }
        destroy_move_table();//������� ����� �����
        init_move_table();//������������� ����� �����
    }
    void output_desk() {//������� ������ �����
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

    Chess() {//����������
        init_move_table();//������������� ����� �����
        unpack_position(starting);//���������� ������� ����
        position_handler();//���������� �������
        update_moves();//������ �����
    }
    void start_timer() {//������ ������� �������
        start = clock();
    }
    void game_info() {
        double time = (clock() - start) / CLOCKS_PER_SEC / 60.;
        cout << "\n\t\t\t\t\t" << (side ? "White won\n" : "Black won\n");
        cout << "\t\t\tTime of the game: " << (int)time << " minutes, " << setprecision(2)<< (time - (int)time)*100 << " seconds\n";
        cout << "\t\t\t\tTotal moves: " << white_moves + black_moves << endl;
        cout << "\t\t\tWhite_moves -- " << white_moves << ", black_moves -- " << black_moves << endl;
    }
    bool turn(int number) {//������� ������������� ����
        //�������� ������ ����������� ����, ����� �����������
        if (number < 0 || number >= moves.size()) {//�������� ������������ ��������� ������
            return false;
        }
        //����������, ����������� �������� �������, �������� �� ������� � ������� � ������ � �������� �����
        tie(current_move.fromX, current_move.fromY, current_move.toX, current_move.toY) = moves[number];
        position_handler();//��������� ��� ���������� ����� � �������
        update_moves();
        return true;
    }
    void play() {//������� ������ ����, ����� ����� ���������� �����
        start_timer();
        while (!moves.empty()) {//���� �� ����� �������� ��� ���������� ����
            output_desk();//����� �����
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
            cin >> number;//���� ������ ����
            turn(number - 1);
        }
        game_info();
    }
    void setup() {//���� ����
        string answer;//����� ������������
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
