#pragma once
#include "block.hpp"
// #include "utils.hpp"
// #include <limits>
// #include <string_view>
// #include <array>
// #include <type_traits>
// #include <cstdint>
// #include <bit>
#include <experimental/simd>

namespace reachability {
  template <unsigned W, unsigned H, typename under_t=std::uint64_t>
    requires
      std::numeric_limits<under_t>::is_integer
      && std::is_unsigned_v<under_t>
      && (std::numeric_limits<under_t>::digits >= W)
  //テトリスの盤面データを扱うboard_t構造体
  //盤面の幅, 高さ, およびunder_t型のビット幅を指定
  struct board_t {
    //盤面のビット演算に関する静的定数
    static constexpr int under_bits = std::numeric_limits<under_t>::digits;// `under_t` のビット数
    static constexpr int width = W; // 盤面の幅
    static constexpr int height = H; // 盤面の高さ
    static constexpr int lines_per_under = under_bits / W;// `under_t` に格納できる行数
    static constexpr int used_bits_per_under = lines_per_under * W;// `under_t` 内で使用するビット数
    static constexpr int num_of_under = (H - 1) / lines_per_under + 1;// `under_t` の配列数
    static constexpr int last = num_of_under - 1;// 最後の `under_t` のインデックス
    static constexpr int remaining_per_under = under_bits - used_bits_per_under;// 余りのビット数
    static constexpr under_t mask = under_t(-1) >> remaining_per_under;// 有効ビットのマスク
    static constexpr int remaining_in_last = num_of_under * used_bits_per_under - H * W; // 最後の `under_t` の不要なビット数
    static constexpr under_t last_mask = mask >> remaining_in_last;// 最後の `under_t` 用のマスク
    constexpr board_t() = default;// デフォルトコンストラクタ
    constexpr board_t(std::string_view s): board_t(convert_to_array(s)) {}// 文字列から盤面を初期化するコンストラクタ
    constexpr board_t(std::array<under_t, num_of_under> d): data{d.data(), std::experimental::element_aligned} {}// `under_t` の配列から盤面を初期化するコンストラクタ

    static constexpr std::array<under_t, num_of_under> convert_to_array(std::string_view s) {// 文字列を `under_t` の配列に変換する関数
      std::array<under_t, num_of_under> data = {};
      for (std::size_t i = 0; i < last; ++i) {
        data[i] = convert_to_under_t(s.substr(W * H - (i + 1) * used_bits_per_under, used_bits_per_under));
      }
      data[last] = convert_to_under_t(s.substr(0, used_bits_per_under - remaining_in_last));
      return data;
    }
    // 盤面の特定の座標 (x, y) にブロックをセットする関数
    template <int x, int y>
    constexpr void set() {
      data[y / lines_per_under] |= under_t(1) << ((y % lines_per_under) * W + x);
    }
    // 盤面の特定の座標 (x, y) の値を取得する関数
    // 0: 空, 1: ブロックあり, 2: 範囲外
    template <int x, int y>
    constexpr int get() const {
      if ((x < 0) || (x >= W) || (y < 0) || (y >= H)) {
        return 2;
      }
      return data[y / lines_per_under] & (under_t(1) << ((y % lines_per_under) * W + x)) ? 1 : 0;
    }
    // y 行目の右端の値を取得する関数
    template <int y>
    constexpr int get() const {
      // use highest bit as the result
      return get<W - 1, y>();
    }
    // 盤面が空でないか判定する関数
    constexpr bool any() const {
      return *this != board_t{};
    }
    // 盤面同士の比較 (等しくない場合 true)
    constexpr bool operator!=(board_t other) const {
      return any_of(data != other.data);
    }
    // 盤面のビットを反転する演算子
    constexpr board_t operator~() const {
      board_t other;
      other.data = mask_board() & ~data;
      return other;
    }
    // AND 演算
    constexpr board_t &operator&=(board_t rhs) {
      data &= rhs.data;
      return *this;
    }
    constexpr board_t operator&(board_t rhs) const {
      board_t result = *this;
      result &= rhs;
      return result;
    }
    // OR 演算
    constexpr board_t &operator|=(board_t rhs) {
      data |= rhs.data;
      return *this;
    }
    constexpr board_t operator|(board_t rhs) const {
      board_t result = *this;
      result |= rhs;
      return result;
    }
    // XOR 演算
    constexpr board_t &operator^=(board_t rhs) {
      data ^= rhs.data;
      return *this;
    }
    constexpr board_t operator^(board_t rhs) const {
      board_t result = *this;
      result ^= rhs;
      return result;
    }
    
