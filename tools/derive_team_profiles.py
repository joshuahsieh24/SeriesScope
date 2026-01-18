def derive_profile(raw_stats):
    # Logic for normalization
    return {
        "off_rating": raw_stats["OFF_RATING"],
        "def_rating": raw_stats["DEF_RATING"]
    }

