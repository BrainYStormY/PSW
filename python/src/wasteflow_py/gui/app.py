"""WasteFlow GUI: layout skeleton based on the UI mockups.

All buttons and fields exist but do not act yet (only a notification).
Logic (grid editing, timer, simulator connection, statistics) is added
step by step. See cheatsheet.py for NiceGUI syntax.

Start from the python/ folder:
    python -m wasteflow_py.gui.app
"""

from nicegui import ui

# --- Grid settings (placeholders until the team defines the city size) ---
GRID_COLS: int = 20
GRID_ROWS: int = 14
CELL_PX: int = 36

# --- Colours used on the grid ---
COLOR_CELL: str = "#f5f5f4"
COLOR_LINE: str = "#d6d3d1"
COLOR_DEPOT: str = "#1d4ed8"
COLOR_LANDFILL: str = "#78350f"
COLOR_BIN_OK: str = "#16a34a"
COLOR_BIN_FULL: str = "#dc2626"
COLOR_TRUCK: str = "#7c3aed"

# --- Demo entities so the grid is not empty (replaced by real data later) ---
# Each entity: (column, row)
DEMO_DEPOT: tuple[int, int] = (1, 1)
DEMO_LANDFILL: tuple[int, int] = (18, 12)
DEMO_BINS: list[tuple[int, int, float]] = [  # (column, row, fill level 0..1)
    (5, 3, 0.2),
    (9, 6, 0.6),
    (13, 4, 1.0),
    (7, 10, 0.4),
    (15, 9, 0.9),
]
DEMO_TRUCK: tuple[int, int] = (3, 1)

# --- Resident profiles from the mockup: (name, share %, kg/day, awareness) ---
RESIDENT_PROFILES: list[tuple[str, int, float, float]] = [
    ("Eco-Conscious", 30, 0.9, 0.85),
    ("Standard", 50, 1.3, 0.55),
    ("Careless", 20, 1.8, 0.20),
]


def not_implemented(feature: str) -> None:
    """Show a short popup for buttons that have no logic yet.

    Args:
        feature: Name of the feature shown in the popup.
    """
    ui.notify(f"{feature}: not implemented yet", type="warning")


def cell_center(col: int, row: int) -> tuple[float, float]:
    """Convert a grid cell to the pixel position of its centre.

    Args:
        col: Column index, 0 is the left edge.
        row: Row index, 0 is the top edge.

    Returns:
        (x, y) pixel coordinates of the cell centre.
    """
    return col * CELL_PX + CELL_PX / 2, row * CELL_PX + CELL_PX / 2


def render_grid_svg() -> str:
    """Build the SVG drawing of the city grid with all entities.

    Returns:
        SVG content as a string, used by ui.interactive_image.
    """
    parts: list[str] = []

    # Background cells
    for row in range(GRID_ROWS):
        for col in range(GRID_COLS):
            parts.append(
                f'<rect x="{col * CELL_PX}" y="{row * CELL_PX}" '
                f'width="{CELL_PX}" height="{CELL_PX}" '
                f'fill="{COLOR_CELL}" stroke="{COLOR_LINE}"/>'
            )

    # Depot and landfill as squares
    for (col, row), color, label in [
        (DEMO_DEPOT, COLOR_DEPOT, "D"),
        (DEMO_LANDFILL, COLOR_LANDFILL, "L"),
    ]:
        x, y = cell_center(col, row)
        size = CELL_PX * 0.7
        parts.append(
            f'<rect x="{x - size / 2}" y="{y - size / 2}" width="{size}" '
            f'height="{size}" rx="4" fill="{color}"/>'
        )
        parts.append(
            f'<text x="{x}" y="{y + 5}" text-anchor="middle" '
            f'font-size="14" fill="white">{label}</text>'
        )

    # Bins as circles, red when full (US29)
    for col, row, fill in DEMO_BINS:
        x, y = cell_center(col, row)
        color = COLOR_BIN_FULL if fill >= 1.0 else COLOR_BIN_OK
        parts.append(f'<circle cx="{x}" cy="{y}" r="{CELL_PX * 0.3}" fill="{color}"/>')

    # Truck as a triangle
    x, y = cell_center(*DEMO_TRUCK)
    half = CELL_PX * 0.35
    parts.append(
        f'<polygon points="{x},{y - half} {x - half},{y + half} {x + half},{y + half}" '
        f'fill="{COLOR_TRUCK}"/>'
    )

    return "".join(parts)


def build_grid() -> ui.interactive_image:
    """Create the grid view element.

    Returns:
        The interactive image showing the grid (redraw via .content).
    """
    return ui.interactive_image(
        size=(GRID_COLS * CELL_PX, GRID_ROWS * CELL_PX),
        content=render_grid_svg(),
        events=["click"],
    ).classes("w-full border rounded-lg")


