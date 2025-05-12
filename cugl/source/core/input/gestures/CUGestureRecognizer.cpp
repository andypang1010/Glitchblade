//
//  CUGestureRecognizer.cpp
//  Cornell University Game Library (CUGL)
//
//  This module implements support for DollarGestures, built upon the CUGL
//  Path2 interface. This divorces DollarGesture recognition from the device
//  input, focusing solely on the geometry. This module is based upon the
//  following work:
//
//  Dollar Gestures
//  Wobbrock, J.O., Wilson, A.D.and Li, Y. (2007). Gestures without libraries,
//  toolkits or training: A $1 recognizer for user interface prototypes.
//  Proceedings of the ACM Symposium on User Interface Software and Technology
//  (UIST '07). Newport, Rhode Island (October 7 - 10, 2007). New York :
//  ACM Press, pp. 159 - 168.
//  https ://dl.acm.org/citation.cfm?id=1294238
//
//  The Protractor Enhancement
//  Li, Y. (2010). Protractor: A fast and accurate gesture recognizer.
//  Proceedings of the ACM Conference on Human Factors in Computing Systems
//  (CHI '10). Atlanta, Georgia (April 10 - 15, 2010).New York : ACM Press,
//  pp. 2169 - 2172.
//  https ://dl.acm.org/citation.cfm?id=1753654/
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Authors: Chinmay Gangurde and Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/input/gestures/CUGestureRecognizer.h>
#include <limits>

using namespace cugl;

/** Macro to convert degrees to radians */
#define DEG2RAD(x) ((float)(x * M_PI / 180))

/** The default similarity threshhold */
const float GESTURE_RECOGNIZER_ACCURACY  = 0.8;
/** The default orientation tolerance */
const float GESTURE_RECOGNIZER_TOLERANCE = DEG2RAD(20);
/** The default number of points in a normalized gesture */
const int GESTURE_RECOGNIZER_NORM_LENGTH = 64;
/** The default grid size of the normalization space */
const int GESTURE_RECOGNIZER_NORM_SIZE   = 250;

/** Phi for Golden Mean calculation */
const float GESTURE_RECOGNIZER_PHI = static_cast<float>(0.5 * (-1.0 + std::sqrt(5.0)));

#pragma mark -
#pragma mark Static Helpers
/**
 * Returns the cumulative the magnitude of the gesture.
 *
 * @param points    an array of points representing a gesture.
 * @param psize     the number of points in the gesture.
 *
 * @return the cumulative the magnitude pf the gesture vector.
 */
static float path_length(const Vec2* points, size_t psize) {
    float d = 0.0;
    for (size_t ii = 1; ii < psize; ii++) {
        d += points[ii-1].distance(points[ii]);
    }
    return d;
}

/**
 * Returns the centroid of the gesture.
 *
 * @param points    a vector of points representing a gesture.
 *
 * @return the centroid of the gesture.
 */
static Vec2 path_centroid(const std::vector<cugl::Vec2>& points) {
    Vec2 result;
    for(auto it = points.begin(); it != points.end(); ++it) {
        result += *it;
    }
    
    result /= points.size();
    return result;
}

/**
 * Returns the centroid of the gesture.
 *
 * @param points    an array of points representing a gesture.
 * @param psize     the number of points in the gesture.
 *
 * @return the centroid of the gesture.
 */
static Vec2 path_centroid(const Vec2* points, size_t psize) {
    Vec2 result;
    for(size_t ii = 0; ii < psize; ++ii) {
        result += points[ii];
    }
    
    result /= psize;
    return result;
}

/**
 * Returns the orientation of the gesture.
 *
 * The orientation of a gesture is the vector from its first point
 * to the (normalized) centroid.
 *
 * @param points    an array of points representing a gesture.
 * @param psize     the number of points in the gesture.
 * @param bounds    the bounding box for the normalization space
 *
 * @return the orientation of the gesture.
 */
