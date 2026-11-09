"""Internal data schema for the WasteFlow analysis.

Defines the column names of the internal tables and the model
parameters. All other modules use these names, never raw strings.
"""

from dataclasses import dataclass
from typing import Final


# Shared column

TICK: Final[str] = "tick"


# stats_log: one row per tick with summed values

DISTANCE_KM: Final[str] = "distance_km"
FUEL_L: Final[str] = "fuel_l"
COST_EUR: Final[str] = "cost_eur"
DAY: Final[str] = "day"

STATS_LOG_COLUMNS: Final[list[str]] = [TICK, DISTANCE_KM, FUEL_L, COST_EUR]


# truck_log: one row per truck per tick (raw data)

TRUCK_ID: Final[str] = "truck_id"
X: Final[str] = "x"
Y: Final[str] = "y"
LOAD: Final[str] = "load"
TRUCK_STATE: Final[str] = "truck_state"

TRUCK_LOG_COLUMNS: Final[list[str]] = [TICK, TRUCK_ID, X, Y, LOAD, TRUCK_STATE]  



# Model parameters (placeholder values, override later)

@dataclass(frozen=True)
class ModelParameters:  
    """Model parameters used to convert raw data into km, fuel and cost.

    Attributes:
        tick_duration: Simulated time per tick in minutes.
        cell_size_km: Length of one grid cell in km.
        fuel_per_km: liters per km
        fuel_price: euro per liter
        wear_cost_per_km: euro per km
    """

    tick_duration: float = 1.0     # minutes per tick, placeholder: 1.0
    cell_size_km: float = 0.1     # km per grid cell, placeholder: 0.1
    fuel_per_km: float = 0.5       # litres per km, placeholder: 0.5
    fuel_price: float = 1.70        # euro per liter, placeholder: 1.70
    wear_cost_per_km: float = 0.30  # euro per km, placeholder: 0.30