def section_title(text: str, icon: str) -> None:
    """Show a bold section title with an icon.

    Args:
        text: Title text.
        icon: Material icon name.
    """
    with ui.row().classes("items-center gap-2"):
        ui.icon(icon).classes("text-green-700")
        ui.label(text).classes("font-bold")


# =============================================================================
# Header
# =============================================================================


def build_header() -> ui.tabs:
    """Create the top bar with logo, tabs and map buttons.

    Returns:
        The tabs element, needed to connect the tab panels.
    """
    with ui.header().classes("bg-white text-gray-800 shadow-sm items-center gap-4"):
        ui.icon("recycling").classes("text-2xl text-green-700")
        ui.label("WasteFlow").classes("text-lg font-bold")
        ui.badge("PLANNER", color="green-2", text_color="green-9")

        with ui.tabs().props("dense no-caps") as tabs:
            ui.tab("editor", label="Map Editor", icon="map")
            ui.tab("live", label="Live Simulation", icon="play_arrow")
            ui.tab("analytics", label="Analytics", icon="bar_chart")
            ui.tab("reports", label="Reports", icon="description")

        ui.space()
        ui.button("Load Map", icon="folder_open",
                  on_click=lambda: not_implemented("Load map (US10)")).props("outline")
        ui.button("Save Map", icon="save",
                  on_click=lambda: not_implemented("Save map (US09)"))
    return tabs


# =============================================================================
# Tab 1: Map Editor
# =============================================================================


def build_scenario_setup() -> None:
    """Create the left panel: residents, bins, fleet."""
    with ui.card().classes("w-80 gap-3"):
        ui.label("Scenario Setup").classes("text-lg font-bold")

        # --- Waste producers (US17-US20, US65) ---
        section_title("Waste Producers", "groups")
        ui.number("Total residents (agents)", value=200, min=0, step=10).props("dense")
        with ui.grid(columns=4).classes("w-full gap-1 items-center text-sm"):
            for header in ("Profile", "Share %", "kg/day", "Awareness"):
                ui.label(header).classes("text-gray-500")
            for name, share, waste, awareness in RESIDENT_PROFILES:
                ui.label(name)
                ui.number(value=share, min=0, max=100).props("dense")
                ui.number(value=waste, min=0, step=0.1).props("dense")
                ui.number(value=awareness, min=0, max=1, step=0.05).props("dense")
        ui.button("Distribute Agents", icon="person_pin",
                  on_click=lambda: not_implemented("Distribute agents")).props("outline")

        ui.separator()

        # --- Bin positioning ---
        section_title("Bin Positioning", "delete")
        walk = ui.slider(min=50, max=300, value=150, step=10)
        ui.label().bind_text_from(walk, "value",
                                  backward=lambda v: f"Walking range: {v} m")
        ui.toggle(["General", "Recycling"], value="General")
        ui.button("Best Bin Positioning", icon="auto_fix_high",
                  on_click=lambda: not_implemented("Best bin positioning")
                  ).classes("w-full")

        ui.separator()

        # --- Fleet and routes (US11-US16) ---
        section_title("Fleet & Routes", "local_shipping")
        with ui.grid(columns=2).classes("w-full gap-2"):
            ui.number("Trucks", value=3, min=1, step=1).props("dense")
            ui.number("Speed (km/h)", value=30, min=1).props("dense")
            ui.number("Capacity (t)", value=1.5, min=0.1, step=0.1).props("dense")
            ui.number("Breakdown (%/day)", value=2.0, min=0, max=100,
                      step=0.5).props("dense")
        ui.button("See Ideal Route", icon="route",
                  on_click=lambda: not_implemented("Ideal route")).classes("w-full")


def build_editor_toolbar() -> None:
    """Create the toolbar above the grid (place / move / remove entities)."""
    with ui.row().classes("w-full items-center gap-2"):
        with ui.button_group().props("outline"):
            for label, icon in [
                ("Select", "near_me"),
                ("Building", "home"),
                ("Depot", "warehouse"),
                ("Landfill", "landscape"),
                ("Recycling Ctr", "recycling"),
                ("Add Bin", "delete"),
                ("Move", "open_with"),
                ("Remove", "backspace"),
            ]:
                ui.button(label, icon=icon,
                          on_click=lambda lbl=label: not_implemented(lbl)
                          ).props("flat no-caps dense")
        ui.space()
        ui.switch("Fixed random seed", value=True)  # US59
        ui.number("Seed", value=20261031).props("dense").classes("w-32")


def build_inspector() -> None:
    """Create the right panel: selected entity, facilities, routes."""
    with ui.card().classes("w-72 gap-3"):
        section_title("Inspector", "search")
        ui.label("Click an entity on the map").classes("text-sm text-gray-500")
        # Filled later with details of the clicked bin or truck (US33, US34)

        ui.separator()
        ui.label("Facilities").classes("font-bold")
        ui.label("Depot · start of all routes").classes("text-sm")
        ui.label("Landfill · receives general waste").classes("text-sm")

        ui.separator()
        ui.label("Ideal Routes").classes("font-bold")
        ui.label("No routes yet").classes("text-sm text-gray-500")