static Vec2 path_orientation(const Vec2* points, size_t psize, Size bounds) {
    Vec2 centroid;
    float minX = points[0].x;
    float maxX = minX;
    float minY = points[0].y;
    float maxY = minY;

    for(size_t ii = 0; ii < psize; ii++) {
        centroid += points[ii];
        minX = std::min(minX, points[ii].x);
        minY = std::min(minY, points[ii].y);
        maxX = std::max(maxX, points[ii].x);
        maxY = std::max(maxY, points[ii].y);
    }
    centroid /= psize;
    float width  = maxX-minX;
    float height = maxY-minY;

    Vec2 origin = (points[0]-centroid);
    if (width > 0) {
        origin.x *= bounds.width/width;
    }
    if (height > 0) {
        origin.y *= bounds.height/height;
    }

    return -origin;
}


/**
 * Returns the bounding rectangle for a gesture.
 *
 * @param points    a vector of points representing a gesture.
 *
 * @return the bounding rectangle for a gesture.
 */
static Size bound_dimensions(const std::vector<cugl::Vec2>& points) {
    float minX = points[0].x;
    float maxX = minX;
    float minY = points[0].y;
    float maxY = minY;

    for(auto it = points.begin()+1; it != points.end(); ++it) {
        minX = std::min(minX, it->x);
        minY = std::min(minY, it->y);
        maxX = std::max(maxX, it->x);
        maxY = std::max(maxY, it->y);
    }
    return Size(maxX - minX, maxY - minY);
}

/**
 * Returns a resampled copy of a raw gesture .
 *
 * The raw gestures may have different numbers of points but it is important
 * that we resample it into a fixed number of points. Ideally this value should
 * be between 30-128. A higher value means more accuracy and more computation
 * time. A lower value means lower accuracy and lower computation time.
 *
 * @param points    an array of points representing a gesture
 * @param psize     the number of points in the gesture
 * @param rsize     the number of points to resample to
 *
 * @return a vector of 2D points representing a resampled gesture.
 */
static std::vector<cugl::Vec2> resample_points(const Vec2* points, size_t psize, size_t rsize) {
    float intervalLength = path_length(points,psize)/(rsize - 1);
    
    float totalDistance = 0.0;
    std::vector<cugl::Vec2> result;

    result.push_back(points[0]);
    Vec2 prev  = points[0];
    for (size_t ii = 1; ii < psize;) {
        Vec2 curr = points[ii];
        float d = prev.distance(curr);
        if ((totalDistance + d) >= intervalLength) {
            float qx = prev.x + ((intervalLength-totalDistance) / d) * (curr.x-prev.x);
            float qy = prev.y + ((intervalLength-totalDistance) / d) * (curr.y-prev.y);
            Vec2 q(qx, qy);
            result.push_back(q);
            totalDistance = 0.0;
            prev = q;
        } else {
            totalDistance += d;
            prev = curr;
            ii++;
        }
    }
    
    // Round-off error means sometimes we forget last point
    if (result.size() < rsize) {
        result.push_back(points[psize-1]);
    }

    return result;
}

/**
 * Returns the indicative angle of a gesture.
 *
 * This angle is measured with respect to the centroid of the gesture
 * and the first point of the gesture. It is used to rotate the
 * gesture into a canonical position for normalization.
 *
 * @param points    a vector of points representing a gesture.
 *
 * @return angle in radians.
 */
static float indicative_angle(const std::vector<cugl::Vec2>& points) {
    Vec2 centroid = path_centroid(points);
    return std::atan2(centroid.y-points[0].y, centroid.x-points[0].x);
}

/**
 * Rotates the gesture by the specified angle.
 *
 * @param points    a vector of points representing a gesture.
 * @param angle     angle to rotate by
 */
static void rotate_by(std::vector<cugl::Vec2>& points, float angle) {
    Vec2 centroid = path_centroid(points);
    float cos = std::cos(angle);
    float sin = std::sin(angle);
    
    for(auto it = points.begin(); it != points.end(); ++it) {
        float qx = (it->x-centroid.x) * cos - (it->y-centroid.y) * sin + centroid.x;
        float qy = (it->x-centroid.x) * sin + (it->y-centroid.y) * cos + centroid.y;
        it->x = qx;
        it->y = qy;
    }
}

/**
 * Scales the gesture to a specific size for normalization.
 *
 * You can change this grid size. The current value is 200 x 200 which is moderate and
 * works well.
 *
 * @param points    a vector of points representing a gesture.
 * @param bounds    the bounding box for the normalization space
 */
static void scale_to(std::vector<cugl::Vec2>& points, const Size bounds) {
    Size box = bound_dimensions(points);
    for(auto it = points.begin(); it != points.end(); ++it) {
        it->x *= (bounds.width / box.width);
        it->y *= (bounds.height / box.height);
    }
}

