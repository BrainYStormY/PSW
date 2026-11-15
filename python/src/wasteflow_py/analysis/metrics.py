"""Statistics computed from internal simulation data."""

import pandas as pd

from wasteflow_py.analysis import schema
from wasteflow_py.analysis.schema import ModelParameters
from typing import Final


MINUTES_PER_DAY: Final[int] = 24 * 60
_STEPS: Final[str] = "steps"  # internal helper column

def distance_km_per_truck(
    truck_log: pd.DataFrame,
    params: ModelParameters,
) -> pd.Series:
    """Compute the distance driven by each truck.

    One grid step is |dx| + |dy| between two consecutive ticks.

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
        params: Model parameters, uses cell_size_km.

    Returns:
        Series indexed by truck_id with the distance in km.
    """
    df = truck_log.sort_values([schema.TRUCK_ID, schema.TICK])  # 1. by truck, then by tick
    grouped = df.groupby(schema.TRUCK_ID)  # 2. one group per truck
    dx = grouped[schema.X].diff().abs()  # 3. |change in x|
    dy = grouped[schema.Y].diff().abs()  # |change in y|
    df = df.assign(steps=dx + dy)  # 4. grid steps per row
    steps = df.groupby(schema.TRUCK_ID)["steps"].sum()  # 5. total steps per truck
    return (steps * params.cell_size_km).rename(schema.DISTANCE_KM)   # 6. steps -> km


def total_distance_km(
    truck_log: pd.DataFrame,
    params: ModelParameters,
) -> float:
    """Compute the total distance driven by all trucks (US35).

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
        params: Model parameters, uses cell_size_km.

    Returns:
        Total distance in km.
    """
    return float(distance_km_per_truck(truck_log, params).sum())



def total_fuel_l(
    truck_log: pd.DataFrame,
    params: ModelParameters,
) -> float:
    """Compute the total fuel used by all trucks (US36).

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
        params: Model parameters, uses cell_size_km and fuel_per_km.

    Returns:
        Total fuel in litres.
    """
    distance = total_distance_km(truck_log, params)
    return distance * params.fuel_per_km


def total_cost_eur(
    truck_log: pd.DataFrame,
    params: ModelParameters,
) -> float:
    """Compute the total cost of all trucks (US38).

    Cost = fuel cost + wear cost.

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
        params: Model parameters, uses cell_size_km, fuel_per_km,
            fuel_price and wear_cost_per_km.

    Returns:
        Total cost in euro.
    """
    distance = total_distance_km(truck_log, params)  # total km, reuse the function from US35
    fuel = total_fuel_l(truck_log, params)  # total litres, reuse the function above
    fuel_cost = fuel * params.fuel_price
    wear_cost = distance * params.wear_cost_per_km
    return fuel_cost + wear_cost


def _steps_per_row(truck_log: pd.DataFrame) -> pd.DataFrame:
    """Add a helper column with the grid steps since the previous tick.

    The first row of each truck gets 0 steps.

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.

    Returns:
        Copy of truck_log, sorted by truck and tick, with column _STEPS.
    """
    df = truck_log.sort_values([schema.TRUCK_ID, schema.TICK])
    grouped = df.groupby(schema.TRUCK_ID)
    dx = grouped[schema.X].diff().abs()
    dy = grouped[schema.Y].diff().abs()
    return df.assign(**{_STEPS: (dx + dy).fillna(0)})


def _cost_eur(
    distance_km: float | pd.Series,
    fuel_l: float | pd.Series,
    params: ModelParameters,
) -> float | pd.Series:
    """Apply the cost model: fuel cost + wear cost.

    Args:
        distance_km: Distance in km (single value or per row).
        fuel_l: Fuel in litres (single value or per row).
        params: Model parameters, uses fuel_price and wear_cost_per_km.

    Returns:
        Cost in euro, same shape as the input.
    """
    return fuel_l * params.fuel_price + distance_km * params.wear_cost_per_km


def fuel_l_per_truck(
    truck_log: pd.DataFrame,
    params: ModelParameters,
) -> pd.Series:
    """Compute the fuel used by each truck.

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
        params: Model parameters, uses cell_size_km and fuel_per_km.

    Returns:
        Series indexed by truck_id with the fuel in litres.
    """
    distance = distance_km_per_truck(truck_log, params)
    return (distance * params.fuel_per_km).rename(schema.FUEL_L)


def cost_eur_per_truck(
    truck_log: pd.DataFrame,
    params: ModelParameters,
) -> pd.Series:
    """Compute the cost of each truck.

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
        params: Model parameters (distance, fuel and cost fields).

    Returns:
        Series indexed by truck_id with the cost in euro.
    """
    distance = distance_km_per_truck(truck_log, params)
    fuel = fuel_l_per_truck(truck_log, params)
    return _cost_eur(distance, fuel, params).rename(schema.COST_EUR)


def stats_per_tick(
    truck_log: pd.DataFrame,
    params: ModelParameters,
) -> pd.DataFrame:
    """Compute distance, fuel and cost of all trucks per tick.

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
        params: Model parameters (distance, fuel and cost fields).

    Returns:
        DataFrame with the columns of schema.STATS_LOG_COLUMNS,
        one row per tick.
    """
    df = _steps_per_row(truck_log)
    steps = df.groupby(schema.TICK)[_STEPS].sum()
    distance = steps * params.cell_size_km
    fuel = distance * params.fuel_per_km
    stats = pd.DataFrame({
        schema.DISTANCE_KM: distance,
        schema.FUEL_L: fuel,
        schema.COST_EUR: _cost_eur(distance, fuel, params),
    })
    return stats.reset_index()[schema.STATS_LOG_COLUMNS]


def cumulative_stats(stats: pd.DataFrame) -> pd.DataFrame:
    """Turn per-tick values into running totals (basis for US39).

    Args:
        stats: DataFrame from stats_per_tick.

    Returns:
        Copy of stats where each value is the total up to that tick.
    """
    result = stats.copy()
    value_cols = [schema.DISTANCE_KM, schema.FUEL_L, schema.COST_EUR]
    result[value_cols] = result[value_cols].cumsum()
    return result


def stats_per_day(
    stats: pd.DataFrame,
    params: ModelParameters,
) -> pd.DataFrame:
    """Sum per-tick values per simulated day.

    Args:
        stats: DataFrame from stats_per_tick.
        params: Model parameters, uses tick_duration.

    Returns:
        DataFrame with column schema.DAY and the summed values per day.
    """
    minutes = stats[schema.TICK] * params.tick_duration
    day = (minutes // MINUTES_PER_DAY).astype(int).rename(schema.DAY)
    per_day = stats.drop(columns=schema.TICK).groupby(day).sum()
    return per_day.reset_index()


def time_in_state_min(
    truck_log: pd.DataFrame,
    params: ModelParameters,
) -> pd.DataFrame:
    """Compute how many minutes each truck spent in each state.

    Each row of the log counts as one tick in that state.

    Args:
        truck_log: DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
        params: Model parameters, uses tick_duration.

    Returns:
        DataFrame with one row per truck and one column per state.
    """
    counts = truck_log.groupby([schema.TRUCK_ID, schema.TRUCK_STATE]).size()
    minutes = counts * params.tick_duration
    return minutes.unstack(fill_value=0)