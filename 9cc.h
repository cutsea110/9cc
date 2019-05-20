/*
 * 9cc.h
 */

// トークンの型を表す値
enum {
      TK_NUM = 256, // 整数トークン
      TK_IDENT,     // 識別子
      TK_RETURN,    // return
      TK_IF,        // if
      TK_ELSE,      // else
      TK_WHILE,     // while
      TK_FOR,       // if
      TK_EQ,        // ==
      TK_NE,        // !=
      TK_GE,        // >=
      TK_GT,        // >
      TK_LE,        // <=
      TK_LT,        // <
      TK_EOF,       // 入力の終端を表すトークン
};

enum {
      ND_NUM = 256, // 整数のノードの型
      ND_IDENT,     // 識別子
      ND_RETURN,    // return
      ND_IF,        // if
      ND_ELSE,      // else
      ND_WHILE,     // while
      ND_FOR,       // for
      ND_BLOCK,     // block
      ND_NOP,       // no operation
      ND_EQ,        // ==
      ND_NE,        // !=
      ND_GE,        // >=
      ND_GT,        // >
      ND_LE,        // <=
      ND_LT,        // <
};

typedef struct {
  int ty;           // トークンの型
  int val;          // tyがTK_NUMの場合その数値
  char* name;       // tyがTK_IDENTの場合その名前
  char* input;      // トークン文字列(エラーメッセージ用)
} Token;

typedef struct {
  void** data;      // データ本体
  int capacity;     // バッファの大きさ(data[0]~data[capacity-1]がバッファ領域)
  int len;          // ベクタに追加済みの要素数
} Vector;

typedef struct {
  Vector* keys;
  Vector* vals;
} Map;

typedef struct Node {
  int ty;           // 演算子かND_NUM,ND_IDENT
  struct Node* lhs; // 左辺
  struct Node* rhs; // 右辺
  int val;          // tyがND_NUMの場合その数値
  char* name;       // ND_IDENTの場合のみ使う
  Vector* blk;     // ND_BLOCKの場合のみ使う
} Node;

extern void error(char* fmt, ...);
extern void DEBUG(char* fmt, ...);
extern void DUMP_TOKENS();
extern void DUMP_CODES();

extern Vector* new_vector();
extern void vec_push(Vector*, void*);

extern Vector* new_vector();
extern void vec_push(Vector* vec, void* elem);

extern Map* new_map();
extern void map_put(Map* map, char* key, void* val);
extern void* map_get(Map* map, char* key);
extern int map_exists(Map* map, char* key);

extern void runtest();

extern Token* add_token(Vector* v, int ty, char* p);
extern Vector* tokenize(char* p);
extern void program();

extern void gen_lval(Node* node);
extern void gen(Node* node);

extern Map* variables;
extern Vector* tokens;
extern int pos;
extern Node* code[100];
extern int debug_flg;
extern int label_count;
