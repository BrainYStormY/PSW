# WasteFlow — Project Context for AI Tools

> Shared context file for AI assistants (Claude, ChatGPT, Copilot, …) used by the team.
> Paste it into a chat or keep it in the repo root (it can be copied as `AGENTS.md` / `CLAUDE.md`).
> **Source of truth:** GitLab wiki (gitlab.up.pt) for backlog, sprint planning and decisions. If this file and the wiki disagree, the wiki wins — then update this file.
> Last updated: 2026-11-09

---

## 1. Project in one paragraph

WasteFlow is a **multi-agent city waste simulator** built by a team of 4 students for the course *Software Design* (FEUP, U.Porto), team project "Main Quest". The simulation core is written in **C++**; the **GUI and all statistics** are written in **Python**. Trucks and residents are agents on a city grid; the simulator shows how routes, costs and (later) citizen behaviour emerge from simple rules.

**Vision:** For city waste planners who decide the best waste management strategy without data, WasteFlow is a city waste simulator that shows how citizens react to a policy. Unlike sensor dashboards, our product predicts a policy before it is paid for.

## 2. Mandatory course constraints

Every design decision must keep these satisfiable:

1. GUI to configure the simulation and view its current status (separate window, one or more human users)
2. At least two distinct programs communicating over the Internet (may run on separate computers)
3. Data persistence (no data loss between program runs)
4. At least two distinct OO programming languages (here: C++ and Python)
5. Multi-agent simulator: world emulator + ≥ 2 agent types interacting under simulator rules, showing emergent behaviour

Optional: a GUI through which a human acts as an agent.

## 3. Architecture

| Part | Language | Responsibility | Status |
|------|----------|----------------|--------|
| Simulation core | C++ | World (grid, bins, depot, landfill, houses), agents (trucks, residents), simulation loop | Headers set up (US00); Simulation/Truck not yet wired to the route planner |
| Route planner | C++ | Visiting order and grid path for a truck | Done for 1 truck (US15) |
| Statistics | Python | Computes all KPIs from raw simulation data (distance, fuel, cost, per truck / tick / day) | Done for US52, US53, US55 on sample data |
| GUI | Python (NiceGUI, browser-based) | Configure map and simulation, live view, KPIs, inspector | Skeleton with demo data; buttons not wired yet |
| C++ ↔ Python link | open | Raw data from C++ to Python, commands from GUI to C++ (US44) | **Format not decided** |
| Database | open | Store results of each simulation (US62–US65) | **Not chosen yet** (US62, Sprint 2) |

**Principle:** C++ delivers *raw* data only (positions, loads, states per tick). Python computes every statistic. Each formula exists in exactly one place.

## 4. C++ side

### Shared headers
- `Types.hpp` — `Position` (grid x/y). `operator==` takes `const Position&` and is `const`.
- `Environment.hpp` — world objects incl. `Bin` (has `getId()`, `getPosition()`).
- `SimulationEngine.hpp` — `Simulation` (with `step()`) and `Statistics`.

### Route planner (`RoutePlanner.hpp/.cpp`, by Bruno)
- `RoutePlanner(depot)` — constructed with the depot position.
- `planOrder(bins)` → order in which the truck visits the bins. Solves a TSP-style tour (depot → all bins → depot):
  1. **nearest-neighbour** pass builds an initial route,
  2. **2-opt** reverses route segments while the total distance gets shorter.
- Distance = **Manhattan distance** `|dx| + |dy|` — trucks move only horizontally/vertically on a grid **without obstacles**.
- `buildPath(order)` → every grid cell the truck passes (x first, then y between stops), ending back at the depot.
- `totalDistance(order)` → route length in **cells**.

### Standalone test (`main.cpp`)
- 1 truck on a 30×30 grid. Reads depot and bin positions from the terminal (`5 7`, `5,7` or `(5,7)`), prints the collection order, total distance, a step-by-step description and an arrow map. Does **not** use `Simulation`/`Truck` yet.
- Build: `g++ -std=c++17 -Wall main.cpp RoutePlanner.cpp -o wasteflow`

### Known issues
- `SimulationEngine.hpp`: `Statistics::update` is **private** but called in `Simulation::step()` → does not compile until it is made `public` (or the call is removed, see open question on Statistics below).
- `Truck` has **no position** in the class diagram → needed for movement, distance and the GUI map.

## 5. Python side

Location: `a01_2/python/` (src layout, venv in `python/.venv`) — **location to be confirmed by the team before merge.** Current work on branch `s2/mk`.

| Module | Purpose |
|--------|---------|
| `schema.py` (package root) | Internal column names + `ModelParameters` dataclass |
| `io/mapping.py` | Maps team/simulator column names → internal names (`STATS_LOG_MAPPING`, `TRUCK_LOG_MAPPING`) |
| `io/loader.py` | `load_csv`, `load_truck_log` (CSV for now; real format depends on US44) |
| `metrics.py` | Distance, fuel, cost — total, per truck, per tick, cumulative, per day; time in each state per truck |
| `gui/app.py` | NiceGUI app: 4 tabs, map editor panels, SVG grid, live sim controls, KPI tiles, inspector |
| `gui/cheatsheet.py` | NiceGUI reference snippets |

Libraries in use: **pandas** (time-indexed tables), **NiceGUI** (GUI). Further libraries are added only with a short reason.

## 6. Data model and units

Internal schema (Python):
- `truck_log`: `tick, truck_id, x, y, load, truck_state`
- `stats_log`: `tick, distance_km, fuel_l, cost_eur`
- parameters: `tick_duration` (min), `cell_size_km`, `fuel_per_km`, `fuel_price` (+ wear cost per km)

