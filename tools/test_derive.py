import pytest
from derive_team_profiles import derive_profile

def test_derive_basic():
    stats = {"OFF_RATING": 115, "DEF_RATING": 110}
    profile = derive_profile(stats)
    assert profile["off_rating"] == 115

