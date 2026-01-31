# tui-do

A tiny terminal to-do list app—because sticky notes don’t run `npm install`.

## Links
- npm: https://www.npmjs.com/package/@gouveags/tui-do
- GitHub: https://github.com/gouveags/tui-do

## Install
Requires Node 18+.

Global (recommended):
```bash
npm install -g @gouveags/tui-do
tui-do
```

Local (development):
```bash
npm install
npm run start
```

## Usage
- Launch with `tui-do`.
- Navigate with the arrow keys and `Enter`.
- `Esc` goes back, `Ctrl+C` quits.
- In rename/input prompts, type to edit. Use `Backspace`, `Delete`, `Left`, `Right`, `Home`, `End`, `Enter`, and `Esc`.

## Data storage
- Linux/macOS: `~/.local/share/tui-do` (or `$XDG_DATA_HOME/tui-do`)
- Windows: `%APPDATA%\tui-do`
- Override with `TUI_DO_DIR=/path/to/dir`

## Scripts
- `npm run build` – build
- `npm run test` – tests
- `npm run lint` – lint

## License
MIT. Go forth and procrastinate responsibly.
