//
//  GBLevelModel.h
//

#ifndef __GB_LEVEL_MODEL_H__
#define __GB_LEVEL_MODEL_H__
#include <cugl/cugl.h>
#include "GBPlayerModel.h"
#include "GBEnemyModel.h"
#include "GBWaveModel.h"

using namespace cugl;
using namespace cugl::graphics;

/**
 * This model holds all states of the current level. This includes the current wave number, the total number of waves, references to 
 * the enemies in the level, and sprites of all environmental objects.
 */
class LevelModel {
private:
	std::string _levelName;
	std::shared_ptr<Texture> _background;
	std::vector<std::shared_ptr<Rect>> _platforms;
    std::vector<std::shared_ptr<WaveModel>> _waves;

public:
    /**
     * Constructor.
     */
    LevelModel();

    /**
     * Destructor.
     */
    ~LevelModel();

    /**
     * Initializes the level model with the given parameters.
     */
    void init();

    /** Getters and Setters **/

    // Getter and Setter for _levelName
    std::string getLevelName() const { return _levelName; }
    void setLevelName(const std::string& levelName) { _levelName = levelName; }

    // Getter and Setter for _background
    std::shared_ptr<Texture> getBackground() const { return _background; }
    void setBackground(const std::shared_ptr<Texture>& background) { _background = background; }

    // Getter and Setter for _platforms
    std::vector<std::shared_ptr<Rect>> getPlatforms() const { return _platforms; }
    void setPlatforms(const std::vector<std::shared_ptr<Rect>>& platforms) { _platforms = platforms; }

    // Getter and Setter for _waves
    std::vector<std::shared_ptr<WaveModel>> getWaves() const { return _waves; }
    void setWaves(const std::vector<std::shared_ptr<WaveModel>>& waves) { _waves = waves; }
};

#endif /* __GB_LEVEL_MODEL_H__ */
