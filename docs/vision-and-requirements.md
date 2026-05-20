# tui-do Vision And Requirements

## Product Vision

`tui-do` should become a fast terminal notebook for work, memory, and execution.

The product is inspired by a paper desk notebook: every request, bug, idea, observation, follow-up, and work note can be captured immediately without ceremony. The software should preserve the frictionless feeling of writing in a notebook while fixing the parts that paper makes hard:

- dates are captured automatically
- messy pages become structured, searchable records
- old unfinished work resurfaces instead of disappearing
- related notes, subtasks, dependencies, and context stay connected
- agents and CLIs can read and update the same source of truth

The app should feel like a continuous catalog of what happened, what matters now, and what should happen next.

## Core Principles

- **Capture must be instant.** Adding a note or task should be faster than deciding where it belongs.
- **Everything is dated.** Creation, updates, completion, scheduled review, due dates, and priority recomputation should all be explicit.
- **Search must be powerful.** The user should be able to find old thoughts with fuzzy matching, filters, dates, status, tags, and indexed history.
- **Notes are first-class.** A task can be simple, but it can also own rich markdown context.
- **Tasks can become work objects.** A task may have subtasks, dependencies, linked tasks, markdown notes, and agent execution history.
- **The terminal UI and CLI share one model.** Humans and LLM agents should operate on the same storage and contracts.
- **Performance matters.** The app should stay fast with years of notebook entries by using compact indexes and C-friendly data structures.
- **Local-first by default.** The local filesystem is the source of truth unless a future sync layer is deliberately added.

## Main Use Cases

### Capture Inbox

The user needs a low-friction place to capture:

- requests from other people
- bugs spotted during work
- ideas for later
- meeting notes
- partial thoughts
- links and references
- follow-ups
- work logs
- decisions
- questions for future investigation

Capture should require minimal structure at first. The app can enrich records later.

### Daily Work Notebook

The app should support a daily flow:

- open today
- see ranked open work
- capture new notes/tasks
- mark progress
- complete or defer tasks
- review unfinished work from previous days
- leave markdown context for future self or agents

### Search And Retrieval

The app should make old work easy to recover:

- fuzzy search across titles, notes, item text, tags, and markdown summaries
- filter by created date, updated date, due date, completed date, status, and priority
- search only open tasks, completed tasks, blocked tasks, or all historical records
- search by linked task/dependency relationships
- keep indexes that make repeated searches fast

### Agentic Execution

The long-term goal is for LLM agents, including tools like Codex, to interact with the system:

- list open goals
- inspect task context and markdown notes
- create plans
- update progress
- create subtasks
- mark blockers
- attach execution logs
- rank work
- dispatch multiple tasks concurrently when appropriate

This requires a stable CLI/API contract that is friendly to both humans and agents.

## Conceptual Model

### Notebook Entry

A notebook entry is the broadest record type. It may be a thought, note, bug, task, or goal.

Required fields:

- id
- title
- created_at
- updated_at
- status

Optional fields:

- body markdown
- tags
- due_at
- started_at
- completed_at
- priority score
- source
- links
- parent id
- dependency ids

### Task

A task is a notebook entry that represents work to be done.

Statuses:

- inbox
- planned
- active
- blocked
- done
- archived

Task fields:

- title
- status
- created_at
- updated_at
- due_at, optional
- started_at, optional
- completed_at, optional
- priority score
- subtasks
- dependencies
- markdown note path
- execution log path

### Subtask

Subtasks are nested work items under a parent task.

Subtasks should support:

- own status
- own markdown note
- own dependencies
- completion tracking
- future agent assignment metadata

### Markdown Notes

Every task or subtask may have a markdown file.

Markdown should be used for:

- detailed scope
- acceptance criteria
- observations
- links
- code references
- meeting/context notes
- agent instructions
- execution logs

Markdown files should remain plain files that can be edited outside the app.

## Storage Direction

Use a filesystem-first storage model:

```text
data-root/
  index.tsv
  search/
    ...
  entries/
    <entry-id>/
      entry.tsv
      note.md
      items/
        <item-id>.tsv
        <item-id>.md
      logs/
        <timestamp>.md
```

Current implementation uses `lists/`; the long-term model may rename that concept to `entries/` or `tasks/` once the data model is clearer.

Performance strategy:

