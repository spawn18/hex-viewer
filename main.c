#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

#include "header.h"


Cursor cursor;
Content content;
Info info;


HANDLE handle = 0;
HANDLE consoleOut = 0;
HANDLE consoleIn = 0;
HANDLE file = 0;


int CURSOR_BORDER_X = 0;
unsigned long long CURSOR_BORDER_Y = 0;


// Изменить цвета вывода
static inline void set_colors(int colors)
{
    SetConsoleTextAttribute(consoleOut, colors);
}


// Вывод текста в консоль на позицию заранее выбранную курсором
static inline void draw(const char *text)
{
    WriteConsole(consoleOut, text, strlen(text), NULL, NULL);
}


// Рисование
void cprintf(const char *__format, ... )
{
    char buffer[MAX_BUF_SIZE];
    va_list ap;
    va_start(ap, __format);
    vsprintf(buffer, __format, ap);
    va_end(ap);
    draw(buffer);
}


// Передвинуть курсор в нужном направлении
void move_cursor(int direction)
{
    cursor_display(FALSE);

    if(direction == DIR_LEFT)
    {
        if(cursor.y != 0 || cursor.x != 0)
            cursor.x--;

        if(cursor.x < 0)
        {
            cursor.x = 15; // Аналогично как и для правого, курсор выйдет справа сверху
            cursor.y--;
        }
    }
    else if(direction == DIR_UP)
    {
        if(cursor.y != 0)
            cursor.y--;
    }
    else if(direction == DIR_RIGHT)
    {
        if(cursor.y != CURSOR_BORDER_Y)
        {
            if(cursor.x == 15)
            {
                cursor.x = 0; // если не последняя строка файла но последний символ
                cursor.y++; // то курсор появится слева ниже
            }
            else cursor.x++;
        }
        else
        {
            if(cursor.x < CURSOR_BORDER_X) cursor.x++;
        }
    }
    else if(direction == DIR_DOWN)
    {
        if(cursor.y != CURSOR_BORDER_Y)
         cursor.y++;

        if(cursor.y == CURSOR_BORDER_Y && cursor.x > CURSOR_BORDER_X)
            cursor.x = CURSOR_BORDER_X; // Спустится на последнюю строку к последнему символу
    }

    // Если курсор ушел за рамку вниз или вверх то прокрутить
    if(cursor.y >= content.end)
    {
        content.begin += 1;
        content.end += 1;

        scroll(SCROLL_DOWN);
    }
    else if(cursor.y < content.begin)
    {
        content.begin -= 1;
        content.end -= 1;

        scroll(SCROLL_UP);
    }

    cursor_display(TRUE);
}


// Создает новую строку из предыдущей нужного размера
// и центрирует содержимое (используется в шапке)
void pad_to(char* out, char* in, int len)
{
    int inLen = strlen(in);
    memset(out, ' ', len);
    out[len] = '\0';

    int off = floor((len - inLen)/2);

    if (off < 0)
    {
        off = 2;
        inLen = len-2;
    }

    strncpy(out+off, in, inLen);
}


// Изменяет позицию рисования
void set_draw_pos(int x, int y)
{
    COORD coord = {x, y};
    SetConsoleCursorPosition(consoleOut, coord);
}


// Отобразить путь к файлу
void draw_path()
{
    set_colors(BACK_WHITE | BACK_BRIGHTER);
    set_draw_pos(PATH_X, PATH_Y);

    char path[PATH_W+1];
    pad_to(path, info.path, PATH_W);
    cprintf("%s", path);
    set_colors(TEXT_DEFAULT | BACK_DEFAULT);
}


// Отобразить размер файла
void draw_size()
{
    set_colors(BACK_WHITE | BACK_BRIGHTER);
    set_draw_pos(SIZE_X, SIZE_Y);

    char sizeBuf[SIZE_W+1];
    sprintf(sizeBuf, "%lld bytes", info.size);

    char size[SIZE_W+1];
    pad_to(size, sizeBuf, SIZE_W);

    cprintf(size);
    set_colors(TEXT_DEFAULT | BACK_DEFAULT);
}


// Отобразить текущую кодировку
void draw_enc()
{
    set_colors(TEXT_WHITE | TEXT_BRIGHTER | BACK_BLUE | BACK_BRIGHTER);
    set_draw_pos(ENC_X, ENC_Y);

    char encBuf[10];

    if(info.encoding == ENC_CP1252)
        strcpy(encBuf, "CP1252");
    else
        strcpy(encBuf, "CP1251");

    char enc[ENC_W+1];
    pad_to(enc, encBuf, ENC_W);

    cprintf("%s", enc);
    set_colors(TEXT_DEFAULT | BACK_DEFAULT);
}


// Отобразить дату создания
void draw_date()
{
    set_colors(BACK_WHITE | BACK_BRIGHTER);
    set_draw_pos(DATE_X, DATE_Y);

    char date[DATE_W+1];
    pad_to(date, info.date, DATE_W);
    cprintf("%s", date);

    set_colors(TEXT_DEFAULT | BACK_DEFAULT);
}


