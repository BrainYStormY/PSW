"""Mapping from team (simulator) names to internal schema names.

This is the only place that knows the team's naming. When the
simulator's field names change, only this file needs updating.
"""

from typing import Final

from wasteflow_py.schema import (
    TICK,
    DISTANCE_KM,
    FUEL_L,
    COST_EUR,
    TRUCK_ID,
    X,
    Y,
    LOAD,
    TRUCK_STATE,
    )  # import all constants you need below

# Team name -> internal name for stats_log.
STATS_LOG_MAPPING: Final[dict[str, str]] = {
    "tick": TICK,
    "totalKm": DISTANCE_KM,
    "fuelUsed": FUEL_L,
    "totalCost": COST_EUR,
}

# Team name -> internal name for truck_log.
TRUCK_LOG_MAPPING: Final[dict[str, str]] = {
    "tick": TICK,  
    "id": TRUCK_ID,
    "posX": X,
    "posY": Y,
    "load": LOAD,  
    "state": TRUCK_STATE,
}