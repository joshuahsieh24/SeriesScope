def derive_profile(raw_stats):
    return {
        "id": str(raw_stats["TEAM_ID"]),
        "off_rating": raw_stats["OFF_RATING"],
        "def_rating": raw_stats["DEF_RATING"]
    }