Model:
- distance = grid steps × `cell_size_km` (matches the route planner's Manhattan cells)
- fuel = distance × `fuel_per_km` (constant consumption)
- cost = fuel cost + wear cost per km
- day = `tick × tick_duration // 1440`, day 0 starts at tick 0
- Parameter values are placeholders and **never hard-coded** (they become user settings with US58).

## 7. Sprint status

Convention: course "iteration #N" = Sprint N.

- **Sprint 0** — done (vision, vision board, backlog, Sprint 1 goal + planning).
- **Sprint 1** — done in team backlog: US00 (libraries + headers), US15 (automatic routes), US16 (C4 level 2), US17 (class diagram), US18 (UI mockups), US52 (km), US53 (fuel), US55 (cost in €).
- **Sprint 2** — planned: US19 (several trucks, each with own route), US62 (choose database), US88 (acceptance tests for all stories), US89 (demo video, 10–60 s, as comment on Wiki Home).

Every sprint produces in the wiki: Sprint Goal, Sprint Planning (US ids, priorities, estimates, assignees), Done User Stories (planned vs. done), Retrospective (Did well / Do differently / Puzzles), Review (increment, challenges, self-assessment).

Backlog format: *As a <role>, I want <goal>, so that <reason>.* IDs US00–US89 (renumbered 2026-11-07).

## 8. Open questions (team)

1. **C++ ↔ Python interface (US44):** CSV, JSON or network? How do routes and truck positions reach the GUI?
2. **Statistics placement:** UML attaches `Statistics` to `Simulation` (C++), but all KPIs are computed in Python. Keep a C++ `Statistics` only for raw counters, or remove it?
3. **Truck position:** add `Position` to `Truck`.
4. **Multiple trucks (US19/US20):** how are bins divided between trucks before `planOrder` runs per truck?
5. **Obstacles (US22/US25):** Manhattan distance is only valid without obstacles; with roads/obstacles the planner needs a path search (e.g. BFS/A*) for distances.
6. **Units:** tick duration, cell size in km, fuel consumption, fuel price, wear cost (US58).
7. **Database choice (US62).**
8. **Repo layout:** final location of `python/` and of the C++ sources.

## 9. Rules for AI assistants

- **Language:** all code, comments, docstrings, commit messages, wiki text and user stories in **English**.
- **Python:** PEP 8, type hints everywhere, Google-style docstrings (`Args:`, `Returns:`, `Raises:`).
- **C++:** C++17, compile with `-Wall` without warnings.
- **Shared headers:** changes to shared `.hpp` files are marked with the author's tag in a comment (e.g. `// [Bruno]`) and listed in the author's status notes.
- Propose changes before making them; do not rename shared classes, columns or files without asking.
- Don't hard-code model parameters; keep each formula in one place.
- Don't invent the C++ ↔ Python data format or the database — they are open decisions (see §8).
- Point out contradictions with this file or the wiki instead of silently choosing one.
- When suggesting commits, follow §10.
- **Git reminders:** remind the user regularly to commit and push (see §10 "When to commit") and to keep branches clean — without waiting to be asked.

## 10. Commit conventions

**Scope — one user story per commit.**
- Each commit covers exactly **one** user story wherever possible.
- Several stories may share one commit **only if they are closely linked** and cannot be committed separately in a sensible way (e.g. US45 bin fill level + US46 full bins turn red; US35–US38 start/pause/resume/stop).
- Do not mix unrelated stories in one commit. If a change touches two unrelated stories, split it into two commits.

**Message format (first line):**

```
Sprint <N>, US<xx>: <what was done>
```

- `<N>` = sprint in which the work is done (course "iteration #N" = Sprint N).
- `US<xx>` = two-digit story ID from the backlog (`US05`, `US19`, …).
- Several linked stories: list them separated by commas → `Sprint <N>, US<xx>, US<yy>: …`
- `<what was done>`: English, imperative mood ("Add", "Fix", "Wire", not "Added"), short and concrete, no full stop at the end. Keep the whole first line ≤ 72 characters.
- Optional body after a blank line: why the change was made, open points, changes to shared headers (with author tag, see §9).

**Examples:**

```
Sprint 2, US19: Assign bins to several trucks before route planning
Sprint 2, US45, US46: Show bin fill level and colour full bins red
Sprint 2, US35, US36, US37, US38: Wire start/pause/resume/stop buttons
Sprint 1, US55: Add total cost in euros to metrics
```

**When to commit (AI assistants remind the user):**
- after each finished, working step — a user story or a self-contained part of one,
- before a larger refactoring or a risky change,
- at the end of every work session: commit **and push**, so no work exists only locally.

**Branches:**
- Never commit directly to `master`. Changes reach `master` only via a merge request in GitLab.
- **Every team member has their own branch per sprint:** `s<N>/<initials>` (e.g. `s2/mk`). All stories a member works on in that sprint are committed there — no separate branch per story; the commit messages (`US<xx>`) keep the stories traceable.
- Every team member commits in the format above — no exceptions, so the history of all branches reads the same.

**Git hygiene:**
- Before starting work: `git status` (clean?), then `git pull` on the current branch; update the sprint branch from `master` regularly.
- Before committing: check `git status` and `git diff`; stage only files that belong to the story (no `git add .` without checking).
- Never commit generated or local files: `.venv/`, `__pycache__/`, build output (e.g. `wasteflow` binary), IDE settings, large data files — keep `.gitignore` up to date.
- Do not rewrite history that is already pushed (no `push --force` on shared branches).
- Delete merged branches locally and on GitLab.