/**
 * Translates the gesture so that is centroid is at the given position
 *
 * @param points    a vector of points representing a gesture.
 * @param centroid  the coordinates for the new centroid location
 */
static void translate_to(std::vector<cugl::Vec2>& points, const cugl::Vec2 centroid)
{
    Vec2 current = path_centroid(points);
    for(auto it = points.begin(); it != points.end(); ++it) {
        it->x += centroid.x - current.x;
        it->y += centroid.y - current.y;
    }
}

/**
 * Returns the squared distance between the two gestures.
 *
 * @param path1     a vector of points representing the first gesture.
 * @param path2     a vector of points representing the second gesture.
 *
 * @return the squared distance between the two gestures.
 */
static float path_distance(const std::vector<cugl::Vec2>& path1,
                           const std::vector<cugl::Vec2>& path2) {
    CUAssertLog(path1.size() == path2.size(),
                "The paths do not have equal lengths [%zu vs %zu]",
                path1.size(),path2.size());

    float d = 0.0;
    for (size_t ii = 0; ii < path1.size(); ii++) {
        d += path1[ii].distance(path2[ii]);
    }
    return d/path1.size();
}

/**
 * Returns the distance between two gestures after rotating the first one
 *
 * This method rotates the first gesture by the specified angle and then
 * computes the distance between it and the second.
 *
 * @param path1     a vector of points representing the first gesture.
 * @param path2     a vector of points representing the second gesture.
 * @param angle     the angle to rotate by
 *
 * @return the distance between two gestures after rotating the first one
 */
static float distance_at_angle(const std::vector<cugl::Vec2>& path1,
                               const std::vector<cugl::Vec2>& path2, float angle) {
    std::vector<Vec2> candidate = path1;
    rotate_by(candidate, angle);
    return path_distance(candidate, path2);
}


/**
 * Returns the best distance between two gestures according to constraints
 *
 * This method will iteratively rotate the first gesture to the best possible angle
 * within the given range, in order to get the gestures as close as possible. The
 * angle chosen will be within the error threshold of the actual optimum.
 *
 * @param path1     a vector of points representing the first gesture.
 * @param path2     a vector of points representing the second gesture.
 * @param start     start of the search range in radians
 * @param end       end of the search range in radians
 * @param error     the allowable error threshold for the angle chosen
 *
 * @return the best distance between two gestures according to constraints
 */
static float distance_at_best_angle(const std::vector<cugl::Vec2>& path1,
                                    const std::vector<cugl::Vec2>& path2,
                                    float start, float end, float error) {
    
    float x1 = static_cast<float>(GESTURE_RECOGNIZER_PHI * start +
                                  (1.0 - GESTURE_RECOGNIZER_PHI) * end);
    float f1 = distance_at_angle(path1, path2, x1);
    float x2 = static_cast<float>((1.0 - GESTURE_RECOGNIZER_PHI) * start +
                                  GESTURE_RECOGNIZER_PHI * end);
    float f2 = distance_at_angle(path1, path2, x2);
    
    while (std::abs(end - start) > error) {
        if (f1 < f2) {
            end = x2;
            x2 = x1;
            f2 = f1;
            x1 = static_cast<float>(GESTURE_RECOGNIZER_PHI * start +
                                    (1.0 - GESTURE_RECOGNIZER_PHI) * end);
            f1 = distance_at_angle(path1, path2, x1);
        } else {
            start = x1;
            x1 = x2;
            f1 = f2;
            x2 = static_cast<float>((1.0 - GESTURE_RECOGNIZER_PHI) * start +
                                    GESTURE_RECOGNIZER_PHI * end);
            f2 = distance_at_angle(path1, path2, x2);
        }
    }
    return std::min(f1, f2);
}

/**
 * Returns the cosing distance between the two gestures.
 *
 * This is the distance used the PROTRACTOR algorithm.
 *
 * @param path1     the feature vector of the first gesture
 * @param path2     the feature vector of the second gesture
 *
 * @return cosine distance between the two gestures.
 */
