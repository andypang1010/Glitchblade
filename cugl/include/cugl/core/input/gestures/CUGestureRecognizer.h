//
//  CUGestureRecognizer.h
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
#ifndef __CU_GESTURE_RECOGNIZER_H__
#define __CU_GESTURE_RECOGNIZER_H__
#include <cugl/core/math/CUVec2.h>
#include <cugl/core/math/CUPath2.h>
#include <vector>
#include <unordered_map>

namespace cugl {

/** Forward declaration of CUGL class */
class JsonValue;

/**
 * A class representing a unistroke gesture
 *
 * This object represents a normalized unistroke gesture stored inside of
 * a gesture recognizer. As the normalization algorithm is deteremined by
 * the current settings, there are no publicly accessible constructors for
 * this class.
 */
class UnistrokeGesture {
private:
    /** Identifier string for a gesture **/
    std::string _name;
    
    /** Normalized vector representing the orientation of this gesture */
    Vec2 _orientation;
        
    /** 2D points forming this gesture */
    std::vector<Vec2> _points;

    /** A vectorized version of this gesture for the protractor method */
    std::vector<float> _vector;

private:
    /**
     * Creates a gesture with a given name and points
     *
     * The points are assumed to be normalized.
     *
     * @param name      The gesture name
     * @param points    The gesture point sequence
     */
    UnistrokeGesture(const std::string name, const Vec2 orientation) {
        _name = name;
        _orientation = orientation;
    }
    
    /**
     * Sets this gesture to have the given points
     *
     * The points are assumed to be normalized.
     *
     * @param points    The gesture point sequence
     */
    void setPoints(const std::vector<Vec2> points) {
        _points = points;
    }

    void setVector(const std::vector<float> vector) {
        _vector = vector;
    }

    // All access to the constructors
    friend class GestureRecognizer;

public:
    /**
     * Creates an uninitialized gesture with no information
     */
    UnistrokeGesture() : _name("") {}

    /**
     * Returns the string identifier of this gesture
     *
     * @return the string identifier of this gesture
     */
    const std::string getName() const { return _name; }

    /**
     * Returns the orientation of the gesture.
     *
     * The orientation is defined as the vector from the initial point
     * to a (normalized) centroid. This allows us to control the
     * rotation of gestures.
     *
     * @return the orientation of the gesture.
     */
    const Vec2& getOrientation() const { return _orientation; }
    
    /**
     * Returns the vector of 2D points representing this gesture
     *
     * @return the vector of 2D points representing this gesture
     */
    const std::vector<Vec2>& getPoints() const { return _points; }
    
    /**
     * Returns the vectorized representation of this gesture.
     *
     * The vectorized representation is a normalized, high dimensional
     * vector.
     *
     * @return the vectorized representation of this gesture.
     */
    const std::vector<float>& getVector() const { return _vector; }

    /**
     * Returns the (normalized) angles between the two gestures
     *
     * @param other The gesture to compare
     *
     * @return the (normalized) angles between the two gestures
     */
    float getAngle(const UnistrokeGesture& other) const;

    /**
     * Returns the $1 similarity between the two gestures
     *
     * @param other The gesture to compare
     *
     * @return the $1 similarity between the two gestures
     */
    float getDollarSimilarity(const UnistrokeGesture& other) const;

    /**
     * Returns the PROTRACTOR similarity between this gesture and the other
     *
     * @param other The gesture to compare
     *
     * @return the PROTRACTOR similarity between the two gestures
     */
    float getProtractorSimilarity(const UnistrokeGesture& other) const;

};

#pragma mark -
/**
 * A class representing the gesture recognition engine.
 *
 * This class contains all the required funtionality needed for recognizing
 * user defined gestures. It's major responsibilities are to store a collection
 * of template gestures (for comparison), and normalize a gesture to a grid of
 * fixed size. Gesture matches are determined by computing a similarity score
 * between normalized instances.
 *
 * Normalization involves resampling the gesture to a fixed number of points,
 * as defined by {@link #getSampleSize}. In addition, the gesture is rescaled
 * to a box defined by {@link #getNormalizedBounds}.
 *
 * The recognition algorithm can use either the traditional $1 algorithm
 * algorithm or the PROTACTOR method, depending on what suits you and what
 * gives you the best results for your application. For more information on
 * these algorithms, see
 *
 *    https ://dl.acm.org/citation.cfm?id=1294238
 *    https ://dl.acm.org/citation.cfm?id=1753654/
 *
 * Both of these algorithms are rotationally oblivious, meaning that they
 * can recognize the gestures at any orientation. Typically this is not what
 * we want in game development, however. Therfore, this recognizer includes
 * the option to reject any matches whose angles of rotation exceed a certain
 * threshold. See {@link #getOrientationTolerance} for more information.
 *
 * Similarity is determined on a scale of 0 to 1 where 1 is a complete match
 * and 0 is no match at all. A pure 0 is difficult to achieve. By default, we
 * consider any gesture a possible match if it has a similarity of at least
 * 0.8.
 */
class GestureRecognizer {
public:
    /**
     * An enumeration listing the type of matching algorithms
     */
    enum class Algorithm {
        /**
         * The classic $1 algorithm (default)
         *
         * This is a slower, but more accurate method. It has to
         * to iterate through several angles when comparing two
         * gestures to each other
         */
        ONEDOLLAR,
        /**
         * The PROTRACTOR algorithm
         *
         * This is a faster, less accurate method. It converts the
         * gesture into a high dimensional feature vector so that
         * it only needs one step to compare two gestures.
         */
        PROTRACTOR
    };
    
private:
    /** The collection of template gestures for matching */
    std::unordered_map<std::string,UnistrokeGesture> _templates;
    
