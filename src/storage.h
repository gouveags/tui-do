#ifndef TUI_DO_STORAGE_H
#define TUI_DO_STORAGE_H

#include <stddef.h>

#define TODO_ID_MAX 32
#define TODO_TITLE_MAX 128
#define TODO_TEXT_MAX 256
#define TODO_NOTE_MAX 4096
#define TODO_PATH_MAX 512
#define TODO_MAX_ITEMS 128
#define TODO_INDEX_MAX_ENTRIES 1024

typedef struct TodoItem {
    char id[TODO_ID_MAX];
    char text[TODO_TEXT_MAX];
    int done;
    int has_markdown;
    char markdown[TODO_TEXT_MAX];
} TodoItem;

typedef struct Todo {
    char id[TODO_ID_MAX];
    char title[TODO_TITLE_MAX];
    long long created_at;
    long long updated_at;
    size_t item_count;
    TodoItem items[TODO_MAX_ITEMS];
} Todo;

typedef struct TodoIndexEntry {
    char id[TODO_ID_MAX];
    char title[TODO_TITLE_MAX];
    long long created_at;
    long long updated_at;
    size_t item_count;
    size_t done_count;
} TodoIndexEntry;

typedef struct TodoIndex {
    size_t entry_count;
    TodoIndexEntry entries[TODO_INDEX_MAX_ENTRIES];
} TodoIndex;

int storage_save_todo(const char *root, const Todo *todo);
int storage_load_index(const char *root, TodoIndex *index);
int storage_todo_note_path(const char *root, const char *todo_id, char *out, size_t out_size);
int storage_load_todo_note(const char *root, const char *todo_id, char *out, size_t out_size);
int storage_save_todo_note(const char *root, const char *todo_id, const char *content);
int storage_markdown_path(const char *root, const char *todo_id, const char *item_id, char *out, size_t out_size);

#endif
