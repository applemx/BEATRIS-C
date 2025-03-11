// test_board.cpp
#include "../board.hpp"
#include "../findfinalattachableminostates.hpp"
#include "test_utils.hpp"
#include <iostream>
#include <cassert>
#include <string>
#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

// board.hpp 内では、ブロックの座標型は reachability::coord（std::array<int,2> として定義）
// 盤面型の型エイリアスを作成（幅10、高さ24 とする例）
using Board = reachability::board_t<10, 24>;

// 各テスト項目を個別の関数に分割

// 1. 初期状態のテスト：盤面は全セル空であるはず（get() が 0 を返す）
void test_initial_state() {
    Board board;
    // 代表的な座標だけチェック（テンプレート引数はリテラルで指定する必要があるため）
    assert((board.template get<0, 0>() == 0));
    assert((board.template get<3, 5>() == 0));
    // 例えば x==10 は範囲外の場合（実際はテンプレート引数で負の数などは使いにくいので省略）
    std::cout << "Test: initial board is empty passed.\n";
}

// 2. set()/get() のテスト：(3,5) にブロックをセットして確認
void test_set_get() {
    Board board;
    board.template set<3, 5>();
    assert((board.template get<3, 5>() == 1));
    std::cout << "Test: set()/get() passed.\n";
}

// 3. operator~ のテスト：セット済みのセルを反転すれば 0 になるはず
void test_operator_not() {
    Board board;
    board.template set<3, 5>();
    Board board_not = ~board;
    assert((board_not.template get<3, 5>() == 0));
    std::cout << "Test: operator~ passed.\n";
}

// 4. move() のテスト：右方向へ 1 マス移動する例
void test_move() {
    Board board;
    board.template set<3, 5>();
    // move() はテンプレート非型引数として coord を受け取る前提
    Board board_moved = board.template move<reachability::coord{{1, 0}}>();
    // (3,5) にあったブロックは、右に 1 移動して (4,5) に現れるはず
    assert((board_moved.template get<4, 5>() == 1));
    std::cout << "Test: move() passed.\n";
}

// 5. to_string() のテスト：盤面の文字列表現を表示
void test_to_string() {
    Board board;
    board.template set<3, 5>();
    std::string s = to_string(board);
    std::cout << "Board representation:\n" << s << "\n";
    std::cout << "Test: to_string() passed.\n";
}

void test_example1_findfinalattachableminostates(){
    constexpr auto example1 = merge_str({
        "     X    ",
        "XXX X     ",
        "         X",
        " X X  XX  ",
        " X        ",
        "    X X  X",
        "X   XXX   ",
        "X XX      ",
        "       X  ",
        "    X     ",
        "  XX X X  ",
        "X     XX  ",
        "   X X    ",
        " X      X ",
        "    X     ",
        "XX     X X",
        "    X X   ",
        "X  X      ",
        "    XX    ",
        "  X       ",
        "       X  ",
        " XXX XX X ",
    });
    auto positions = reachability::search::usable_positions<reachability::blocks::T>(example1);
    printf(positions);
}

int main() {
    test_initial_state();
    test_set_get();
    test_operator_not();
    test_move();
    test_to_string();
    test_example1_findfinalattachableminostates();
    
    std::cout << "All board tests passed.\n";
    return 0;
}