    //盤面を指定した方向に移動する関数
    //d 移動する座標の変化量(dx,dy)
    //check 移動後の制約チェックを行うかどうか　デフォルトはtrue
    template <coord d, bool check = true>
    constexpr void move_() {
      constexpr int dx = d[0], dy = d[1];
      // 垂直移動なし (dx のみの変化)
      if constexpr (dy == 0) {
        if constexpr (dx > 0) {// 右方向への移動
          data <<= dx;
          data &= mask_board();

          
        } else if constexpr (dx < 0) {// 左方向への移動
          data >>= -dx;
        }
      //　上方向への移動
      } else if constexpr (dy > 0) {
        constexpr int pad = (dy - 1) / lines_per_under;
        constexpr int shift = (dy - 1) % lines_per_under + 1;
        auto not_moved = my_shift<dx, shift>(my_split<pad, true>(data));
        auto moved = my_shift<dx, shift-lines_per_under>(my_split<pad+1, true>(data));
        data = (not_moved | moved) & mask_board();
      // 下方向への移動
      } else {
        constexpr int pad = (-dy - 1) / lines_per_under;
        constexpr int shift = (-dy - 1) % lines_per_under + 1;
        auto not_moved = my_shift<dx, -shift>(my_split<pad, false>(data));
        auto moved = my_shift<dx, lines_per_under-shift>(my_split<pad+1, false>(data));
        data = (not_moved | moved) & mask_board();
      }
      // 横方向の移動制約チェックを適用
      if constexpr (check && dx != 0) {
        data &= mask_move<dx>();
      }
    }
    // 盤面を指定した方向に移動した結果を返す関数 (元の盤面は変更しない)
    template <coord d, bool check = true>
    constexpr board_t move() const {
      board_t result = *this;
      result.move_<d, check>();
      return result;
    }