- keep compact index files for list/search screens
- avoid scanning every markdown file during normal startup
- write records atomically with temp file plus rename
- keep markdown as separate content so metadata remains fast to parse
- add specialized search indexes when needed

SQLite is a future option if the app needs relational queries, very large datasets, migrations, full-text search, or concurrent writers. The storage boundary should make that possible later without rewriting the UI.

## Search Requirements

Search should support:

- fast fuzzy matching
- exact text matching
- date range filters
- status filters
- due date filters
- completion filters
- tag filters
- dependency filters
- search history
- reusable saved searches

Indexed fields:

- id
- title
- status
- created_at
- updated_at
- due_at
- completed_at
- tags
- parent id
- dependency ids
- item counts
- done counts
- markdown summary or extracted terms

The app should eventually support a dedicated search index optimized for fuzzy retrieval and repeated queries.

## Ranking And Priority

The app should compute a daily ranking of work.

Inputs:

- due date proximity
- overdue status
- task age
- whether work was started but unfinished
- dependency readiness
- blocked state
- explicit user priority
- recency of updates
- historical deferrals

Suggested default priority order:

1. overdue due-date tasks
2. due-date tasks close to deadline
3. started but unfinished tasks from previous days
4. unblocked dependencies that enable other work
5. active tasks without due dates
6. inbox tasks by creation order
7. archived or completed tasks excluded by default

A scheduled job can recompute priority daily. The app should support a command that cron can call.

## Dependency Model

Tasks may depend on other tasks.

Requirements:

- a task can list dependency ids
- blocked tasks should be visible as blocked
- tasks become actionable when dependencies are done
- dependency chains should be detectable
- cycles should be prevented or reported
- ranking should boost unblocked tasks that enable other tasks

## CLI Requirements

The CLI should expose stable commands for both users and agents.

Possible commands:

```sh
tui-do capture "Fix login bug"
tui-do list --status open --since today
tui-do search "auth timeout"
tui-do show <id>
tui-do note <id>
tui-do update <id> --status active
tui-do done <id>
tui-do add-subtask <id> "Write regression test"
tui-do depend <task-id> <dependency-id>
tui-do rank --today
tui-do export --json
```

Agent-friendly output:

- `--json` for structured machine-readable responses
- stable ids
- deterministic error messages
- no terminal UI escape codes in CLI mode
- commands should be safe for scripting

## Terminal UI Requirements

Primary screens:

- main menu
- capture inbox
- daily view
- search
- task detail
- markdown note view/edit launcher
- dependency view
- completed history
- settings/debug

Navigation:

- arrow keys
- number shortcuts
- Enter select
- Escape/back
- Ctrl+C quit
- fuzzy finder shortcuts

The terminal UI should be responsive to resize and remain full-screen.

## Agent Integration Requirements

The app should eventually allow an LLM agent to:

- read the current prioritized queue
- inspect task context
- append markdown notes
- create subtasks
- mark progress
- record commands run and outcomes
- update status
- link dependencies
- propose daily priorities

Agent operations must be auditable. Important agent actions should leave logs.

## Non-Goals For The Current Phase

- remote sync
- multi-user collaboration
- full SQLite migration
- rich markdown rendering
- interactive markdown editor inside the app
- background daemon
- complex recurring-task engine

These can be revisited after the local model, CLI, and core UI are stable.

## Early Milestones

1. **Storage Foundation**
   - filesystem layout
   - index metadata
   - per-item markdown files
   - atomic writes

2. **Main Menu And Input**
   - fullscreen Clay UI
   - resize-safe rendering
   - keyboard navigation

3. **Capture Flow**
   - create inbox entries quickly
   - auto-date everything
   - persist to storage

4. **List And Search**
   - load index
   - filter by status/date
   - fuzzy search titles

5. **Task Detail**
   - show task metadata
   - show subtasks
   - open or create markdown note

6. **CLI Contract**
   - capture
   - list
   - show
   - update
   - JSON output

7. **Ranking**
   - due date weighting
   - stale active work boost
   - dependency readiness

## Open Questions

- Should the top-level concept be called task, entry, note, goal, or something else?
- Should every captured note be promotable to a task?
- Should markdown be edited inside `tui-do` or opened in `$EDITOR`?
- How much hierarchy should subtasks support before becoming too complex?
- Should search index markdown content fully or only summaries/terms?
- How should agent execution logs be displayed in the UI?
- What priority formula feels useful without being surprising?
