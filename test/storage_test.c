#define _POSIX_C_SOURCE 200809L

#include "test.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../src/storage.h"

static const char *test_root = "build/storage-test";

static int file_exists(const char *path) {
    struct stat info;
    return stat(path, &info) == 0;
}

static void reset_storage_root(void) {
    system("rm -rf build/storage-test");
}

static Todo sample_todo(void) {
    Todo todo = {
        .id = "goal-alpha",
        .title = "Ship local storage",
        .created_at = 100,
        .updated_at = 200,
        .item_count = 2,
    };

    snprintf(todo.items[0].id, sizeof(todo.items[0].id), "%s", "item-plan");
    snprintf(todo.items[0].text, sizeof(todo.items[0].text), "%s", "Write the storage plan");
    todo.items[0].done = 1;
    todo.items[0].has_markdown = 1;
    snprintf(todo.items[0].markdown, sizeof(todo.items[0].markdown), "%s", "# Storage plan\n\nKeep goals agent-readable.\n");

    snprintf(todo.items[1].id, sizeof(todo.items[1].id), "%s", "item-code");
    snprintf(todo.items[1].text, sizeof(todo.items[1].text), "%s", "Implement the storage backend");
    todo.items[1].done = 0;

    return todo;
}

SCENARIO(saving_a_todo_writes_index_metadata_and_item_markdown) {
    Todo todo = sample_todo();
    TodoIndex index = {0};
    char markdown_path[TODO_PATH_MAX];
    char tmp_path[TODO_PATH_MAX + 4];

    GIVEN("a todo with item-level markdown context");
    reset_storage_root();

    WHEN("the todo is saved");
    EXPECT_INT_EQ(storage_save_todo(test_root, &todo), 0);

    THEN("the index can be loaded without scanning every todo file");
    EXPECT_INT_EQ(storage_load_index(test_root, &index), 0);
    EXPECT_INT_EQ((int)index.entry_count, 1);
    EXPECT_TRUE(strcmp(index.entries[0].id, "goal-alpha") == 0);
    EXPECT_TRUE(strcmp(index.entries[0].title, "Ship local storage") == 0);
    EXPECT_INT_EQ((int)index.entries[0].item_count, 2);
    EXPECT_INT_EQ((int)index.entries[0].done_count, 1);

    THEN("the item's markdown file is stored beside that todo's item files");
    EXPECT_INT_EQ(storage_markdown_path(test_root, "goal-alpha", "item-plan", markdown_path, sizeof(markdown_path)), 0);
    EXPECT_TRUE(file_exists(markdown_path));

    THEN("atomic temp files are not left behind");
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", markdown_path);
    EXPECT_TRUE(!file_exists(tmp_path));
}

SCENARIO(saving_a_todo_creates_a_default_entry_note) {
    Todo todo = {
        .id = "entry-note",
        .title = "Write richer notes",
        .created_at = 100,
        .updated_at = 100,
    };
    char note_path[TODO_PATH_MAX];

    GIVEN("a todo without item-level markdown");
    reset_storage_root();

    WHEN("the todo is saved");
    EXPECT_INT_EQ(storage_save_todo(test_root, &todo), 0);

    THEN("a default markdown note exists for the entry itself");
    EXPECT_INT_EQ(storage_todo_note_path(test_root, "entry-note", note_path, sizeof(note_path)), 0);
    EXPECT_TRUE(file_exists(note_path));
}

SCENARIO(todo_note_can_be_loaded_and_saved) {
    Todo todo = {
        .id = "entry-note-edit",
        .title = "Edit markdown",
        .created_at = 100,
        .updated_at = 100,
    };
    char note[TODO_NOTE_MAX];

    GIVEN("a todo note exists");
    reset_storage_root();
    EXPECT_INT_EQ(storage_save_todo(test_root, &todo), 0);

    WHEN("the note is replaced");
    EXPECT_INT_EQ(storage_save_todo_note(test_root, "entry-note-edit", "# Plan\n\nWrite it down.\n"), 0);

    THEN("the updated markdown can be loaded");
    EXPECT_INT_EQ(storage_load_todo_note(test_root, "entry-note-edit", note, sizeof(note)), 0);
    EXPECT_TRUE(strcmp(note, "# Plan\n\nWrite it down.\n") == 0);
}

SCENARIO(saving_a_todo_updates_the_existing_index_entry) {
    Todo todo = sample_todo();
    TodoIndex index = {0};

    GIVEN("a todo that has already been indexed");
    reset_storage_root();
    EXPECT_INT_EQ(storage_save_todo(test_root, &todo), 0);

    WHEN("the same todo is saved with new metadata");
    snprintf(todo.title, sizeof(todo.title), "%s", "Ship fast local storage");
    todo.updated_at = 300;
    todo.items[1].done = 1;
    EXPECT_INT_EQ(storage_save_todo(test_root, &todo), 0);

    THEN("the index entry is updated in place instead of duplicated");
    EXPECT_INT_EQ(storage_load_index(test_root, &index), 0);
    EXPECT_INT_EQ((int)index.entry_count, 1);
    EXPECT_TRUE(strcmp(index.entries[0].title, "Ship fast local storage") == 0);
    EXPECT_INT_EQ((int)index.entries[0].updated_at, 300);
    EXPECT_INT_EQ((int)index.entries[0].done_count, 2);
}

int main(void) {
    RUN_SCENARIO(saving_a_todo_writes_index_metadata_and_item_markdown);
    RUN_SCENARIO(saving_a_todo_creates_a_default_entry_note);
    RUN_SCENARIO(todo_note_can_be_loaded_and_saved);
    RUN_SCENARIO(saving_a_todo_updates_the_existing_index_entry);

    return finish_tests();
}