    /** The recognition algorithm used by the engine */
    Algorithm _algorithm;
    
    /** The accuracy threshhold */
    float _accuracy;
    /** The orientation tolerance */
    float _tolerance;
    /** The bounding box of the normalization space */
    Size _normbounds;
    /** The number of points in a normalized gesture */
    size_t _normlength;
    
#pragma mark Constructors
public:
    /**
     * Creates an uninitialized Gesture Recognizer.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    GestureRecognizer();
    
    /**
     * Deletes this recognizer and all of its resources.
     */
    ~GestureRecognizer() { }
    
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
    bool init();
    
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
    bool init(size_t samples, Size bounds);
    
    /**
     * Initializes the gesture recognizer with the given JSON entry
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
     * @param json      The JSON object specifying the recognizer
     *
     * @return true if the recognizer is initialized properly, false otherwise.
     */
    bool initWithJson(const std::shared_ptr<JsonValue>& json);
    
    /**
     * Empties the recongizer of all gestures and resets all attributes.
     *
     * This will set the sample size to 0, meaning no future matches are
     * possible. You must reinitialize the object to use it.
     */
    void dispose();
    
    /**
     * Returns a newly allocated gesture recognizer with the default settings.
     *
     * The sample rate and normalization bounds will be set the default values.
     * The recognier will be empty, and therefore will not match any gestures
     * until some are added via {@link #addGesture}.
     *
     * @return a newly allocated gesture recognizer with the default settings.
     */
    static std::shared_ptr<GestureRecognizer> alloc() {
        std::shared_ptr<GestureRecognizer> result = std::make_shared<GestureRecognizer>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated gesture recognizer with the given accuracy.
     *
     * The recognier will be empty, and therefore will not match any gestures
     * until some are added via {@link #addGesture}.
     *
     * @param samples   The number of samples to use in a normalized gesture
     * @param bounds    The bounding box for normalized gestures
     *
     * @return a newly allocated gesture recognizer with the default accuracy.
     */
    static std::shared_ptr<GestureRecognizer> alloc(size_t samples, Size bounds) {
        std::shared_ptr<GestureRecognizer> result = std::make_shared<GestureRecognizer>();
        return (result->init(samples,bounds) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated gesture recognizer with the given JSON entry
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
     * @param json      The JSON object specifying the recognizer
     *
     * @return a newly allocated gesture recognizer with the given JSON entry
     */
    static std::shared_ptr<GestureRecognizer> allocWithJson(const std::shared_ptr<JsonValue>& json) {
        std::shared_ptr<GestureRecognizer> result = std::make_shared<GestureRecognizer>();
        return (result->initWithJson(json) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Returns the sample rate of a normalized gesture
     *
     * When a gesture is either added to the recognizer or posed as a
     * candidate for matching, it will first be normalized. All
     * normalized gestures will have this many points (the the
     * PROCTRACTOR algorithm will only use 1/4 this many points).
     *
     * Increasing this number will hurt performance, while decreasing
     * this value will hurt accuracy. Ideally it should be between 30-128.
     * By default this value is 64 (and hence 16 for the PROTRACTOR 
     * algorithm).
     *
     * This value is set when the gesture recognizer is allocated and
     * cannot be changed. Changing this value would require the
     * reintialization of all gestures.
     *
     * @return the sample rate of a normalized gesture
     */
    float getSampleSize() const { return _normlength; }
    
    /**
     * Returns the bounding box of the normalization space
     *
     * When a gesture is normalized, it is resized to so that its
     * bounding box matches that of all other gestures. This value is
     * the size of the box. By default it is 250x250.
     *
     * This value is set when the gesture recognizer is allocated and
     * cannot be changed. Changing this value would require the
     * reintialization of all gestures.
     *
     * @return the bounding box of the normalization space
     */
    Size getNormalizedBounds() const { return _normbounds; }
    
    /**
     * Returns the current matching algorithm
     *
     * This gesture recognizer can use either the classic $1 algorithm
     * or the PROTRACTOR algorithm. The PROTRACTOR is faster with less
     * accuracy. However the need for speed really depends on the number
     * of gestures stored in this recognizer. Determining which algorithm
     * is best is typically a matter of experiementation.
     *
     * Note that changing this value can possible invalidate any uniqueness
     * constraints previously applied to the gestures. That is because
     * uniqueness is determined using the active algorithm. If uniqueness
     * is important, we recommend that you clear the existing gestures and
     * add them back.
     *
     * @return the current matching algorithm
     */
    Algorithm getAlgorithm() const { return _algorithm; }

    /**
     * Sets the current matching algorithm
     *
     * This gesture recognizer can use either the classic $1 algorithm
     * or the PROTRACTOR algorithm. The PROTRACTOR is faster with less
     * accuracy. However the need for speed really depends on the number
     * of gestures stored in this recognizer. Determining which algorithm
     * is best is typically a matter of experiementation.
     *
     * Note that changing this value can possible invalidate any uniqueness
     * constraints previously applied to the gestures. That is because
     * uniqueness is determined using the active algorithm. If uniqueness
     * is important, we recommend that you clear the existing gestures and
     * add them back.
     *
     * @param algorithm    the matching algorithm
     */
    void setAlgorithm(Algorithm algorithm) { _algorithm = algorithm; }

    /**
     * Returns the similarity threshold
     *
     * When matching a candidate gesture against the stored values, we only
     * consider matches whose similarity value is above this threshold. If
     * this value is non-zero, it is possible that a candidate gesture will
     * have no matches. By default this value is 0.8.
     *
     * Note that changing this value can possible invalidate any uniqueness
     * constraints previously applied to the gestures. That is because
     * uniqueness is determined using the current similarity threshold. If
     * uniqueness is important, we recommend that you clear the existing
     * gestures and add them back.
     *
     * @return the similarity threshold
     */
    float getSimilarityThreshold() const { return _accuracy; }
    
    /**
     * Sets the similarity threshold
     *
     * When matching a candidate gesture against the stored values, we only
     * consider matches whose similarity value is above this threshold. If
     * this value is non-zero, it is possible that a candidate gesture will
     * have no matches. By default this value is 0.3.
     *
     * Note that changing this value can possible invalidate any uniqueness
     * constraints previously applied to the gestures. That is because
     * uniqueness is determined using the current similarity threshold. If
     * uniqueness is important, we recommend that you clear the existing
     * gestures and add them back.
     *
     * @param accuracy  the similarity threshold
     */
    void setSimilarityThreshold(float accuracy) { _accuracy = accuracy; }
    
    /**
     * Returns the rotational tolerance for gesture matching.
     *
     * The matching algorithms are rotationally oblivious, meaning that they
     * can recognize the gestures at any orientation. Typically this is not
     * what we want in game development, however. If a gesture is rotated too
     * far, we want to reject it.
     *
     * The tolerance is the maximum allowable angle (in radians) of rotation
     * for a gesture to be recognized. The angle of a gesture is measured
     * using the vector from its first point to the (normalized) centroid.
     * If θ is the angle between the gesture and potential match, it will
     * be rejected if it is greater than +/- the tolerance.
     *
     * By default this value is 20°. Setting this to a negative value will
     * allow free rotation of gestures.
     *
     * Note that changing this value can possible invalidate any uniqueness
     * constraints previously applied to the gestures. That is because
     * uniqueness is determined using the current rotational tolerance. If
     * uniqueness is important, we recommend that you clear the existing
     * gestures and add them back.
     *
     * @return the rotational tolerance for gesture matching.
     */
    float getOrientationTolerance() const { return _tolerance; }
    
    /**
     * Sets the rotational tolerance for gesture matching.
     *
     * The matching algorithms are rotationally oblivious, meaning that they
     * can recognize the gestures at any orientation. Typically this is not
     * what we want in game development, however. If a gesture is rotated too
     * far, we want to reject it.
     *
     * The tolerance is the maximum allowable angle (in radians) of rotation
     * for a gesture to be recognized. The angle of a gesture is measured
     * using the vector from its first point to the (normalized) centroid.
     * If θ is the angle between the gesture and potential match, it will
     * be rejected if it is greater than +/- the tolerance.
     *
     * By default this value is 20°. Setting this to a negative value will
     * allow free rotation of gestures.
     *
     * Note that changing this value can possible invalidate any uniqueness
     * constraints previously applied to the gestures. That is because
     * uniqueness is determined using the current rotational tolerance. If
     * uniqueness is important, we recommend that you clear the existing
     * gestures and add them back.
     *
     * @param tolerance    the rotational tolerance for gesture matching.
     */
    void setOrientationTolerance(float tolerance) { _tolerance = tolerance; }
    
#pragma mark Gesture Matching
    /**
     * Returns the name of the gesture with the closest match to the given one.
     *
     * The match will be performed using the current active algorithm. If
     * there is no match within the similarity threshold or orientation
     * tolerance, this method will return the empty string. A gesture must
     * consist of at least two points.
     *
     * @param points    a vector of points representing a candidate gesture.
     *
     * @return the name of the gesture with the closest match to the given one.
     */
    const std::string match(const std::vector<Vec2> points) {
        float similarity;
        return match(points,similarity);
    }
    
    /**
     * Returns the name of the gesture with the closest match to the given one.
     *
     * The match will be performed using the current active algorithm. If
     * there is no match within the similarity threshold or orientation
     * tolerance, this method will return the empty string. A gesture must
     * consist of at least two points.
     *
     * @param points    an array of points representing a candidate gesture.
     * @param psize     the number of points in the candidate gesture.
     *
     * @return the name of the gesture with the closest match to the given one.
     */
    const std::string match(const Vec2* points,  size_t psize) {
        float similarity;
        return match(points,psize,similarity);
    }
    
    /**
     * Returns the name of the gesture with the closest match to the given one.
     *
     * The match will be performed using the current active algorithm. If
     * there is no match within the similarity threshold or orientation
     * tolerance, this method will return the empty string. A gesture must
     * consist of at least two points.
     *
     * When matching as a gesture, the path will be treated as a linear sequence
     * of points. Corner classifications and whether the path is closed will be
     * ignored.
     *
     * @param path  a path representing a candidate gesture.
     *
     * @return the name of the gesture with the closest match to the given one.
     */
    const std::string match(const Path2& path) {
        float similarity;
        return match(path,similarity);
    }
    
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
     * @param points        a vector of points representing a candidate gesture.
     * @param similarity    parameter to store the similarity score
     *
     * @return the name of the gesture with the closest match to the given one.
     */
    const std::string match(const std::vector<Vec2> points, float& similarity) {
        return match(points.data(),points.size(),similarity);
    }
    
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
    const std::string match(const Vec2* points,  size_t psize, float& similarity);
    
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
     * When matching as a gesture, the path will be treated as a linear sequence
     * of points. Corner classifications and whether the path is closed will be
     * ignored.
     *
     * @param path          a path representing a candidate gesture.
     * @param similarity    parameter to store the similarity value
     *
     * @return the name of the gesture with the closest match to the given one.
     */
    const std::string match(const Path2& path, float& similarity) {
        return match(path.vertices.data(),path.vertices.size(),similarity);
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
     * @param points    a vector of points representing a candidate gesture
     * @param invariant whether to ignore rotation when computing similarity
     *
     * @return the similarity measure of the named gesture to this one.
     */
    float similarity(const std::string name, const std::vector<Vec2>& points,
                     bool invariant=true) {
        return similarity(name, points.data(),points.size(),invariant);
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
    float similarity(const std::string name, const Vec2* points, size_t psize,
                     bool invariant=true);
    
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
     * When matching as a gesture, the path will be treated as a linear sequence
     * of points. Corner classifications and whether the path is closed will be
     * ignored.
     *
     * @param name      the name of the gesture to compare
     * @param path      a path representing a candidate gesture
     * @param invariant whether to ignore rotation when computing similarity
     *
     * @return the similarity measure of the named gesture to this one.
     */
    float similarity(const std::string name, const Path2& path,bool invariant=true) {
        return similarity(name, path.vertices.data(),path.vertices.size(),invariant);
    }
    
#pragma mark Gesture Management
    /**
     * Adds the given gesture to this recognizer using the given name.
     *
     * The gesture will be normalized before storing it. If the gesture has the
     * same name as an existing one, the previous gesture will be replaced.
     *
     * If the optional parameter unique is set to true, this method will first
     * check that the gesture is unique (e.g. it does not exceed the similarity
     * threshold when compared to any existing gestures) before adding it. If
     * the gesture is too close to an existing one, this method will return false.
     *
     * Note that uniqueness is determined according the current algorithm,
     * similarity threshold, and orientation tolerance. If any of these values
     * change, then uniqueness is no longer guaranteed.
     *
     * @param name      the gesture name
     * @param points    a vector of points representing a gesture
     * @param unique    whether to enforce gesture uniqueness
     *
     * @return true if the gesture was added to this recognizer
     */
    bool addGesture(const std::string name, const std::vector<Vec2>& points, bool unique=false) {
        return addGesture(name, points.data(), points.size(),unique);
    }
    
    /**
     * Adds the given gesture to this recognizer using the given name.
     *
     * The gesture will be normalized before storing it. If the gesture has the
     * same name as an existing one, the previous gesture will be replaced.
     *
     * If the optional parameter unique is set to true, this method will first
     * check that the gesture is unique (e.g. it does not exceed the similarity
     * threshold when compared to any existing gestures) before adding it. If
     * the gesture is too close to an existing one, this method will return false.
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
    bool addGesture(const std::string name, const Vec2* points, size_t psize, bool unique=false);
    
    /**
     * Adds the given gesture to this recognizer using the given name.
     *
     * The gesture will be normalized before storing it. If the gesture has the
     * same name as an existing one, the previous gesture will be replaced.
     *
     * If the optional parameter unique is set to true, this method will first
     * check that the gesture is unique (e.g. it does not exceed the similarity
     * threshold when compared to any existing gestures) before adding it. If
     * the gesture is too close to an existing one, this method will return false.
     *
     * Note that uniqueness is determined according the current algorithm,
     * similarity threshold, and orientation tolerance. If any of these values
     * change, then uniqueness is no longer guaranteed.
     *
     * When converting to a gesture, the path will be treated as a linear sequence
     * of points. Corner classifications and whether the path is closed will be
     * ignored.
     *
     * @param name      the name of the gesture to compare
     * @param path      a path representing a candidate gesture
     * @param unique    whether to enforce gesture uniqueness
     *
     * @return true if the gesture was added to this recognizer
     */
    bool addGesture(const std::string name, const Path2& path, bool unique=false) {
        return addGesture(name, path.vertices.data(), path.vertices.size(), unique);
    }
    
    /**
     * Adds all of the gestures in the given JSON specification
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
     * @param json      The JSON object specifying the recognizer
     * @param unique    whether to enforce gesture uniqueness
     */
    void addGestures(const std::shared_ptr<JsonValue>& json, bool unique=false);
    
    /**
     * Removes the gesture with the given name from the recongizer
     *
     * @param name  The name of the gesture to remove
     */
    void removeGesture(const std::string name);
    
    /**
     * Removes all gestures from this recognizer
     */
    void clearGestures() {
        _templates.clear();
    }
    
    /**
     * Returns true if this recognizer has a gesture of the given name
     *
     * @param name      the name of the gesture to query
     *
     * @return true if this recognizer has a gesture of the given name
     */
    bool hasGesture(const std::string name) const {
        return _templates.find(name) != _templates.end();
    }
    
    /**
     * Returns (a copy of) gesture of the given name
     *
     * If there is no gesture of that name, it will return an empty gesture.
     *
     * @param name      the name of the gesture to query
     *
     * @return (a copy of) gesture of the given name
     */
    const UnistrokeGesture getGesture(const std::string name) const {
        auto it = _templates.find(name);
        if (it == _templates.end()) {
            return UnistrokeGesture();
        }
        return it->second;
    }
    
    /**
     * Returns a vector of all the gesture names
     *
     * @return a vector of all the gesture names
     */
    std::vector<std::string> getGestureNames() const;
    
    /**
     * Returns all the (normalized) gestures stored in this recognizer
     *
     * @return all the (normalized) gestures stored in this recognizer
     */
    std::vector<UnistrokeGesture> getGestures() const;
    
#pragma mark Internal Helpers
private:
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
    std::vector<Vec2> normalize(const Vec2* points, size_t psize);
    
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
    std::vector<float> vectorize(const Vec2* points, size_t psize);
};

}
#endif /* __CU_GESTURE_RECOGNIZER_H__ */
