extern void screen_write_character_with_color(unsigned int character, unsigned int color);
extern void screen_set_cursor_pos(unsigned int pos);
extern void screen_move_cursor_up(unsigned int amount);
extern void screen_move_cursor_down(unsigned int amount);
extern void screen_move_cursor_left(unsigned int amount);
extern void screen_move_cursor_right(unsigned int amount);
extern unsigned int keyboard_get_key(void);
extern void fat8_load_root_dir(void);

extern unsigned char fat8_read_file(const char *filename, unsigned int load_addr);

#define MENU_ITEM_LENGTH 8
#define ROOT_LOAD_ADDR 0x3e00

char i = 0;
char j = 0;
unsigned int pos = 0;
unsigned int current_key = 0;

unsigned char menu_height = 20;
unsigned char menu_width = 72;

unsigned char *root_dir = 0;
unsigned char *file_pointer = 0;

unsigned char selected_item = 0;
unsigned char num_menu_items = 5;
char menu_items[MENU_ITEM_LENGTH][20] = {
    "12345678",
    "12345678",
    "12345678",
    "12345678",
    "12345678",
};

void draw_background()
{
    // Reset cursor to top-left
    screen_set_cursor_pos((unsigned int)0x0000);

    for (i = 0; i < 26; ++i)
    {
        j = 0;
        for (j = 0; j < 80; ++j)
        {
            screen_write_character_with_color((unsigned int)0, (unsigned int)0x40);
        }
        screen_set_cursor_pos((unsigned int)0x0100 * i);
    }

    return;
}

void draw_menu()
{
    // Reset cursor to top-left of our window
    screen_set_cursor_pos((unsigned int)0x0204);

    // Draw menu
    for (i = 0; i < menu_height; i++)
    {
        j = 0;
        for (j = 0; j < menu_width; j++)
        {
            screen_write_character_with_color((unsigned int)0, (unsigned int)0x70);
        }
        screen_set_cursor_pos((unsigned int)0x0100 * i + 0x0204);
    }

    // Draw menu shadow
    screen_move_cursor_right(1);
    for (j = 0; j < menu_width + 1; ++j)
    {
        screen_write_character_with_color((unsigned int)0, (unsigned int)0x80);
    }
    screen_move_cursor_up(1);
    screen_move_cursor_left(2);
    for (i = 0; i < menu_height - 2; ++i)
    {
        screen_write_character_with_color((unsigned int)0, (unsigned int)0x80);
        screen_write_character_with_color((unsigned int)0, (unsigned int)0x80);
        screen_move_cursor_up(1);
        screen_move_cursor_left(2);
    }

    return;
}

void draw_item(int index, int color)
{
    for (j = 0; j < MENU_ITEM_LENGTH; ++j)
    {
        screen_write_character_with_color((unsigned int)menu_items[index][j], color);
    }
}

void draw_menu_items()
{
    // Reset cursor to top-left of our window
    screen_set_cursor_pos((unsigned int)0x0305);

    for (i = 0; i < num_menu_items; ++i)
    {
        if (i == selected_item)
        {
            draw_item(i, 0x89);
        }
        else
        {
            draw_item(i, 0x79);
        }

        screen_set_cursor_pos((unsigned int)0x0305);
        screen_move_cursor_down((i + 1) * 2);
    }
}

void main()
{
    draw_background();
    draw_menu();

    fat8_load_root_dir();

    root_dir = ROOT_LOAD_ADDR;
    for (i = 0; i < num_menu_items; ++i)
    {
        for (j = 0; j < MENU_ITEM_LENGTH - 1; ++j)
        {
            menu_items[i][j] = root_dir[j + i * MENU_ITEM_LENGTH];
        }
        menu_items[i][j] = 0;
    }

    while (1)
    {

        draw_menu_items();

        current_key = keyboard_get_key();

        if ((current_key & 0x00FF) == 13)
        {
            screen_set_cursor_pos(0);

            fat8_read_file(menu_items[selected_item], 0x9000);
            file_pointer = 0x9000;
            for (i = 0; i < 33; ++i)
            {
                screen_write_character_with_color((unsigned int)file_pointer[i], 0x79);
            }
            break;
        }
        else if (current_key == 0x4800)
        {
            if (selected_item == 0)
            {
                selected_item = num_menu_items - 1;
            }
            else
            {
                selected_item = selected_item - 1;
            }
        }
        else if (current_key == 0x5000)
        {
            if (selected_item == num_menu_items - 1)
            {
                selected_item = 0;
            }
            else
            {
                selected_item = selected_item + 1;
            }
        }
    }

    return;
}
