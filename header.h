#ifndef HEADER_H
#define HEADER_H

// �������� ���������
#define PROGRAM_TITLE "Hex Viewer"

// ������������ ������ ������ �� ������ (�������)
#define MAX_BUF_SIZE 256

// ���������� �������� ��� ��������� ������
// ��������� ����� ���������� ����������� ����������
#define TEXT_BRIGHTER FOREGROUND_INTENSITY
#define TEXT_BLACK 0
#define TEXT_WHITE FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
#define TEXT_DEFAULT TEXT_WHITE | TEXT_BRIGHTER

// ���������� �� ��� ������ ����
#define BACK_BRIGHTER BACKGROUND_INTENSITY
#define BACK_BLACK 0
#define BACK_BLUE BACKGROUND_BLUE
#define BACK_WHITE BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED
#define BACK_DEFAULT BACK_BLACK

// ������ � ������ ����������� ������
#define BUF_W 120
#define BUF_H 30

#define CONTENT_SIZE 448

// ������ ��������
#define TEXT_W 16
#define BYTES_W 47
#define OFFSET_W 8

// ������ �������� � ������ �������
#define PATH_W 60
#define SIZE_W 30
#define DATE_W 18
#define ENC_W 12

// ���������� ��������� ������� � ������� ��������������
#define HEAD_OFFSET_X 0
#define HEAD_OFFSET_Y 1
#define OFFSET_X 0
#define OFFSET_Y 2

#define HEAD_BYTES_X 10
#define HEAD_BYTES_Y 1
#define BYTES_X 10
#define BYTES_Y 2

#define HEAD_TEXT_X 59
#define HEAD_TEXT_Y 1
#define TEXT_X 59
#define TEXT_Y 2

// ���������� ������ ������ (������)
#define LINE_TOP_X 10
#define LINE_TOP_Y 2

// ���������� ��������� ������ (�����)
#define LINE_BOT_X 10
#define LINE_BOT_Y 29

// ���������� �������� � ������ �������
#define ENC_X 108
#define ENC_Y 0

#define PATH_X 0
#define PATH_Y 0

#define SIZE_X 60
#define SIZE_Y 0

#define DATE_X 90
#define DATE_Y 0


// ��������� ���������
enum ENC
{
    ENC_CP1251, // ���������
    ENC_CP1252 // ����������� ��������
};


// ����������� ����������� �������
enum DIR
{
    DIR_LEFT,
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN
};


// ����������� ���������
enum SCROLL
{
    SCROLL_DOWN,
    SCROLL_UP
};


// ������ ����������� �� �����
typedef struct CURSOR
{
    int x;
    unsigned long long y;
} Cursor;


// �������� �����
typedef struct INFO
{
    char* path;
    char date[17]; // ����� ���� �� �������� 16 �������� (+ '\0')
    unsigned long long size;
    int encoding;
} Info;


// ���������� �����
typedef struct CONTENT
{
    unsigned long long begin; // ������� �����������, ������� ���������
    unsigned long long end;
} Content;


// ���������� ��������
// �����������
extern HANDLE handle;
extern HANDLE consoleOut;
extern HANDLE consoleIn;
extern HANDLE file;

// ������� �������
extern int CURSOR_BORDER_X;
extern unsigned long long CURSOR_BORDER_Y;


// ��������� �������
void cprintf(const char *, ... );
void move_cursor(int);
void pad_to(char*, char*, int );
void set_draw_pos(int, int);
void draw_path();
void draw_size();
void draw_enc();
void draw_date();
void draw_info();
void draw_headers();
void cursor_display(int);
int is_unused_char(char);
void draw_offset_line(unsigned long long);
void draw_bytes_line(char*, int);
void draw_text_line(char*, int );
void draw_file(int, unsigned long long, unsigned long long);
void change_encoding();
void scroll_area(int, int, int, int, int, int);
void scroll(int);
void init();
void file_get_creation_date(FILE*);


#endif
