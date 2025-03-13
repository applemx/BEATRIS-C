// language: cpp
#include "board.hpp"
#include "block.hpp"
#include "renderer_debug.cpp"
#include "findfinalattachableminostates.hpp"  // ../を削除
#include "input_handler.hpp"  // 新しく作成したヘッダファイルを使用
#include <iostream>
#include <thread>
#include <chrono>

// ゲームループ用のサンプル
int main() {
    // 非ブロッキング入力の初期化
    input_handler::init_terminal();
    
    // 盤面の初期化
    reachability::board_t<10, 24> board;
    
    // 現在のミノと初期位置・回転状態
    constexpr char currentMino = 'T';
    using SRS = reachability::blocks::SRS;
    constexpr reachability::coord start_pos{{4, 20}};//まぁ実質定数だわな
    // board.set_mino_init<SRS,0,start_pos>(currentMino);　凍結　使えなかった
    
    // 現在の位置と回転
    int posx = 5, posy = 0;
    int rotation = 0;
    
    bool running = true;
    while(running) {
        // 画面クリア（Unix系）
        std::cout << "\033[2J\033[H";
        
        // 盤面の描画
        std::cout << "BEATRIS-C Game - WASD to move/rotate, Q to quit\n\n";
        std::cout << renderer::draw(board) << "\n";
        
        // 操作説明
        std::cout << "Controls: A/D - move left/right, W/S - rotate, Space - drop, Q - quit\n";
        
        // ユーザー入力受付
        char input = input_handler::get_user_input();
        switch(input) {
            case 'q': running = false; break;
            case 'a': posx--; break; // 左移動
            case 'd': posx++; break; // 右移動
            case 'w': rotation = (rotation + 1) % 4; break; // 時計回り回転
            case 's': rotation = (rotation + 3) % 4; break; // 反時計回り回転
        }
        
        // 表示更新のためのディレイ
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}