static float optimal_cosine_distance(const std::vector<float>& path1,
                                     const std::vector<float>& path2) {
    CUAssertLog(path1.size() == path2.size(),
                "The paths do not have equal lengths [%zu vs %zu]",
                path1.size(),path2.size());

    float a = 0.0;
    float b = 0.0;
    
    for (size_t ii = 0; ii < path1.size(); ii += 2) {
        a += path1[ii] * path2[ii]   + path1[ii+1] * path2[ii+1];
        b += path1[ii] * path2[ii+1] - path1[ii+1] * path2[ii];
    }
    float angle  = std::atan2(b,a);
    float cosine = a * std::cos(angle) + b * std::sin(angle);
    
    // Handle round-off error gracefully
    cosine = cosine > 1 ? 1 : (cosine < -1 ? -1 : cosine);
    return std::acos(cosine);
}

#pragma mark -
#pragma mark Constructors
/**
 * Initializes the engine object with a few default properties
 */
GestureRecognizer::GestureRecognizer() :
_accuracy(0),
_tolerance(-1),
_normlength(0) {
    _algorithm = Algorithm::ONEDOLLAR;
}

/**
 * Initializes an empty gesture recognizer with the default settings.
 *
 * Calling this method initializes the sample rate and the bounding box
 * for normalization. These values will be set to the defaults. The
 * recognizer will be empty, and therefore will not match any gestures
 * until some are added via {@link #addGesture}.
 *
 * @return true if the recognizer is initialized properly, false otherwise.
 */
bool GestureRecognizer::init() {
    _accuracy  = GESTURE_RECOGNIZER_ACCURACY;
    _tolerance = GESTURE_RECOGNIZER_TOLERANCE;
    _normlength = GESTURE_RECOGNIZER_NORM_LENGTH;
    _normbounds = Size(GESTURE_RECOGNIZER_NORM_SIZE,
                       GESTURE_RECOGNIZER_NORM_SIZE);
    return true;
}

/**
 * Initializes an empty gesture recognizer with the given settings.
 *
 * The recognier will be empty, and therefore will not match any gestures
 * until some are added via {@link #addGesture}.
 *
 * @param samples   The number of samples to use in a normalized gesture
 * @param bounds    The bounding box for normalized gestures
 *
 * @return true if the recognizer is initialized properly, false otherwise.
 */
bool GestureRecognizer::init(size_t samples, Size bounds) {
    _accuracy  = GESTURE_RECOGNIZER_ACCURACY;
    _tolerance = GESTURE_RECOGNIZER_TOLERANCE;
    _normlength = samples;
    _normbounds = Size(bounds,bounds);
    return _normlength > 0 && _normbounds.width > 0 && _normbounds.height > 0;
}

/**
 * Initializes the gesture recognizer with the given JSON entry
 *
 * The JSON entry supports the following attribute values:
 *
 * The JSON entry supports the following attribute values:
 *
 *      "algorithm": One of "onedollar" or "protractor"
 *      "accuracy":  A float with the similarity threshold
 *      "tolerance": A float with the rotational tolerance
 *      "samples":   An int for the number of sample points
 *      "bounds":    A two-element list of floats representing the
 *                   bounding box for normalized gestures
 *      "gestures":  A list of path entries
 *
 * The path entries should all follow the format used by the
 * {@link Path2} class.
 *
 * The path entries should all follow the format used by the
 * {@link Path2} class.
 *
 * @param data      The JSON object specifying the recognizer
 *
 * @return true if the recognizer is initialized properly, false otherwise.
 */
bool GestureRecognizer::initWithJson(const std::shared_ptr<JsonValue>& json) {
    std::string algorithm = json->getString("algorithm","onedollar");
    _algorithm = (algorithm == "protractor") ? Algorithm::PROTRACTOR : Algorithm::ONEDOLLAR;

    _accuracy  = json->getFloat("accuracy",GESTURE_RECOGNIZER_ACCURACY);
    _tolerance = json->getFloat("tolerance",GESTURE_RECOGNIZER_TOLERANCE);
    _normlength = json->getFloat("samples",GESTURE_RECOGNIZER_NORM_LENGTH);
    if (json->has("bounds") && json->get("bounds")->size() >= 2) {
        auto child = json->get("bounds");
        _normbounds.width  = child->get(0)->asFloat(GESTURE_RECOGNIZER_NORM_SIZE);
        _normbounds.height = child->get(1)->asFloat(GESTURE_RECOGNIZER_NORM_SIZE);
    }
    if (json->has("gestures")) {
        addGestures(json->get("gestures"));
    }
    return _normlength > 0 && _normbounds.width > 0 && _normbounds.height > 0;
}