// Отобразить панель свойств файла
// Путь, имя, размер, дата создания и выбранная кодировка
void draw_info()
{
    draw_path();
    draw_size();
    draw_date();
    draw_enc();
}

// Отображает шапку - названия колонок
void draw_headers()
{
    set_draw_pos(HEAD_OFFSET_X, HEAD_OFFSET_Y);

    char offsetTitle[OFFSET_W+1];
    char bytesTitle[BYTES_W+1];
    char textTitle[TEXT_W+1];
    char* spaces = "  ";

    // Не выносится в отдельные константы потому что используется лишь один раз
    pad_to(offsetTitle, "Offset", OFFSET_W);
    pad_to(bytesTitle, "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", BYTES_W);
    pad_to(textTitle, "Text", TEXT_W);

    set_colors(TEXT_WHITE | TEXT_BRIGHTER | BACK_WHITE);

    cprintf("%s", offsetTitle);
    cprintf("%s", spaces);
    cprintf("%s", bytesTitle);
    cprintf("%s", spaces);
    cprintf("%s", textTitle);

    set_colors(TEXT_DEFAULT | BACK_DEFAULT);
}

// Включает/выключает курсор(ы)
void cursor_display(int state)
{
    // Координаты курсора в символах
    COORD coord = {OFFSET_W+2 + cursor.x*3, 2+(cursor.y-content.begin)};

    int color = 0;

    if(state == TRUE)
        color = TEXT_BLACK | BACK_WHITE | BACK_BRIGHTER;
    else
        color = TEXT_DEFAULT | BACK_DEFAULT;

    DWORD n = 0;
    FillConsoleOutputAttribute(consoleOut, color, 2, coord, &n);

    // Курсор на колонке текста
    coord.X = OFFSET_W+BYTES_W+4 + cursor.x;
    coord.Y = 2+(cursor.y-content.begin);

    FillConsoleOutputAttribute(consoleOut, color, 1, coord, &n);
}


// Является ли символ "пустым"
// В таблице ASCII у значения может не быть символа
int is_unused_char(char c)
{
    return 0 <= c && c < 32;
}


// Отобразить одну строку счетчика байтов
void draw_offset_line(unsigned long long i)
{
    cprintf("%08x", i*16);
}


// Отобразить строку байт в шестнадцатеричном виде длины n
void draw_bytes_line(char* line, int n)
{
    for(int j = 0; j < n; j++)
    {
        cprintf("%02x", (unsigned char)line[j]);

        if(j != 15)
            cprintf(" ");
    }
}


// Отобразить одну строку декодированного текста длины n
void draw_text_line(char* line, int n)
{
    for(int j = 0; j < n; j++)
    {
        char c = line[j];

        // "Пустые" символы отображаются точкой
        if(is_unused_char(c))
            c = '.';

        cprintf("%c", c);
    }
}


// Отрисовывает часть файла выбранную строками [begin:end] в координату y
// в три колонны (оффсет, байты и текст)
void draw_file(int y, unsigned long long begin, unsigned long long end)
{
    for(unsigned long long i = begin; i <= end; i++)
    {
        // Прочитать строку файла
        char line[16];
        SetFilePointer(file, i*16, NULL, FILE_BEGIN);
        DWORD r = 0;
        ReadFile(file, (void*)line, 16, &r, NULL);

        set_draw_pos(OFFSET_X, y+i-begin);
        draw_offset_line(i);

        set_draw_pos(BYTES_X, y+i-begin);
        draw_bytes_line(line, r);

        set_draw_pos(TEXT_X, y+i-begin);
        draw_text_line(line, r);
    }
}

// Меняет кодировку файла между двумя
void change_encoding()
{
    CHAR_INFO buf[CONTENT_SIZE];
    COORD size = {16, 28};
    COORD pos = {0, 0};
    SMALL_RECT region = {TEXT_X, TEXT_Y, TEXT_X+16, TEXT_Y+28};
    ReadConsoleOutput(consoleOut, buf, size, pos, &region);

    if(info.encoding == ENC_CP1251)
    {
        info.encoding = ENC_CP1252;
        SetConsoleOutputCP(1252);
    }
    else
    {
        info.encoding = ENC_CP1251;
        SetConsoleOutputCP(1251);
    }

    cursor_display(FALSE);

    draw_path(); // Если название файла не на латинице
    draw_enc();
    WriteConsoleOutput(consoleOut, buf, size, pos, &region);

    cursor_display(TRUE);
}


// Прокрутить выделенную область экрана
void scroll_area(int x1, int y1, int x2, int y2, int toX, int toY)
{
    SMALL_RECT from = {x1, y1, x2, y2};
    COORD to = {toX, toY};
    CHAR_INFO data;
    data.Attributes = TEXT_DEFAULT | BACK_DEFAULT;
    data.Char.AsciiChar = ' ';
    ScrollConsoleScreenBuffer(consoleOut, &from, NULL, to, &data);
}


