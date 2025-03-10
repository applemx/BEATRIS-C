# コンパイラとオプション
CXX      = g++
CXXFLAGS = -std=c++2b -Wall
# 必要に応じて includePath やライブラリオプションを追加します
# 例: CXXFLAGS += -I/path/to/your/headers

# テスト対象の実行ファイル名とソースファイル
TARGET = test_board
SRCS   = test_board.cpp
OBJS   = $(SRCS:.cpp=.o)

# デフォルトターゲット：実行ファイルの生成
all: $(TARGET)

# リンクルール
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# 個々のソースファイルのコンパイル
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 後始末（オブジェクトファイルと実行ファイルを削除）
clean:
	rm -f $(OBJS) $(TARGET)