def build_map_editor() -> None:
    """Create the content of the Map Editor tab."""
    with ui.row().classes("w-full no-wrap items-start gap-4"):
        build_scenario_setup()
        with ui.column().classes("flex-1 gap-2"):
            build_editor_toolbar()
            build_grid()
        build_inspector()


# =============================================================================
# Tab 2: Live Simulation
# =============================================================================


def kpi_tile(title: str, value: str, unit: str, icon: str) -> ui.label:
    """Create one KPI tile.

    Args:
        title: Small caption above the value.
        value: Initial value text.
        unit: Unit shown next to the value.
        icon: Material icon name.

    Returns:
        The value label, so it can be updated later with set_text().
    """
    with ui.card().classes("gap-1 p-3"):
        with ui.row().classes("items-center gap-1 text-gray-500 text-xs"):
            ui.icon(icon)
            ui.label(title)
        with ui.row().classes("items-baseline gap-1"):
            value_label = ui.label(value).classes("text-2xl font-bold")
            ui.label(unit).classes("text-sm text-gray-500")
    return value_label


def build_simulation_controls() -> None:
    """Create the control bar: start/pause/resume/stop, speed, clock, save."""
    with ui.row().classes("w-full items-center gap-2"):
        ui.button("Start", icon="play_arrow",
                  on_click=lambda: not_implemented("Start (US21)"))
        ui.button("Pause", icon="pause",
                  on_click=lambda: not_implemented("Pause (US22)")).props("outline")
        ui.button("Resume", icon="play_circle",
                  on_click=lambda: not_implemented("Resume (US23)")).props("outline")
        ui.button("Stop & Reset", icon="stop", color="red",
                  on_click=lambda: not_implemented("Stop & reset (US24)")
                  ).props("outline")

        ui.label("Speed").classes("ml-4 text-gray-500")
        ui.toggle(["0.5x", "1x", "2x", "5x"], value="1x")  # US25, US26

        # Simulation clock (US27)
        with ui.card().classes("bg-gray-900 text-white px-4 py-1 ml-4"):
            ui.label("Day 1 · 00:00").classes("font-mono text-lg")

        ui.space()
        ui.button("Save State", icon="save",
                  on_click=lambda: not_implemented("Save state (US45)")).props("outline")
        ui.button("Load State", icon="folder_open",
                  on_click=lambda: not_implemented("Load state (US46)")).props("outline")
        ui.button("Save Final Results", icon="task",
                  on_click=lambda: not_implemented("Save results (US47)"))


def build_kpi_row() -> None:
    """Create the row of KPI tiles (values from the statistics module later)."""
    with ui.grid(columns=6).classes("w-full gap-2"):
        kpi_tile("Total distance", "0.0", "km", "route")           # US35
        kpi_tile("Fuel used", "0.0", "L", "local_gas_station")     # US36
        kpi_tile("Overflowing bins", "0", "bins", "warning")       # US37
        kpi_tile("Recycling rate", "0.0", "%", "recycling")        # US54
        kpi_tile("Mis-sorted waste", "0.0", "kg", "report")        # US57
        kpi_tile("Total cost", "0.00", "EUR", "euro")              # US38


def build_live_simulation() -> None:
    """Create the content of the Live Simulation tab."""
    with ui.column().classes("w-full gap-3"):
        build_simulation_controls()
        build_kpi_row()
        with ui.row().classes("w-full no-wrap items-start gap-4"):
            with ui.column().classes("flex-1"):
                build_grid()
            with ui.card().classes("w-72 gap-2"):
                section_title("Entity Inspector", "search")
                ui.label("Click any bin or truck on the map").classes(
                    "text-sm text-gray-500")


# =============================================================================
# Tab 3 and 4: placeholders
# =============================================================================


def build_analytics() -> None:
    """Create the Analytics tab (charts from the statistics module later)."""
    ui.label("Analytics: cost over time (US39), overflowing bins over time (US40)"
             ).classes("text-gray-500")


def build_reports() -> None:
    """Create the Reports tab (report for the mayor later)."""
    ui.label("Reports: report for the mayor (US52), PDF export (US53)"
             ).classes("text-gray-500")


# =============================================================================
# Page
# =============================================================================


@ui.page("/")
def index() -> None:
    """Build the main page. Each browser tab gets its own copy (US44)."""
    ui.colors(primary="#2e7d32")
    tabs = build_header()
    with ui.tab_panels(tabs, value="editor").classes("w-full bg-transparent"):
        with ui.tab_panel("editor"):
            build_map_editor()
        with ui.tab_panel("live"):
            build_live_simulation()
        with ui.tab_panel("analytics"):
            build_analytics()
        with ui.tab_panel("reports"):
            build_reports()


if __name__ in {"__main__", "__mp_main__"}:
    ui.run(title="WasteFlow", port=8080)