/**
 * Empties the recongizer of all gestures and resets all attributes.
 *
 * This will set the sample size to 0, meaning no future matches are
 * possible. You must reinitialize the object to use it.
 */
void GestureRecognizer::dispose() {
    _templates.clear();
    _accuracy  = 0;
    _tolerance = -1;
    _normlength = 0;
}

#pragma mark -
#pragma mark Gesture Matching
/**
 * Returns the name of the gesture with the closest match to the given one.
 *
 * The match will be performed using the current active algorithm, with the
 * (rotationally invariant) similarity score stored in the reference
 * parameter. If there is no match within the similarity threshold, or
 * orientation tolerance, this method will return the empty string (though
 * it will still report the rotationally invariant similarity. A gesture
 * must consist of at least two points.
 *
 * @param points        an array of points representing a candidate gesture.
 * @param psize         the number of points in the candidate gesture.
 * @param similarity    parameter to store the similarity value
 *
 * @return the name of the gesture with the closest match to the given one.
 */
const std::string GestureRecognizer::match(const Vec2* points,  size_t psize,
                                           float& similarity) {
    CUAssertLog(psize > 1, "A gesture must have at least two points");
    if (_templates.empty()) {
        similarity = 0;
        return "";
    }
    
    Vec2 orientation = path_orientation(points, psize, _normbounds);
    std::vector<cugl::Vec2> normalized;
    std::vector<float> vectorized;
    if (_algorithm == Algorithm::ONEDOLLAR) {
        normalized = normalize(points,psize);
    } else {
        vectorized = vectorize(points,psize);
    }

    float bestdist = std::numeric_limits<float>::max();
    std::string bestmatch = "";
    for(auto it = _templates.begin(); it != _templates.end(); ++it) {
        float angle = Vec2::angle(it->second.getOrientation(), orientation);
        if (_tolerance < 0 || (-_tolerance < angle && angle < _tolerance)) {
            float distance = 0;
            switch (_algorithm) {
                case Algorithm::ONEDOLLAR:
                    distance = distance_at_best_angle(normalized, it->second.getPoints(),
                                                      -M_PI_4,M_PI_4,DEG2RAD(2.0));
                    break;
                case Algorithm::PROTRACTOR:
                    distance = optimal_cosine_distance(it->second.getVector(),vectorized);
                    break;
            }
            
            if (distance < bestdist) {
                bestdist  = distance;
                bestmatch = it->first;
            }
        }
    }
    
    if (_algorithm == Algorithm::ONEDOLLAR) {
        float halfdiag = 0.5f * ((Vec2)_normbounds).length();
        similarity = 1.0f - (bestdist / halfdiag);
    } else {
        similarity = std::atan2(1.0,bestdist)/M_PI_2;
    }
    
    if (similarity < _accuracy) {
        return "";
    }
    return bestmatch;
}

/**
 * Returns the similarity measure of the named gesture to this one.
 *
 * The similarity measure will be computed using the active algorithm. As
 * those algorithms are rotationally invariant, it will ignore the rotation
 * when computing that value. However, if the parameter invariant is set to
 * false, this method will return 0 for gestures not within the orientation
 * tolerance.
 *
 * If there is no gesture of the given name, this method will return 0. A
 * gesture must consist of at least two points.
 *
 * @param name      the name of the gesture to compare
 * @param points    an array of points representing a candidate gesture
 * @param psize     the number of points in the candidate gesture
 * @param invariant whether to ignore rotation when computing similarity
 *
 * @return the similarity measure of the named gesture to this one.
 */
