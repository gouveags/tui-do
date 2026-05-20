# Agent Instructions

This repository is developed with strict test-first discipline. Treat this file as binding.

## Test-First Workflow

- Always write or update tests before changing production code.
- Work red-green:
  1. Add the smallest test that captures the desired behavior or regression.
  2. Run the test and confirm it fails for the expected reason.
  3. Change production code until the test passes.
  4. Run the complete check loop before considering the work done.
- Do not weaken, delete, or casually rewrite tests to make implementation easier.
- If a test must change, the reason must be explicit: the product contract changed, the test was wrong, or the test was asserting implementation details that block the intended behavior.
- Regression fixes must include a regression test that would have caught the bug.
- New UI behavior must have tests for the visible contract, not only implementation helpers.

## Complete Check Loop

Run the complete loop before every handoff, commit, or push:

```sh
make check
```

`make check` runs:

- formatting verification
- linting
- unit tests
- production build
- terminal smoke run

If the loop fails, fix the failure before moving on.

## Formatting And Linting

- Run `make format` before final verification when files have changed.
- Run `make format-check` to verify whitespace and final-newline rules.
- Run `make lint` to compile with warnings treated as errors.
- Keep formatting changes separate from behavioral intent when possible.

## Terminal Feedback

This is a terminal app. Build and operate it in the terminal at every meaningful step.

- Use `make run` for interactive checks.
- Use `make smoke` for an automated terminal startup/shutdown check.
- When changing layout, rendering, input, terminal sizing, raw mode, or colors, verify the real terminal behavior, not just the unit tests.

## Design Approach

- Keep the app small and direct.
- Prefer visible, testable contracts over clever abstractions.
- Keep Clay layout code in `src/ui.*`.
- Keep ANSI, raw mode, sizing, and render output code in `src/terminal.*`.
- Keep `src/main.c` focused on app lifecycle and state flow.
