import pandas as pd
from ._core import _decode_oast

def decode_oast(subdomains):
    """
    Decode one or more OAST subdomains.

    Returns a pandas DataFrame with columns:
    - original
    - timestamp (UTC, timezone-aware)
    - machine_id
    - pid
    - counter

    Invalid inputs produce NaT/NaN/None as appropriate.
    """
    raw = _decode_oast(subdomains)
    originals, timestamps, machine_ids, pids, counters = raw

    df = pd.DataFrame({
        "original": originals,
        "machine_id": machine_ids,
        "pid": pids,
        "counter": counters,
    })
    df["timestamp"] = pd.to_datetime(timestamps, unit="s", utc=True, errors="coerce")
    return df
