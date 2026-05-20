#ifndef TUI_DO_MARKDOWN_H
#define TUI_DO_MARKDOWN_H

#include <stddef.h>

#define MARKDOWN_TEXT_MAX 256
#define MARKDOWN_MAX_BLOCKS 128

typedef enum MarkdownBlockType {
    MARKDOWN_BLOCK_BLANK,
    MARKDOWN_BLOCK_PARAGRAPH,
    MARKDOWN_BLOCK_HEADING_1,
    MARKDOWN_BLOCK_HEADING_2,
    MARKDOWN_BLOCK_HEADING_3,
    MARKDOWN_BLOCK_QUOTE,
    MARKDOWN_BLOCK_UNORDERED_LIST_ITEM,
    MARKDOWN_BLOCK_ORDERED_LIST_ITEM,
    MARKDOWN_BLOCK_TASK_OPEN,
    MARKDOWN_BLOCK_TASK_DONE,
    MARKDOWN_BLOCK_CODE,
    MARKDOWN_BLOCK_THEMATIC_BREAK,
} MarkdownBlockType;

typedef struct MarkdownBlock {
    MarkdownBlockType type;
    char text[MARKDOWN_TEXT_MAX];
} MarkdownBlock;

typedef struct MarkdownDocument {
    size_t block_count;
    MarkdownBlock blocks[MARKDOWN_MAX_BLOCKS];
} MarkdownDocument;

MarkdownDocument markdown_parse(const char *markdown);
const char *markdown_block_type_name(MarkdownBlockType type);

#endif
