//
//  GBLevelModel.h
//

#ifndef __GB_LEVEL_MODEL_H__
#define __GB_LEVEL_MODEL_H__
#include <cugl/cugl.h>
#include "../player/GBPlayerModel.h"
#include "../enemies/GBEnemyModel.h"
#include "GBWaveModel.h"

using namespace cugl;
using namespace cugl::graphics;

/**
 * This model holds all states of the current level. This includes the current wave number, the total number of waves, references to 
 * the enemies in the level, and sprites of all environmental objects.
 */
class LevelModel {
private:
    float _scale;
    int _tiles;
	std::string _levelName;
    std::shared_ptr<Texture> _ground;
    std::shared_ptr<Texture> _background;
    std::vector<std::pair<std::shared_ptr<Texture>, unsigned int>> _layers;
	std::vector<Rect> _platforms;
    std::vector<std::shared_ptr<WaveModel>> _waves;
    std::vector<std::pair<float, float>> _walls;

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
    
    float getScale() { return _scale; }
    void setScale(float scale) { _scale = scale; }
    
    std::vector<std::pair<std::shared_ptr<Texture>, unsigned int>> getLayers() const { return _layers; }
    void addLayer(const std::shared_ptr<Texture>& layer, unsigned int speed) { _layers.push_back(std::make_pair(layer, speed)); }

    // Getter and Setter for _ground
    std::shared_ptr<Texture> getGround() const { return _ground; }
    void setGround(const std::shared_ptr<Texture>& ground) { _ground = ground; }
    
    // Getter and Setter for _background
    std::shared_ptr<Texture> getBackground() const { return _background; }
    void setBackground(const std::shared_ptr<Texture>& bg) { _background = bg; }

    // Getter and Setter for _platforms
    std::vector<Rect> getPlatforms() const { return _platforms; }
    void setPlatforms(const std::vector<Rect>& platforms) { _platforms = platforms; }

    // Getter and Setter for _waves
    std::vector<std::shared_ptr<WaveModel>> getWaves() const { return _waves; }
    void setWaves(const std::vector<std::shared_ptr<WaveModel>>& waves) { _waves = waves; }
    
    // Getter and Setter for _wallZones
    std::vector<std::pair<float, float>> getWalls() const { return _walls; }
    void setWalls(const std::vector<std::pair<float, float>>& walls) { _walls = walls; }
    
    // Getter and Setter for width of level (in terms of BG "tiles")
    int getBGN() { return _tiles; }
    void setBGN(int tiles) { _tiles = tiles; }
};

#endif /* __GB_LEVEL_MODEL_H__ */
