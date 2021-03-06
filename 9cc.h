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
      TK_INT,       // int
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
      ND_DEREF,     // dereference *
      ND_REF,       // reference &
      ND_RETURN,    // return
      ND_IF,        // if
      ND_ELSE,      // else
      ND_WHILE,     // while
      ND_FOR,       // for
      ND_BLOCK,     // block
      ND_VARDEF,    // variable definition
      ND_FUNDEF,    // function definition
      ND_FUNCALL,   // function call
      ND_ARGS,      // argument expr list
      ND_NOP,       // no operation
      ND_EQ,        // ==
      ND_NE,        // !=
      ND_GE,        // >=
      ND_GT,        // >
      ND_LE,        // <=
      ND_LT,        // <
};

enum {
      VOID = 1,
      INT,
      PTR,
};

typedef struct Type {
  int ty;
  struct Type* ptr_to;
  int size;
} Type;

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
  char* name;       // ND_IDENT, ND_FUNDEFの場合のみ使う
  int arg_num;      // ND_FUNDEFの場合のみ使う
  Map* local_vars;  // ND_FUNDEFの場合のみ使う
  Vector* blk;      // ND_BLOCK, ND_FUNDEFの場合のみ使う
  Type* tsig;       // ND_VARDEF,ND_FUNDEFの場合のみ使う
} Node;

int roundup(int x, int align);
void error(char* fmt, ...);
void DEBUG(char* fmt, ...);
void DUMP_TOKENS();
void DUMP_CODES();

Vector* new_vector();
void vec_push(Vector* vec, void* elem);

Map* new_map();
void map_put(Map* map, char* key, void* val);
void* map_get(Map* map, char* key);
int map_exists(Map* map, char* key);

void runtest();

Type* ptr_to(Type* base);
Type* int_ty();

Token* add_token(Vector* v, int ty, char* p);
Vector* tokenize(char* p);
void program();

void gen_lval(Node* node);
void gen(Node* node);

Map* global_vars;
Map* current_vars;
Map* vars_map();

Vector* tokens;
int pos;
Node* code[100];
int debug_flg;
int label_count;
