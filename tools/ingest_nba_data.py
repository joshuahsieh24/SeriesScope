import pandas as pd
from nba_api.stats.endpoints import leaguedashteamstats

def fetch_team_stats(season="2023-24"):
    stats = leaguedashteamstats.LeagueDashTeamStats(season=season).get_data_frames()[0]
    return stats

