#include "test.h"

#include <string.h>

#include "../src/markdown.h"

static void expect_block(MarkdownDocument *document, int index, MarkdownBlockType type, const char *text) {
    EXPECT_INT_EQ((int)document->blocks[index].type, (int)type);
    EXPECT_TRUE(strcmp(document->blocks[index].text, text) == 0);
}

SCENARIO(markdown_parser_translates_block_rules_to_styles) {
    GIVEN("markdown using common block constructs");

    WHEN("the markdown is parsed");
    MarkdownDocument document = markdown_parse(
        "# Title\n"
        "## Section\n"
        "### Detail\n"
        "\n"
        "Paragraph text\n"
        "- Bullet\n"
        "1. Ordered\n"
        "- [ ] Open task\n"
        "- [x] Done task\n"
        "> Quoted text\n"
        "---\n"
        "```\n"
        "# not a heading\n"
        "```\n"
    );

    THEN("each markdown rule becomes an explicit style block");
    EXPECT_INT_EQ((int)document.block_count, 12);
    expect_block(&document, 0, MARKDOWN_BLOCK_HEADING_1, "Title");
    expect_block(&document, 1, MARKDOWN_BLOCK_HEADING_2, "Section");
    expect_block(&document, 2, MARKDOWN_BLOCK_HEADING_3, "Detail");
    expect_block(&document, 3, MARKDOWN_BLOCK_BLANK, "");
    expect_block(&document, 4, MARKDOWN_BLOCK_PARAGRAPH, "Paragraph text");
    expect_block(&document, 5, MARKDOWN_BLOCK_UNORDERED_LIST_ITEM, "Bullet");
    expect_block(&document, 6, MARKDOWN_BLOCK_ORDERED_LIST_ITEM, "Ordered");
    expect_block(&document, 7, MARKDOWN_BLOCK_TASK_OPEN, "Open task");
    expect_block(&document, 8, MARKDOWN_BLOCK_TASK_DONE, "Done task");
    expect_block(&document, 9, MARKDOWN_BLOCK_QUOTE, "Quoted text");
    expect_block(&document, 10, MARKDOWN_BLOCK_THEMATIC_BREAK, "");
    expect_block(&document, 11, MARKDOWN_BLOCK_CODE, "# not a heading");
}

SCENARIO(markdown_parser_keeps_soft_line_breaks_as_separate_paragraphs) {
    GIVEN("plain text split by newline characters");

    WHEN("the markdown is parsed");
    MarkdownDocument document = markdown_parse("First line\nSecond line\n");

    THEN("each newline creates another renderable block");
    EXPECT_INT_EQ((int)document.block_count, 2);
    expect_block(&document, 0, MARKDOWN_BLOCK_PARAGRAPH, "First line");
    expect_block(&document, 1, MARKDOWN_BLOCK_PARAGRAPH, "Second line");
}

int main(void) {
    RUN_SCENARIO(markdown_parser_translates_block_rules_to_styles);
    RUN_SCENARIO(markdown_parser_keeps_soft_line_breaks_as_separate_paragraphs);

    return finish_tests();
}
