#pragma once
#include <QObject>
#include <QtConcurrent>
#include "../models/TeamProfile.h"
#include "../models/ScenarioConfig.h"
#include "../models/SimulationResult.h"

namespace seriesscope {
class ApplicationCoordinator : public QObject {
    Q_OBJECT
public:
    void runSimulation(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config);
signals:
    void resultsReady(AggregatedResults results);
};
}
