#pragma once
#include <array>
#include <initializer_list>
#include <string_view>
#include <chrono>
#include "../board.hpp"
constexpr auto merge_str(std::initializer_list<std::string_view> &&b_str) {
    std::array<char, WIDTH*HEIGHT> res = {};
    unsigned pos = 0;
    // 盤面の高さが指定された文字列リストのサイズより大きい場合、空白で埋める
    if (b_str.size() < HEIGHT) {
      for (pos = 0; pos < WIDTH * (HEIGHT - b_str.size()); ++pos) {
        res[pos] = ' ';
      }
    }
    // 文字列リストを逆順にしてres配列に格納
    for (auto &s : b_str) {
      for (unsigned i = 0; i < WIDTH; ++i) {
        res[pos + i] = s[WIDTH - i - 1];
      }
      pos += WIDTH;
    }
    // BOARD型の配列に変換して返す
    return BOARD::convert_to_array(std::string_view{res.data(), res.size()});
}