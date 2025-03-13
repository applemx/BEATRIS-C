#pragma once
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <thread>
#include <chrono>

namespace input_handler {
  struct TerminalConfig {
    struct termios original;
    bool initialized = false;
    
    ~TerminalConfig() {
      // 終了時に元の設定に戻す
      if (initialized) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original);
      }
    }
  };
  
  static TerminalConfig config;
  
  // ターミナル設定を非カノニカルモードに変更
  inline void init_terminal() {
    if (!config.initialized) {
      // 現在の設定を保存
      tcgetattr(STDIN_FILENO, &config.original);
      
      // 設定を変更
      struct termios raw = config.original;
      raw.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &raw);
      
      // 標準入力を非ブロッキングに設定
      int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
      
      config.initialized = true;
    }
  }
  
  // 非ブロッキングでキー入力を取得
  inline char get_user_input() {
    char ch = 0;
    if (read(STDIN_FILENO, &ch, 1) < 0) {
      // キー入力がない場合は0を返す
      return 0;
    }
    return ch;
  }
}