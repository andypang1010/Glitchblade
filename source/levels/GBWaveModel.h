//
//  GBWaveModel.h
//

#ifndef __GB_WAVE_MODEL_H__
#define __GB_WAVE_MODEL_H__
#include <cugl/cugl.h>
#include "../enemies/GBEnemyModel.h"

using namespace cugl;
using namespace cugl::graphics;

/**
 * This model holds all states of the current wave. This includes the current wave number, the total number of waves, references to
 * the enemies in the wave, and sprites of all environmental objects.
 */
class WaveModel {
private:
    std::vector<std::string> _enemies;
    std::vector<float> _spawnIntervals;

public:
    /**
     * Constructor.
     */
    WaveModel();

    /**
     * Destructor.
     */
    ~WaveModel();

    /**
     * Initializes the wave model with the given parameters.
     */
    void init();

    /** Getters and Setters **/
    // Getter and Setter for _enemies
    std::vector<std::string> getEnemies() const { return _enemies; }
    void setEnemies(const std::vector<std::string>& enemies) { _enemies = enemies; }

    // Getter and Setter for _spawnIntervals
    std::vector<float> getSpawnIntervals() const { return _spawnIntervals; }
    void setSpawnIntervals(const std::vector<float>& spawnIntervals) { _spawnIntervals = spawnIntervals; }
};


#endif /* __GB_WAVE_MODEL_H__ */
