import time
import sqlite3
from datetime import date
import pandas as pd
from nba_api.stats.endpoints import leaguedashteamstats, leaguedashplayerstats, leaguedashplayerclutch


def current_nba_season() -> str:
    """Derive the active NBA season string from today's date.

    The NBA regular season starts in October, so:
      Oct–Dec  →  season starting this calendar year  (e.g. 2025-26)
      Jan–Sep  →  season starting the prior year      (e.g. 2024-25)
    """
    today = date.today()
    start_year = today.year if today.month >= 10 else today.year - 1
    return f"{start_year}-{str(start_year + 1)[-2:]}"


TEAM_ABBR = {
    "1610612738": "BOS",
    "1610612760": "OKC",
    # ...
}

def fetch_team_stats(season=None):
    season = season or current_nba_season()
    # Fix for param rename in latest nba_api
    stats = leaguedashteamstats.LeagueDashTeamStats(season=season, measure_type_detailed_defense="Advanced").get_data_frames()[0]
    return stats

def fetch_player_base_stats(season=None):
    season = season or current_nba_season()
    time.sleep(0.6)
    df = leaguedashplayerstats.LeagueDashPlayerStats(
        season=season,
        measure_type_detailed_defense="Base"
    ).get_data_frames()[0]
    return df[["PLAYER_ID", "PLAYER_NAME", "TEAM_ID", "MIN", "PTS", "STL", "BLK", "GP"]].copy()

def fetch_player_advanced_stats(season=None):
    season = season or current_nba_season()
    time.sleep(0.6)
    df = leaguedashplayerstats.LeagueDashPlayerStats(
        season=season,
        measure_type_detailed_defense="Advanced"
    ).get_data_frames()[0]
    return df[["PLAYER_ID", "TEAM_ID", "USG_PCT", "OFF_RATING", "DEF_RATING",
               "TS_PCT", "AST_PCT", "DREB_PCT", "STL_PCT", "BLK_PCT"]].copy()

def fetch_player_clutch_stats(season=None):
    season = season or current_nba_season()
    time.sleep(0.6)
    df = leaguedashplayerclutch.LeagueDashPlayerClutch(
        season=season,
        measure_type_detailed_defense="Base"
    ).get_data_frames()[0]
    result = df[["PLAYER_ID", "TEAM_ID", "MIN", "PTS", "PLUS_MINUS"]].copy()
    result = result.rename(columns={"MIN": "CLUTCH_MIN", "PTS": "CLUTCH_PTS", "PLUS_MINUS": "CLUTCH_PLUS_MINUS"})
    return result

def build_player_data(season=None):
    base     = fetch_player_base_stats(season)
    advanced = fetch_player_advanced_stats(season)
    clutch   = fetch_player_clutch_stats(season)

    # Merge on PLAYER_ID; keep only players who appear in all three datasets
    df = base.merge(advanced[["PLAYER_ID", "USG_PCT", "OFF_RATING", "DEF_RATING",
                               "TS_PCT", "AST_PCT", "DREB_PCT", "STL_PCT", "BLK_PCT"]],
                    on="PLAYER_ID", how="inner")
    df = df.merge(clutch[["PLAYER_ID", "CLUTCH_MIN", "CLUTCH_PTS", "CLUTCH_PLUS_MINUS"]],
                  on="PLAYER_ID", how="left")
    df[["CLUTCH_MIN", "CLUTCH_PTS", "CLUTCH_PLUS_MINUS"]] = \
        df[["CLUTCH_MIN", "CLUTCH_PTS", "CLUTCH_PLUS_MINUS"]].fillna(0.0)

    # Exclude players with very few games (injury-abbreviated seasons)
    df = df[df["GP"] >= 20]

    # Keep top 5 per team by total minutes
    df = df.sort_values("MIN", ascending=False)
    df = df.groupby("TEAM_ID").head(5).reset_index(drop=True)

    return df

def write_players_to_db(df, db_path):
    con = sqlite3.connect(db_path)
    con.execute("PRAGMA journal_mode=WAL;")
    rows = [
        (
            str(row["PLAYER_ID"]),
            str(row["PLAYER_NAME"]),
            str(row["TEAM_ID"]),
            float(row["MIN"]),
            float(row["PTS"]),
            float(row["STL"]),
            float(row["BLK"]),
            int(row["GP"]),
            float(row["USG_PCT"]),
            float(row["OFF_RATING"]),
            float(row["DEF_RATING"]),
            float(row["TS_PCT"]),
            float(row["AST_PCT"]),
            float(row["DREB_PCT"]),
            float(row["STL_PCT"]),
            float(row["BLK_PCT"]),
            float(row["CLUTCH_MIN"]),
            float(row["CLUTCH_PTS"]),
            float(row["CLUTCH_PLUS_MINUS"]),
        )
        for _, row in df.iterrows()
    ]
    con.executemany(
        "INSERT OR REPLACE INTO players "
        "(player_id, player_name, team_id, minutes, pts, stl, blk, gp, "
        "usg_pct, off_rating, def_rating, ts_pct, ast_pct, dreb_pct, stl_pct, blk_pct, "
        "clutch_min, clutch_pts, clutch_plus_minus) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
        rows
    )
    con.commit()
    con.close()
    print(f"Wrote {len(rows)} player rows to {db_path}")