float GestureRecognizer::similarity(const std::string name, const Vec2* points,
                                    size_t psize, bool invariant) {
    CUAssertLog(psize > 1, "A gesture must have at least two points");
    auto it = _templates.find(name);
    if (it == _templates.end()) {
        return 0;
    }
    
    if (!invariant && _tolerance >= 0) {
        Vec2 orientation = path_orientation(points, psize, _normbounds);
        float angle = Vec2::angle(it->second.getOrientation(), orientation);
        if (-_tolerance >= angle || angle >= _tolerance){
            return 0;
        }
    }
    
    float halfdiag;
    float distance;
    float score;
    switch (_algorithm) {
        case Algorithm::ONEDOLLAR:
            distance = distance_at_best_angle(normalize(points,psize), it->second.getPoints(),
                                              -DEG2RAD(45), DEG2RAD(45), DEG2RAD(2.0));
            halfdiag = 0.5f * ((Vec2)_normbounds).length();
            score = 1.0 - (distance / halfdiag);
            break;
        case Algorithm::PROTRACTOR:
            distance = optimal_cosine_distance(it->second.getVector(),vectorize(points,psize));
            score = std::atan2(1.0,distance)/M_PI_2;
            break;
    }
    return score;
}

#pragma mark -
#pragma mark Gesture Management
/**
 * Adds the given gesture to this recognizer using the given name.
 *
 * The gesture will be normalized before storing it. If the gesture has the
 * same name as an existing one, the previous gesture will be replaced.
 *
 * Note that uniqueness is determined according the current algorithm,
 * similarity threshold, and orientation tolerance. If any of these values
 * change, then uniqueness is no longer guaranteed.
 *
 * @param name      the gesture name
 * @param points    an array of points representing a gesture
 * @param psize     the number of points in the gesture
 * @param unique    whether to enforce gesture uniqueness
 *
 * @return true if the gesture was added to this recognizer
 */
bool GestureRecognizer::addGesture(const std::string name, const Vec2* points,
                                   size_t psize, bool unique) {
    CUAssertLog(psize > 1, "A gesture must have at least two points");

    Vec2 orientation = path_orientation(points, psize, _normbounds);
    std::vector<cugl::Vec2> normalized = normalize(points,psize);
    std::vector<float> vectorized = vectorize(points,psize);

    if (unique) {
        float halfdiag = 0.5f * ((Vec2)_normbounds).length();
        for(auto it = _templates.begin(); it != _templates.end(); ++it) {
            float distance;
            float score;
            switch (_algorithm) {
                case Algorithm::ONEDOLLAR:
                    distance = distance_at_best_angle(normalized, it->second.getPoints(),
                                                      -M_PI_4, M_PI_4, DEG2RAD(2.0));
                    score = 1.0 - (distance / halfdiag);
                    break;
                case Algorithm::PROTRACTOR:
                    distance = optimal_cosine_distance(it->second.getVector(),vectorized);
                    score = std::atan2(1.0,distance)/M_PI_2;
                    break;
            }
            if (score > _accuracy) {
                return false;
            }
        }
    }
    
    
    UnistrokeGesture gesture(name,orientation);
    gesture.setPoints(normalized);
    gesture.setVector(vectorized);
    _templates[name] = gesture;
    return true;
}

/**
 * Adds the given gesture to this recognizer using the given name.
 *
 * The JSON value should be a list of entries that all follow the format
 * used by the {@link Path2} class.
 *
 * If the optional parameter unique is set to true, this method will first
 * check that each gesture is unique (e.g. it does not exceed the similarity
 * threshold when compared to any existing gestures) before adding it. Any
 * gesture that matches one that came previously in the list will be dropped.
 *
 * Note that uniqueness is determined according the current algorithm,
 * similarity threshold, and orientation tolerance. If any of these values
 * change, then uniqueness is no longer guaranteed.
 *
 * @param data      The JSON object specifying the recognizer
 * @param unique    whether to enforce gesture uniqueness
 */
void GestureRecognizer::addGestures(const std::shared_ptr<JsonValue>& json, bool unique) {
    auto children = json->children();
    for(auto it = children.begin(); it != children.end(); ++it) {
        Path2 path(*it);
        addGesture((*it)->key(),path,unique);
    }
}

/**
 * Removes the gesture with the given name from the recongizer
 *
 * @param name  The name of the gesture to remove
 */
void GestureRecognizer::removeGesture(const std::string name) {
    _templates.erase(name);
}

/**
 * Returns a vector of all the gesture names
 *
 * @return a vector of all the gesture names
 */
std::vector<std::string> GestureRecognizer::getGestureNames() const {
    std::vector<std::string> result;
    for(auto it = _templates.begin(); it != _templates.end(); ++it) {
        result.push_back(it->first);
    }
    return result;
}

/**
 * Returns all the (normalized) gestures stored in this recognizer
 *
 * @return all the (normalized) gestures stored in this recognizer
 */
