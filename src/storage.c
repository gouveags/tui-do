#include "storage.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int ensure_dir(const char *path) {
    if (mkdir(path, 0700) == 0 || errno == EEXIST) {
        return 0;
    }

    return -1;
}

static int join_path(char *out, size_t out_size, const char *a, const char *b) {
    int written = snprintf(out, out_size, "%s/%s", a, b);
    return written > 0 && (size_t)written < out_size ? 0 : -1;
}

static int write_file_atomic(const char *path, const char *content) {
    char tmp_path[TODO_PATH_MAX];
    FILE *file;
    int written = snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path);

    if (written <= 0 || (size_t)written >= sizeof(tmp_path)) {
        return -1;
    }

    file = fopen(tmp_path, "wb");
    if (file == NULL) {
        return -1;
    }

    if (fputs(content, file) == EOF) {
        fclose(file);
        unlink(tmp_path);
        return -1;
    }

    if (fflush(file) != 0) {
        fclose(file);
        unlink(tmp_path);
        return -1;
    }

    if (fclose(file) != 0) {
        unlink(tmp_path);
        return -1;
    }

    if (rename(tmp_path, path) != 0) {
        unlink(tmp_path);
        return -1;
    }

    return 0;
}

static int write_file_if_missing(const char *path, const char *content) {
    if (access(path, F_OK) == 0) {
        return 0;
    }

    return write_file_atomic(path, content);
}

static size_t todo_done_count(const Todo *todo) {
    size_t done_count = 0;

    for (size_t i = 0; i < todo->item_count; i++) {
        if (todo->items[i].done) {
            done_count++;
        }
    }

    return done_count;
}

static int ensure_storage_dirs(const char *root, char *lists_dir, size_t lists_dir_size) {
    if (ensure_dir(root) != 0) {
        return -1;
    }

    if (join_path(lists_dir, lists_dir_size, root, "lists") != 0) {
        return -1;
    }

    return ensure_dir(lists_dir);
}

static int ensure_todo_dirs(const char *root, const char *todo_id, char *todo_dir, size_t todo_dir_size, char *items_dir, size_t items_dir_size) {
    char lists_dir[TODO_PATH_MAX];

    if (ensure_storage_dirs(root, lists_dir, sizeof(lists_dir)) != 0) {
        return -1;
    }
    if (join_path(todo_dir, todo_dir_size, lists_dir, todo_id) != 0 || ensure_dir(todo_dir) != 0) {
        return -1;
    }
    if (join_path(items_dir, items_dir_size, todo_dir, "items") != 0 || ensure_dir(items_dir) != 0) {
        return -1;
    }

    return 0;
}

int storage_markdown_path(const char *root, const char *todo_id, const char *item_id, char *out, size_t out_size) {
    char lists_dir[TODO_PATH_MAX];
    char todo_dir[TODO_PATH_MAX];
    char items_dir[TODO_PATH_MAX];
    char markdown_file[TODO_PATH_MAX];

    if (
        join_path(lists_dir, sizeof(lists_dir), root, "lists") != 0 ||
        join_path(todo_dir, sizeof(todo_dir), lists_dir, todo_id) != 0 ||
        join_path(items_dir, sizeof(items_dir), todo_dir, "items") != 0
    ) {
        return -1;
    }

    if (snprintf(markdown_file, sizeof(markdown_file), "%s.md", item_id) <= 0) {
        return -1;
    }

    return join_path(out, out_size, items_dir, markdown_file);
}

int storage_todo_note_path(const char *root, const char *todo_id, char *out, size_t out_size) {
    char lists_dir[TODO_PATH_MAX];
    char todo_dir[TODO_PATH_MAX];

    if (
        join_path(lists_dir, sizeof(lists_dir), root, "lists") != 0 ||
        join_path(todo_dir, sizeof(todo_dir), lists_dir, todo_id) != 0
    ) {
        return -1;
    }

    return join_path(out, out_size, todo_dir, "notes.md");
}

int storage_load_todo_note(const char *root, const char *todo_id, char *out, size_t out_size) {
    char path[TODO_PATH_MAX];
    FILE *file;
    size_t bytes_read;

    if (out == NULL || out_size == 0) {
        return -1;
    }
    out[0] = '\0';
    if (storage_todo_note_path(root, todo_id, path, sizeof(path)) != 0) {
        return -1;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }

    bytes_read = fread(out, 1, out_size - 1, file);
    out[bytes_read] = '\0';
    fclose(file);
    return 0;
}

int storage_save_todo_note(const char *root, const char *todo_id, const char *content) {
    char path[TODO_PATH_MAX];

    if (content == NULL || storage_todo_note_path(root, todo_id, path, sizeof(path)) != 0) {
        return -1;
    }

    return write_file_atomic(path, content);
}

static int save_default_note_file(const char *root, const Todo *todo) {
    char path[TODO_PATH_MAX];
    char content[TODO_TEXT_MAX + TODO_TITLE_MAX];
    int written;

    if (storage_todo_note_path(root, todo->id, path, sizeof(path)) != 0) {
        return -1;
    }

    written = snprintf(content, sizeof(content), "# %s\n\n", todo->title);
    if (written <= 0 || (size_t)written >= sizeof(content)) {
        return -1;
    }

    return write_file_if_missing(path, content);
}