    // 盤面の状態を文字列に変換する関数(一つの盤面を表示)
    friend constexpr std::string to_string(board_t board) {
      std::string ret;
      static_for<H>([&][[gnu::always_inline]](auto y) {
        std::string this_ret;
        static_for<W>([&][[gnu::always_inline]](auto x) {
          this_ret += board.get<x, y>() ? "[]" : "  ";//ブロックがある場合は"[]", ない場合は空白
        });
        this_ret += '\n';
        ret = this_ret + ret;// 盤面の上から順に表示するため、逆順に追加
      });
      return ret;
    }
    // 盤面の状態を2つ比較し、それぞれのブロックの違いを視覚化する関数
    friend constexpr std::string to_string(board_t board1, board_t board2) {
      std::string ret;
      static_for<H>([&][[gnu::always_inline]](auto y) {
        std::string this_ret;
        static_for<W>([&][[gnu::always_inline]](auto x) {
          bool b1 = board1.get<x, y>();
          bool b2 = board2.get<x, y>();
          if (b1 && b2) {
            this_ret += "%%";// 両方の盤面にブロックがある場合
          } else if (b1) {
            this_ret += "..";// board1 にのみブロックがある場合
          } else if (b2) {
            this_ret += "[]";// board2 にのみブロックがある場合
          } else {
            this_ret += "  ";// どちらにもブロックがない場合
          }
        });
        this_ret += '\n';
        ret = this_ret + ret;
      });
      return ret;
    }
    // 盤面の状態を3つ比較し、それぞれのブロックの違いを視覚化する関数
    friend constexpr std::string to_string(board_t board1, board_t board2, board_t board_3) {
      std::string ret;
      static_for<H>([&][[gnu::always_inline]](auto y) {
        std::string this_ret;
        static_for<W>([&][[gnu::always_inline]](auto x) {
          bool tested[2] = {bool(board1.get<x, y>()), bool(board2.get<x, y>())};
          bool b3 = board_3.get<x, y>();
          std::string symbols = "  <>[]%%";// 各盤面のブロックの違いを表すシンボル
          for (int i = 0; i < 2; ++i) {
            this_ret += symbols[b3 * 4 + tested[i] * 2 + i];
          }
        });
        this_ret += '\n';
        ret = this_ret + ret;
      });
      return ret;
    }
    // 完全に埋まったラインをクリアし、その数を返す関数
    constexpr int clear_full_lines() {
      auto board = data;
      constexpr int needed = std::numeric_limits<decltype(W)>::digits - std::countl_zero(W) - 1;
      static_for<needed>([&][[gnu::always_inline]](auto i) {
        auto temp = board;
        temp.template right_shift<1 << i>();
        board &= temp;
      });
      auto temp = board;
      temp.template right_shift<W - (1 << needed)>();
      board &= temp;
      int lines = 0;
      // 指定範囲のラインを削除し詰める関数
      const auto remove_range = [](board_t data, unsigned start, unsigned end) {
        board_t below = data << (H * W - start);
        below >>= (H * W - start);
        board_t above = data >> end;
        above <<= start;
        return below | above;
      };
      static_for<H>([&][[gnu::always_inline]](auto y){
        if (board.template get<0, y>()) {
          data = remove_range(data, (y - lines) * W, (y - lines + 1) * W);
          ++lines;
        }
      });
      return lines;
    }
    // 盤面に唯一のビットが存在するかを確認する関数
    constexpr board_t has_single_bit() const {
      auto saturated = data | one_bit<W - 1>();
      saturated &= saturated - one_bit<0>();
      // データの最上位ビット以外に 1 がない場合: 0
      // データの最上位ビット以外に ちょうど 1 つの 1 がある場合: 10...0
      // データの最上位ビット以外に 1 が 2 つ以上ある場合: 1...1...
      auto saturated2 = saturated | one_bit<W - 1>();
      saturated2 &= saturated2 - one_bit<0>();
      auto result = (saturated ^ data) & ~saturated2;
      return to_board(result);
    }
    // すべてのビットが立っている状態を取得する関数
    constexpr board_t all_bits() const {
      auto low = data & ~one_bit<W - 1>();
      return to_board(data & (low + one_bit<0>()));
    }
    // いずれかのビットが立っているかを確認する関数
    constexpr board_t any_bit() const {
      return ~to_board(~data).all_bits();
    }
    // いずれのビットも立っていないことを確認する関数
    constexpr board_t no_bit() const {
      return ~any_bit();
    }
    // 0 の後に続く 1 をすべて削除する関数
    constexpr board_t remove_ones_after_zero() const {
      auto board = data | ~mask_board();// 有効な盤面外のビットを1にする
      std::array<int, num_of_under> ones;
      // 各 `under_t` における連続する1のカウント
      static_for<num_of_under>([&][[gnu::always_inline]](auto i) {
        ones[i] = std::countl_one(under_t(board[i]));
      });
      bool found = false;
      #pragma unroll num_of_under
      for (int i = num_of_under - 1; i >= 0; --i) {
        if (found) {
          board[i] = 0;// すでに0が見つかっていた場合、その後のすべての1を削除
        } else if (ones[i] < std::numeric_limits<under_t>::digits) {
          found = true;
          board[i] &= ~((~under_t(0)) >> ones[i]);// 0が出現した後の1を削除
        }
      }
      return to_board(board & mask_board());
    }
    // 盤面の最上位ビットをセットし、それをすべてのビットに拡張する関数
    constexpr board_t populate_highest_bit() const {
      // result is in highest bit (0 or 1), other bits are 0
      // populate the result to all bits
      auto result = data & one_bit<W - 1>();// 最上位ビットの抽出
      auto pre_result = one_bit<W - 1>() - (result >> (W - 1));
      return to_board(pre_result ^ one_bit<W - 1>());
    }

