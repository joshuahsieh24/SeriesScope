import pandas as pd
from nba_api.stats.endpoints import leaguedashteamstats

TEAM_ABBR = {
    "1610612738": "BOS",
    "1610612760": "OKC",
    # ...
}

def fetch_team_stats(season="2023-24"):
    # Fix for param rename in latest nba_api
    stats = leaguedashteamstats.LeagueDashTeamStats(season=season, measure_type_detailed_defense="Advanced").get_data_frames()[0]
    return stats

