# Terminal rendering engine plan

## Goals
- Render with a retained, cell-based buffer instead of full-frame string redraws.
- Diff previous and next frames to minimize terminal I/O.
- Keep a clear separation between layout and painting to enable richer UI primitives.

## Current foundation (implemented in this repo)
1. **Cell grid**
   - `src/render/grid.ts` defines a `Grid` of `{ ch, style }` cells.
   - Rendering writes directly into the grid rather than building strings.

2. **Diff-based flush**
   - `src/render/diff.ts` compares previous vs. next grids and emits minimal cursor
     moves + styled text runs.
   - This replaces full-screen clearing on every frame.

3. **Screen integration**
   - `src/render/screen.ts` now maintains a `prevGrid` and `nextGrid`, clearing only
     when the terminal size changes.

## Near-term roadmap
1. **Viewport + scroll region support**
   - Add a clipping viewport surface to render subsets of content.
   - Provide a scrollbar primitive that paints into the grid.

2. **Hit targets**
   - Introduce a parallel hit grid to map terminal cells to interactive regions.
   - Enable mouse events and richer interactions (drag, hover).

3. **Layout abstraction**
   - Add a layout tree that computes absolute rectangles for components.
   - Keep layout separate from paint so components can render into the grid easily.

4. **Terminal backends**
   - Keep ANSI as the default backend.
   - Add optional capability detection hooks (truecolor, kitty graphics, tmux).