    // 盤面の先頭のブロック位置を取得する関数
    constexpr board_t get_heads() const {
      return (*this) & ~move<coord{-1, 0}>(); // 左方向に移動したものとの差分を取ることで「頭」部分を抽出
    }
    // 盤面を膨張させられるかを確認する関数
    friend constexpr board_t can_expand(board_t current, board_t possible) {
      const auto starts = possible & current.template move<coord{-1, 0}>();// 左方向のブロック
      const auto ends = possible & current.template move<coord{1, 0}>();// 右方向のブロック
      const auto all_heads = possible.get_heads(); // 盤面の先頭部分
      const auto heads = starts.data | (ends.data + (possible & ~all_heads).data);
      return to_board(heads);
    }
  private:
  // SIMD 型の定義
    template <std::size_t N>
    using simd_of = std::experimental::simd<under_t, std::experimental::simd_abi::deduce_t<under_t, N>>;
    using data_t = simd_of<num_of_under>;
    alignas(std::experimental::memory_alignment_v<data_t>) data_t data = 0;
    template <std::size_t N>// 定数ゼロ
    static constexpr simd_of<N> zero = {};
    static constexpr board_t to_board(data_t data) {// SIMDデータを `board_t` に変換する関数
      board_t ret;
      ret.data = data;
      return ret;
    }
    // 指定方向の移動制限マスクを生成する関数
    template <int dx>
    static constexpr data_t mask_move() {
      board_t mask;
      if constexpr (dx > 0) {
        static_for<dx>([&][[gnu::always_inline]](auto i) {
          static_for<H>([&][[gnu::always_inline]](auto j) {
            mask.set<i, j>();
          });
        });
      } else if constexpr (dx < 0) {
        static_for<-dx>([&][[gnu::always_inline]](auto i) {
          static_for<H>([&][[gnu::always_inline]](auto j) {
            mask.set<W - 1 - i, j>();
          });
        });
      }
      return (~mask).data;
    }
    // 指定のビットを立てる関数
    template <int dx>
    static constexpr data_t one_bit() {
      board_t ret;
      static_for<H>([&][[gnu::always_inline]](auto j) {
        ret.set<dx, j>();
      });
      return ret.data;
    };
    // 盤面のマスクを生成する関数
    static constexpr data_t mask_board() {
      return data_t{[](auto i) {
        if constexpr (i == last) {
          return last_mask;
        } else {
          return mask;
        }
      }};
    }
    // データを分割する関数
    template <int removed, bool from_right>
    static constexpr data_t my_split(data_t data) {
      return data_t([=][[gnu::always_inline]](auto i) {
        constexpr size_t index = from_right ? i - removed : i + removed;
        if constexpr (index >= num_of_under) {
          return 0;
        } else {
          return data[index];
        }
      });
    }
    // 指定方向へデータをシフトする関数
    template <int x_shift, int y_shift>
    static constexpr data_t my_shift(data_t data) {
      if constexpr (y_shift == lines_per_under || y_shift == -lines_per_under) {
        data = data_t(0);
      } else if constexpr (y_shift < 0) {
        data >>= -y_shift * W;
      } else if constexpr (y_shift > 0) {
        data <<= y_shift * W;
      }
      if constexpr (x_shift > 0) {
        data <<= x_shift;
      } else if constexpr (x_shift < 0) {
        data >>= -x_shift;
      }
      return data;
    }
    //テキストで表現された盤面データをバイナリ（ビット）形式に変換する関数
    static constexpr under_t convert_to_under_t(std::string_view in) {
      under_t res = 0;
      for (char c : in) {
        res *= 2;
        if (c == 'X')
          res += 1;
      }
      return res;
    }
  };
}