std::vector<UnistrokeGesture> GestureRecognizer::getGestures() const {
    std::vector<UnistrokeGesture> result;
    for(auto it = _templates.begin(); it != _templates.end(); ++it) {
        result.push_back(it->second);
    }
    return result;
}

#pragma mark -
#pragma mark Internal Helpers
/**
 * Returns a normalized gesture for the set of points.
 *
 * The input vector to this method defines a raw gesture. This method
 * applies the normalization steps outlined in the $1 gesture algorithm.
 * When complete, the new vector will have {@link #getSamplePoints} and
 * have a bounding box of {@link #getNormalizedBounds}.
 *
 * @param points    an array of points representing a gesture.
 * @param psize     the number of points in the gesture.
 *
 * @return a normalized gesture for the set of points..
 */
std::vector<cugl::Vec2> GestureRecognizer::normalize(const Vec2* points, size_t psize) {
    Vec2 centroid = path_centroid(points,psize);
    centroid -= points[0];
    std::vector<cugl::Vec2> result;
    result = resample_points(points, psize, _normlength);
    float angleInRadians = indicative_angle(result);
    rotate_by(result, -angleInRadians);
    scale_to(result, _normbounds);
    translate_to(result, Vec2(0,0));
    return result;
}

/**
 * Returns a high dimensional feature vector for the set of points.
 *
 * The input vector to this method defines a raw gesture. This method
 * applies the vectorization steps outlined in the PROCTRACTOR algorithm.
 * When complete, the new vector will have {@link #getSamplePoints}/2
 * elements.
 *
 * @param points    an array of points representing a gesture.
 * @param psize     the number of points in the gesture.
 *
 * @return a normalized gesture for the set of points..
 */
std::vector<float> GestureRecognizer::vectorize(const Vec2* points, size_t psize) {
    std::vector<Vec2> translated;
    translated = resample_points(points, psize, _normlength/4);
    translate_to(translated, Vec2::ZERO);
    
    float angle = std::atan2(translated[0].y, translated[0].x);
    float delta = 0;
    if (_tolerance >= 0) {
        // While this is in the paper, experiments show this does nothing at all.
        // We still get a 1.0 match on a 90° rotation. Useless.
        float orientation = M_PI_4*std::floor((angle+M_PI_4/2)/M_PI_4);
        delta = orientation-angle;
    } else {
        delta = -angle;
    }
    
    float sum = 0.0;
    std::vector<float> result;

    for(auto it = translated.begin(); it != translated.end(); ++it) {
        result.push_back(it->x*std::cos(delta)-it->y*std::sin(delta));
        result.push_back(it->y*std::cos(delta)+it->x*std::sin(delta));
        sum += it->x * it->x + it->y * it->y;
    }

    float magnitude = std::sqrt(sum);
    for(auto it = result.begin(); it != result.end(); ++it) {
        *it /= magnitude;
    }

    return result;
}

#pragma mark -
#pragma mark UnistrokeGesture
/**
 * Returns the (normalized) angles between the two gestures
 *
 * @param other The gesture to compare
 *
 * @return the (normalized) angles between the two gestures
 */
float UnistrokeGesture::getAngle(const UnistrokeGesture& other) const {
    return Vec2::angle(getOrientation(), other.getOrientation());
}

/**
 * Returns the $1 similarity between the two gestures
 *
 * @param other The gesture to compare
 *
 * @return the $1 similarity between the two gestures
 */
float UnistrokeGesture::getDollarSimilarity(const UnistrokeGesture& other) const {
    Size bounds = bound_dimensions(_points);
    float halfdiag = 0.5f * ((Vec2)bounds).length();

    float distance = distance_at_best_angle(_points, other._points,
                                            -M_PI_4, M_PI_4, DEG2RAD(2.0));
    return 1.0 - (distance / halfdiag);
}

/**
 * Returns the PROTRACTOR similarity between this gesture and the other
 *
 * @param other The gesture to compare
 *
 * @return the PROTRACTOR similarity between the two gestures
 */
float UnistrokeGesture::getProtractorSimilarity(const UnistrokeGesture& other) const {
    float distance = optimal_cosine_distance(_vector,other._vector);
    return std::atan2(1.0,distance)/M_PI_2;
}