static int save_todo_file(const char *todo_dir, const Todo *todo) {
    char path[TODO_PATH_MAX];
    char buffer[32768];
    size_t offset = 0;
    int written;

    if (join_path(path, sizeof(path), todo_dir, "todo.tsv") != 0) {
        return -1;
    }

    written = snprintf(
        buffer,
        sizeof(buffer),
        "id\t%s\n"
        "title\t%s\n"
        "created_at\t%lld\n"
        "updated_at\t%lld\n"
        "items\n",
        todo->id,
        todo->title,
        todo->created_at,
        todo->updated_at
    );
    if (written <= 0 || (size_t)written >= sizeof(buffer)) {
        return -1;
    }
    offset = (size_t)written;

    for (size_t i = 0; i < todo->item_count; i++) {
        const TodoItem *item = &todo->items[i];
        written = snprintf(
            buffer + offset,
            sizeof(buffer) - offset,
            "%s\t%d\t%s\t%s\n",
            item->id,
            item->done,
            item->has_markdown ? "markdown" : "",
            item->text
        );
        if (written <= 0 || (size_t)written >= sizeof(buffer) - offset) {
            return -1;
        }
        offset += (size_t)written;
    }

    return write_file_atomic(path, buffer);
}

static int save_markdown_files(const char *root, const Todo *todo) {
    char path[TODO_PATH_MAX];

    for (size_t i = 0; i < todo->item_count; i++) {
        const TodoItem *item = &todo->items[i];
        if (!item->has_markdown) {
            continue;
        }
        if (storage_markdown_path(root, todo->id, item->id, path, sizeof(path)) != 0) {
            return -1;
        }
        if (write_file_atomic(path, item->markdown) != 0) {
            return -1;
        }
    }

    return 0;
}

static int write_index_with_todo(const char *root, const Todo *todo) {
    TodoIndex index = {0};
    char path[TODO_PATH_MAX];
    char buffer[65536];
    size_t offset = 0;
    int found = 0;
    int written;

    storage_load_index(root, &index);

    for (size_t i = 0; i < index.entry_count; i++) {
        if (strcmp(index.entries[i].id, todo->id) == 0) {
            snprintf(index.entries[i].title, sizeof(index.entries[i].title), "%s", todo->title);
            index.entries[i].created_at = todo->created_at;
            index.entries[i].updated_at = todo->updated_at;
            index.entries[i].item_count = todo->item_count;
            index.entries[i].done_count = todo_done_count(todo);
            found = 1;
            break;
        }
    }

    if (!found) {
        TodoIndexEntry *entry;
        if (index.entry_count >= TODO_INDEX_MAX_ENTRIES) {
            return -1;
        }
        entry = &index.entries[index.entry_count++];
        snprintf(entry->id, sizeof(entry->id), "%s", todo->id);
        snprintf(entry->title, sizeof(entry->title), "%s", todo->title);
        entry->created_at = todo->created_at;
        entry->updated_at = todo->updated_at;
        entry->item_count = todo->item_count;
        entry->done_count = todo_done_count(todo);
    }

    written = snprintf(buffer, sizeof(buffer), "version\t1\n");
    if (written <= 0 || (size_t)written >= sizeof(buffer)) {
        return -1;
    }
    offset = (size_t)written;

    for (size_t i = 0; i < index.entry_count; i++) {
        TodoIndexEntry *entry = &index.entries[i];
        written = snprintf(
            buffer + offset,
            sizeof(buffer) - offset,
            "%s\t%s\t%lld\t%lld\t%zu\t%zu\n",
            entry->id,
            entry->title,
            entry->created_at,
            entry->updated_at,
            entry->item_count,
            entry->done_count
        );
        if (written <= 0 || (size_t)written >= sizeof(buffer) - offset) {
            return -1;
        }
        offset += (size_t)written;
    }

    if (join_path(path, sizeof(path), root, "index.tsv") != 0) {
        return -1;
    }

    return write_file_atomic(path, buffer);
}

int storage_save_todo(const char *root, const Todo *todo) {
    char todo_dir[TODO_PATH_MAX];
    char items_dir[TODO_PATH_MAX];

    if (todo == NULL || todo->item_count > TODO_MAX_ITEMS) {
        return -1;
    }
    if (ensure_todo_dirs(root, todo->id, todo_dir, sizeof(todo_dir), items_dir, sizeof(items_dir)) != 0) {
        return -1;
    }
    if (save_todo_file(todo_dir, todo) != 0) {
        return -1;
    }
    if (save_default_note_file(root, todo) != 0) {
        return -1;
    }
    if (save_markdown_files(root, todo) != 0) {
        return -1;
    }

    return write_index_with_todo(root, todo);
}

int storage_load_index(const char *root, TodoIndex *index) {
    char path[TODO_PATH_MAX];
    FILE *file;
    char line[1024];

    if (index == NULL) {
        return -1;
    }

    index->entry_count = 0;
    if (join_path(path, sizeof(path), root, "index.tsv") != 0) {
        return -1;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        TodoIndexEntry *entry;
        char *field;

        if (strncmp(line, "version\t", 8) == 0 || line[0] == '\n') {
            continue;
        }
        if (index->entry_count >= TODO_INDEX_MAX_ENTRIES) {
            fclose(file);
            return -1;
        }

        entry = &index->entries[index->entry_count];
        field = strtok(line, "\t");
        if (field == NULL) {
            continue;
        }
        snprintf(entry->id, sizeof(entry->id), "%s", field);

        field = strtok(NULL, "\t");
        if (field == NULL) {
            continue;
        }
        snprintf(entry->title, sizeof(entry->title), "%s", field);

        field = strtok(NULL, "\t");
        entry->created_at = field == NULL ? 0 : atoll(field);

        field = strtok(NULL, "\t");
        entry->updated_at = field == NULL ? 0 : atoll(field);

        field = strtok(NULL, "\t");
        entry->item_count = field == NULL ? 0 : (size_t)atoll(field);

        field = strtok(NULL, "\t\n");
        entry->done_count = field == NULL ? 0 : (size_t)atoll(field);

        index->entry_count++;
    }

    fclose(file);
    return 0;
}
