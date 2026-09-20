# Booth HTML loop

Self-contained kiosk page for the chipIgnite / Caravel demo. No build step, no network.

## Run locally

From a clone of this repo (`arty-100t`):

```bash
open display/index.html
```

On Windows: `start display\index.html`. Keep `display/img/` next to `index.html`.

For a kiosk-style session (fullscreen, no `file://` quirks):

```bash
cd display
python3 -m http.server 8765
```

Open http://127.0.0.1:8765 and press **F**.

Scene timing defaults to 8 seconds. Override with `?seconds=12`.

## Keys

| Key | Action |
| --- | --- |
| `F` | Fullscreen |
| `Space` / `P` | Pause / play |
| Arrow keys | Previous / next scene |

Disable OS sleep on the demo machine.

## Hosted copy

https://d2klqjuo9yefmc.cloudfront.net
