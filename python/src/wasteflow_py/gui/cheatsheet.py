"""NiceGUI cheat sheet for the WasteFlow GUI.

Reference only, not imported anywhere. Keep it open in a second tab.
Full docs: https://nicegui.io/documentation

Basic idea:
    Every ui.xxx(...) call creates an element on the page.
    "with" puts elements INSIDE another element (like nested boxes).
    Each element returns an object you can keep and change later.
"""

# =============================================================================
# 1. LAYOUT (boxes that hold other elements)
# =============================================================================
#
# with ui.row():                     # children side by side (left -> right)
#     ui.label("A")
#     ui.label("B")
#
# with ui.column():                  # children stacked (top -> bottom)
#     ...
#
# with ui.card():                    # white box with shadow (panels in mockup)
#     ui.label("Fleet & Routes")
#
# with ui.grid(columns=3):           # fixed number of columns (KPI tiles)
#     ...
#
# with ui.expansion("Advanced", icon="settings"):   # collapsible section
#     ...
#
# ui.separator()                     # thin horizontal line
# ui.space()                         # flexible gap, pushes following items right
#
# Page frame:
# with ui.header():                  # top bar, always visible
#     ui.label("WasteFlow")
# with ui.left_drawer():             # side panel on the left
#     ...
# with ui.footer():                  # bottom bar
#     ...

# =============================================================================
# 2. TABS (Map Editor / Live Simulation / Analytics / Reports)
# =============================================================================
#
# with ui.tabs() as tabs:                    # the clickable tab headers
#     editor_tab = ui.tab("Map Editor", icon="map")
#     live_tab = ui.tab("Live Simulation", icon="play_arrow")
#
# with ui.tab_panels(tabs, value=editor_tab):   # the content of each tab
#     with ui.tab_panel(editor_tab):
#         ui.label("editor content")
#     with ui.tab_panel(live_tab):
#         ui.label("live content")

# =============================================================================
# 3. TEXT
# =============================================================================
#
# ui.label("Hello")                  # plain text
# lbl = ui.label("0 km")
# lbl.set_text("12 km")              # change text later
# ui.markdown("**bold** and *italic*")
# ui.icon("local_shipping")          # icon names: https://fonts.google.com/icons

# =============================================================================
# 4. INPUTS (configuration panels)
# =============================================================================
#
# ui.button("Start", icon="play_arrow", on_click=start_simulation)
# ui.button("Save", on_click=lambda: ui.notify("saved"))   # inline function
#
# ui.number("Trucks", value=3, min=1, max=20, step=1)      # number field
# ui.input("Name", placeholder="Scenario A")               # text field
# ui.slider(min=50, max=300, value=150, step=10)           # slider
# ui.switch("Fixed random seed", value=True)               # on/off switch
# ui.checkbox("Show routes", value=True)
# ui.select(["General", "Recycling"], value="General", label="Bin type")
# ui.toggle(["0.5x", "1x", "2x", "5x"], value="1x")        # segmented buttons
#
# Read / react to a value:
# trucks = ui.number("Trucks", value=3)
# print(trucks.value)                                      # current value
# ui.number("Trucks", value=3, on_change=lambda e: print(e.value))
#
# Slider with live label:
# s = ui.slider(min=0, max=100, value=50)
# ui.label().bind_text_from(s, "value", backward=lambda v: f"{v} m")

# =============================================================================
# 5. GRID MAP (SVG drawn on ui.interactive_image)
# =============================================================================
#
# SVG = text that describes shapes. Coordinates: (0, 0) is top left,
# x grows to the right, y grows downwards.
#
# Shapes:
#   <rect x="0" y="0" width="40" height="40" fill="#eee" stroke="#ccc"/>
#   <circle cx="20" cy="20" r="8" fill="red"/>
#   <line x1="0" y1="0" x2="100" y2="0" stroke="blue" stroke-width="3"/>
#   <polyline points="20,20 60,20 60,100" fill="none" stroke="blue"/>
#   <text x="10" y="10" font-size="12">B-11</text>
#
# Show it:
# grid = ui.interactive_image(size=(800, 600), content=svg_text, events=["click"])
#
# Redraw (e.g. every timer tick):
# grid.content = new_svg_text        # replaces the whole drawing
#
# Click on the map (placing a bin, US05):
# def on_click(e: events.MouseEventArguments) -> None:
#     col = int(e.image_x // CELL_PX)
#     row = int(e.image_y // CELL_PX)
# grid.on_mouse(on_click)
#
# Grid cell -> pixel centre:
#   px = col * CELL_PX + CELL_PX / 2

# =============================================================================
# 6. LIVE UPDATES (simulation clock, moving trucks)
# =============================================================================
#
# timer = ui.timer(0.5, update)      # calls update() every 0.5 s
# timer.deactivate()                 # pause
# timer.activate()                   # resume
# timer.interval = 0.1               # faster (speed up, US25)
#
# Start paused:
# timer = ui.timer(0.5, update, active=False)
#
# Show / hide / enable:
# btn.disable() / btn.enable()
# card.set_visibility(False)

# =============================================================================
# 7. CHARTS AND TABLES (statistics)
# =============================================================================
#
# chart = ui.echart({
#     "xAxis": {"type": "category", "data": ["Day 1", "Day 2", "Day 3"]},
#     "yAxis": {"type": "value", "name": "EUR"},
#     "series": [{"type": "line", "data": [12.5, 30.1, 41.0]}],
# })
# chart.options["series"][0]["data"].append(55.2)   # add a point
# chart.update()                                     # redraw
#
# From pandas:
#   "data": df["cost_eur"].tolist()
#
# Table:
# columns = [{"name": "id", "label": "Truck", "field": "id"},
#            {"name": "km", "label": "km", "field": "km"}]
# rows = [{"id": "T-01", "km": 9.1}, {"id": "T-02", "km": 4.9}]
# ui.table(columns=columns, rows=rows, row_key="id")
# From pandas: rows = df.to_dict("records")

# =============================================================================
# 8. FEEDBACK
# =============================================================================
#
# ui.notify("Map saved", type="positive")    # small popup (positive/negative/warning)
#
# with ui.dialog() as dialog, ui.card():     # modal window
#     ui.label("Reset simulation?")
#     ui.button("Yes", on_click=dialog.close)
# dialog.open()

# =============================================================================
# 9. STYLING
# =============================================================================
#
# .classes("...")   Tailwind CSS classes  (https://tailwindcss.com/docs)
#   w-full          full width          h-screen    full screen height
#   w-72            fixed width         gap-2       space between children
#   p-4             padding             items-center  vertical centring
#   text-lg font-bold text-gray-500     text size / weight / colour
#   bg-green-100    background colour   rounded-lg  rounded corners
#
# .props("...")     Quasar options      (https://quasar.dev/vue-components)
#   ui.button("X").props("flat")        button without background
#   ui.button("X").props("outline")     outlined button
#   ui.input("X").props("dense")        compact field
#
# .style("...")     raw CSS             ui.label("X").style("color: #2e7d32")
#
# ui.colors(primary="#2e7d32")          app-wide main colour

# =============================================================================
# 10. RUN
# =============================================================================
#
# if __name__ in {"__main__", "__mp_main__"}:
#     ui.run(title="WasteFlow", port=8080)
#
# Start from python/:   python -m wasteflow_py.gui.app
# Stop:                 Ctrl+C
# Saving a file reloads the page automatically.