// Прокручивает файл в нужную сторону
void scroll(int dir)
{
    // x1,y1,x2,y2 границы области
    // y - позиция для рисования новой строки
    // line - номер строка для замещения
    int x1, y1, x2, y2, toX, toY, y;
    unsigned long long line;

    // Выбор границ области для прокрутки
    if(dir == SCROLL_DOWN)
    {
        x1 = 0;
        y1 = 3;
        x2 = BUF_W;
        y2 = BUF_H;

        toX = 0;
        toY = 2;

        y = BUF_H-1;
        line = content.end-1;
    }
    else
    {
        x1 = 0;
        y1 = 2;
        x2 = BUF_W;
        y2 = BUF_H;

        toX = 0;
        toY = 3;

        y = 2;
        line = content.begin;
    }

    scroll_area(x1, y1, x2, y2, toX, toY);
    draw_file(y, line, line);
}


// Инициализация перед началом работы
void init()
{
    // Получить дескрипторы
    SECURITY_ATTRIBUTES attr={sizeof(SECURITY_ATTRIBUTES),NULL,1};
    consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;

    if(!GetConsoleMode(consoleOut,&mode))
        consoleOut = CreateFileA("CONOUT$",GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,&attr,OPEN_EXISTING,0,0);

    consoleIn = GetStdHandle(STD_INPUT_HANDLE);
    if(!GetConsoleMode(consoleIn,&mode))
        consoleIn=CreateFileA("CONIN$",GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,&attr,OPEN_EXISTING,0,0);

    handle = GetConsoleWindow();

    // Установить заголовок окна
    SetConsoleTitle(PROGRAM_TITLE);

    // Выключить возможность изменения размеров окна
    SetWindowLong(handle, GWL_STYLE, GetWindowLong(handle, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

    // Выключить курсор по умолчанию
    CONSOLE_CURSOR_INFO info = {1, FALSE};
    SetConsoleCursorInfo(consoleOut, &info);

    // Кодировка по умолчанию
    SetConsoleOutputCP(1252);
}

// Считать дату создания файла
void file_get_creation_date(FILE* file)
{
    FILETIME time;
    GetFileTime(file, &time, NULL, NULL);
    SYSTEMTIME date;
    FileTimeToSystemTime(&time, &date);
    sprintf(info.date, "%02d/%02d/%d %02d:%02d", date.wDay, date.wMonth, date.wYear, date.wHour, date.wMinute);
}

int main(int argc, char **argv)
{
    // Инициализация
    init();

    // Открытие файла
    file = CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file != INVALID_HANDLE_VALUE)
    {
        // Чтение данных
        info.encoding = ENC_CP1252;
        info.path = argv[1];
        info.size = GetFileSize(file, NULL);
        file_get_creation_date(file);

       // Корректно обработать пустые файлы
        if(info.size == 0)
        {
            CURSOR_BORDER_X = 0;
            CURSOR_BORDER_Y = 0;
        }
        else // Границы курсора определяемые из содержимого файла
        {
            CURSOR_BORDER_X = (info.size-1)%16;
            CURSOR_BORDER_Y = (info.size-1)/16;
        }

        // Границы содержимого, которое сейчас отображается (номера строк на которые делится файл)
        // В каждой строке по 16 байт, за исключением, быть может, последней
        content.begin = 0;
        content.end = 28;

        // Отображение
        draw_info();
        draw_headers();
        draw_file(OFFSET_Y, content.begin, content.end-1);

        cursor_display(TRUE);

        // Пока открыто окно
        while(1)
        {
            INPUT_RECORD ev;
            DWORD read;
            ReadConsoleInput(consoleIn, &ev, 1, &read);

            // Считывание нажатий
            if(ev.EventType == KEY_EVENT)
            {
                if(ev.Event.KeyEvent.bKeyDown == TRUE)
                {
                    // Если нажаты клавиши перемещения курсора
                    // То изменить положение курсора
                    if(ev.Event.KeyEvent.wVirtualKeyCode == VK_LEFT)
                        move_cursor(DIR_LEFT);
                    else if(ev.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT)
                        move_cursor(DIR_RIGHT);
                    else if(ev.Event.KeyEvent.wVirtualKeyCode == VK_DOWN)
                        move_cursor(DIR_DOWN);
                    else if(ev.Event.KeyEvent.wVirtualKeyCode == VK_UP)
                        move_cursor(DIR_UP);

                    // Или если нужно изменить кодировку
                    else if(ev.Event.KeyEvent.wVirtualKeyCode == 0x45) // Символ клавиши E (Encoding)
                        change_encoding();
                }
            }
        }
    }
    else
    {
        cprintf("ERROR: File not found!");
        while(1);
    }

    return 0;
}
