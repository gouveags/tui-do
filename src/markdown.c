#include "markdown.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const char *skip_spaces(const char *text) {
    while (*text == ' ' || *text == '\t') {
        text++;
    }

    return text;
}

static void copy_text(char *out, size_t out_size, const char *text, size_t length) {
    while (length > 0 && (text[length - 1] == ' ' || text[length - 1] == '\t' || text[length - 1] == '\r')) {
        length--;
    }
    if (length >= out_size) {
        length = out_size - 1;
    }

    memcpy(out, text, length);
    out[length] = '\0';
}

static void markdown_add_block(MarkdownDocument *document, MarkdownBlockType type, const char *text, size_t length) {
    MarkdownBlock *block;

    if (document->block_count >= MARKDOWN_MAX_BLOCKS) {
        return;
    }

    block = &document->blocks[document->block_count++];
    block->type = type;
    copy_text(block->text, sizeof(block->text), text, length);
}

static int is_thematic_break(const char *text, size_t length) {
    char marker = '\0';
    int count = 0;

    for (size_t i = 0; i < length; i++) {
        if (text[i] == ' ' || text[i] == '\t' || text[i] == '\r') {
            continue;
        }
        if (text[i] != '-' && text[i] != '_' && text[i] != '*') {
            return 0;
        }
        if (marker == '\0') {
            marker = text[i];
        }
        if (text[i] != marker) {
            return 0;
        }
        count++;
    }

    return count >= 3;
}

static int parse_heading(const char *text, size_t length, MarkdownBlockType *type, const char **content, size_t *content_length) {
    size_t level = 0;

    while (level < length && level < 6 && text[level] == '#') {
        level++;
    }
    if (level == 0 || level > 3 || level >= length || text[level] != ' ') {
        return 0;
    }

    *type = level == 1 ? MARKDOWN_BLOCK_HEADING_1 : level == 2 ? MARKDOWN_BLOCK_HEADING_2 : MARKDOWN_BLOCK_HEADING_3;
    *content = skip_spaces(text + level);
    *content_length = length - (size_t)(*content - text);
    return 1;
}

static int parse_unordered_item(const char *text, size_t length, MarkdownBlockType *type, const char **content, size_t *content_length) {
    if (length < 2 || (text[0] != '-' && text[0] != '*' && text[0] != '+') || text[1] != ' ') {
        return 0;
    }

    *type = MARKDOWN_BLOCK_UNORDERED_LIST_ITEM;
    *content = skip_spaces(text + 2);
    *content_length = length - (size_t)(*content - text);

    if (*content_length >= 4 && (*content)[0] == '[' && ((*content)[1] == ' ' || (*content)[1] == 'x' || (*content)[1] == 'X') && (*content)[2] == ']' && (*content)[3] == ' ') {
        *type = (*content)[1] == ' ' ? MARKDOWN_BLOCK_TASK_OPEN : MARKDOWN_BLOCK_TASK_DONE;
        *content = skip_spaces(*content + 4);
        *content_length = length - (size_t)(*content - text);
    }

    return 1;
}

static int parse_ordered_item(const char *text, size_t length, const char **content, size_t *content_length) {
    size_t index = 0;

    while (index < length && isdigit((unsigned char)text[index])) {
        index++;
    }
    if (index == 0 || index + 1 >= length || (text[index] != '.' && text[index] != ')') || text[index + 1] != ' ') {
        return 0;
    }

    *content = skip_spaces(text + index + 2);
    *content_length = length - (size_t)(*content - text);
    return 1;
}

MarkdownDocument markdown_parse(const char *markdown) {
    MarkdownDocument document = {0};
    int in_code = 0;
    const char *line_start = markdown == NULL ? "" : markdown;

    for (size_t i = 0;; i++) {
        char value = line_start[i];
        if (value != '\n' && value != '\0') {
            continue;
        }

        const char *line = line_start;
        size_t length = i;
        const char *trimmed = skip_spaces(line);
        size_t trimmed_length = length - (size_t)(trimmed - line);
        MarkdownBlockType type;
        const char *content;
        size_t content_length;

        if (value == '\0' && length == 0) {
            break;
        }

        if (trimmed_length >= 3 && strncmp(trimmed, "```", 3) == 0) {
            in_code = !in_code;
        } else if (in_code) {
            markdown_add_block(&document, MARKDOWN_BLOCK_CODE, line, length);
        } else if (trimmed_length == 0 || (trimmed_length == 1 && trimmed[0] == '\r')) {
            markdown_add_block(&document, MARKDOWN_BLOCK_BLANK, "", 0);
        } else if (parse_heading(trimmed, trimmed_length, &type, &content, &content_length)) {
            markdown_add_block(&document, type, content, content_length);
        } else if (trimmed[0] == '>' && trimmed_length >= 2) {
            content = skip_spaces(trimmed + 1);
            markdown_add_block(&document, MARKDOWN_BLOCK_QUOTE, content, trimmed_length - (size_t)(content - trimmed));
        } else if (parse_unordered_item(trimmed, trimmed_length, &type, &content, &content_length)) {
            markdown_add_block(&document, type, content, content_length);
        } else if (parse_ordered_item(trimmed, trimmed_length, &content, &content_length)) {
            markdown_add_block(&document, MARKDOWN_BLOCK_ORDERED_LIST_ITEM, content, content_length);
        } else if (is_thematic_break(trimmed, trimmed_length)) {
            markdown_add_block(&document, MARKDOWN_BLOCK_THEMATIC_BREAK, "", 0);
        } else {
            markdown_add_block(&document, MARKDOWN_BLOCK_PARAGRAPH, trimmed, trimmed_length);
        }

        if (value == '\0') {
            break;
        }
        line_start += i + 1;
        i = (size_t)-1;
    }

    return document;
}

const char *markdown_block_type_name(MarkdownBlockType type) {
    switch (type) {
        case MARKDOWN_BLOCK_BLANK:
            return "blank";
        case MARKDOWN_BLOCK_PARAGRAPH:
            return "paragraph";
        case MARKDOWN_BLOCK_HEADING_1:
            return "heading_1";
        case MARKDOWN_BLOCK_HEADING_2:
            return "heading_2";
        case MARKDOWN_BLOCK_HEADING_3:
            return "heading_3";
        case MARKDOWN_BLOCK_QUOTE:
            return "quote";
        case MARKDOWN_BLOCK_UNORDERED_LIST_ITEM:
            return "unordered_list_item";
        case MARKDOWN_BLOCK_ORDERED_LIST_ITEM:
            return "ordered_list_item";
        case MARKDOWN_BLOCK_TASK_OPEN:
            return "task_open";
        case MARKDOWN_BLOCK_TASK_DONE:
            return "task_done";
        case MARKDOWN_BLOCK_CODE:
            return "code";
        case MARKDOWN_BLOCK_THEMATIC_BREAK:
            return "thematic_break";
    }

    return "unknown";
}
