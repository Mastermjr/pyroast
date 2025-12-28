import pandas as pd
import pytest
from pyroast import decode_oast

def test_basic_usage():
    df = decode_oast("0123456789abcdefghij")
    assert len(df) == 1
    row = df.iloc[0]
    assert row["original"] == "0123456789abcdefghij"
    assert row["timestamp"] == pd.Timestamp("1970-02-21 17:27:48", tz="UTC")
    assert row["machine_id"] == "c7:42:54"
    assert row["pid"] == 46645
    assert row["counter"] == 13599845

def test_vectorized():
    subdomains = [
        "0123456789abcdefghij",
        "abcdef0123456789ghij",
        "fedcba9876543210beef",
    ]
    df = decode_oast(subdomains)
    assert len(df) == 3
    assert df["timestamp"].iloc[1] == pd.Timestamp("2014-01-17 07:09:48", tz="UTC")
    assert df["machine_id"].iloc[2] == "28:39:8a"

def test_error_handling():
    mixed = [
        "0123456789abcdefghij",   # valid
        "short",                   # too short
        "xyz123456789abcdefgh",    # invalid chars
        "ABCDEF0123456789GHIJ",    # valid (case-insensitive)
    ]
    df = decode_oast(mixed)
    assert not pd.isna(df["timestamp"].iloc[0])
    assert pd.isna(df["timestamp"].iloc[1])
    assert pd.isna(df["timestamp"].iloc[2])
    assert not pd.isna(df["timestamp"].iloc[3])