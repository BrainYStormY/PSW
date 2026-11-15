"""Load simulator data and convert it to the internal schema.

Each loader reads raw data with the team's names and returns a
DataFrame that only uses internal schema names.
"""

from pathlib import Path

import pandas as pd

from wasteflow_py import schema
from wasteflow_py.io.mapping import TRUCK_LOG_MAPPING


def load_csv(
    path: Path,
    mapping: dict[str, str],
    columns: list[str],
) -> pd.DataFrame:
    """Read a CSV file and convert it to the internal schema.

    Args:
        path: Path to the CSV file with the team's column names.
        mapping: Maps team column names to internal schema names.
        columns: Internal column names the result must contain, in order.

    Returns:
        DataFrame with exactly the given columns, in the given order.

    Raises:
        ValueError: If a required column is missing after renaming.
    """
    df = pd.read_csv(path)  # 1. read the file
    df = df.rename(columns=mapping)  # 2. rename team names to internal names
    missing = set(columns) - set(df.columns)  # 3. check required columns
    if missing:
        raise ValueError(f"Missing required columns: {missing}")
    return df[columns]  # 4. only the required columns, in this order


def load_truck_log(path: Path) -> pd.DataFrame:
    """Load a truck log CSV and convert it to the internal schema.

    Args:
        path: Path to the truck log CSV file.

    Returns:
        DataFrame with the columns of schema.TRUCK_LOG_COLUMNS.
    """
    return load_csv(path, TRUCK_LOG_MAPPING, schema.TRUCK_LOG_COLUMNS)