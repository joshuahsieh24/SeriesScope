# SeriesScope

SeriesScope is a C++ desktop application for simulating NBA playoff series using Monte Carlo methods.

## Architecture
- **Engine**: C++17, Monte Carlo simulation, Logit-based win probability.
- **Data**: SQLite3 (WAL mode), RAII management, Python ingestion pipeline.
- **UI**: Qt6, Dark Mode, Async simulation dispatch.

## Model Card
- **Algorithm**: Logistic regression on net rating.
- **Calibration**: Adjusted for home court advantage and 2-2-1-1-1 schedule.

## Interview Framing
Built to demonstrate multithreading in C++, modern UI design with Qt, and data engineering pipelines.
