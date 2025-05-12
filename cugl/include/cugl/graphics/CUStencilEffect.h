//
//  CUStencilEffect.h
//  Cornell University Game Library (CUGL)
//
//  The class Spritebatch does support a basic stencil effects.  In order to
//  support SVG files, these effects became fairly elaborate, as they are
//  splitting the stencil space in half, and coordinating between the two
//  halves. Therefore, we decided to pull the functionality out of Spritebatch
//  into its own module.
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
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_STENCIL_EFFECT_H__
#define __CU_STENCIL_EFFECT_H__
#include <cugl/graphics/CUGraphicsBase.h>

// WEIRD Windows work-around
#ifdef CLIP_MASK
#undef CLIP_MASK
#endif

// References to the "two" stencil buffers
/** Neither buffer */
#define STENCIL_NONE            0x000
/** The lower buffer */
#define STENCIL_LOWER           0x001
/** The upper buffer */
#define STENCIL_UPPER           0x002
/** Both buffers */
#define STENCIL_BOTH            0x003

namespace cugl {

    /**
     * The classes and functions needed to construct a graphics pipeline.
     *
     * Initially these were part of the core CUGL module (everyone wants graphics,
     * right). However, after student demand for a headless option that did not
     * have so many OpenGL dependencies, this was factored out.
     */
    namespace graphics {

// Forward declarations
class Shader;

/**
 * An enum to support stenciling effects.
 *
 * A {@link SpriteBatch} can support many types of stencil effects. Classic
 * stencil effects including clipping (limiting drawing to a specific region)
 * or masking (prohibiting drawing to a specific region). The stencil effects
 * supported are designed with {@link scene2::CanvasNode} in mind as the primary
 * use case.
 *
 * In particular, stencil effects are designed to support simple constructive
 * area geometry operations. You can union, intersect, or subtract stencil
 * regions to produce the relevant effects. However, this is only used for
 * drawing and does not actually construct the associated geometries.
 *
 * To support the CAG operations, the sprite batch stencil buffer has two
 * areas: low and high. Operations can be applied to one or both of these
 * regions. All binary operations are operations between these two regions.
 * For example, {@link StencilEffect#CLIP_MASK} will restrict all drawing to
 * the stencil region defined in the low buffer, while also prohibiting any
 * drawing to the stencil region in the high buffer. This has the visible effect
 * of "subtracting" the high buffer from the low buffer.
 *
 * The CAG operations are only supported at the binary level, as we only have
 * two halves of the stencil buffer. However, using non-drawing effects like
 * {@link StencilEffect#CLIP_WIPE} or {@link StencilEffect#CLIP_CARVE}, it is
 * possible to produce more interesting nested expressions.
 *
 * Note that when using split-buffer operations, only one of the operations
 * will modify the stencil buffer. That is why there no effects such as
 * `FILL_WIPE` or `CLAMP_STAMP`.
 */
enum class StencilEffect : int {
    /**
     * Differs the to the existing OpenGL stencil settings. (DEFAULT)
     *
     * This effect neither enables nor disables the stencil buffer. Instead
     * it uses the existing OpenGL settings.  This is the effect that you
     * should use when you need to manipulate the stencil buffer directly.
     */
    NATIVE = 0,
    
    /**
     * Disables any stencil effects.
     *
     * This effect directs a {@link SpriteBatch} to ignore the stencil buffer
     * (both halves) when drawing.  However, it does not clear the contents
     * of the stencil buffer.  To clear the stencil buffer, you will need to
     * call {@link cugl::graphics::stencil::clearBuffer}.
     */
    NONE = 1,
    
    /**
     * Restrict all drawing to the unified stencil region.
     *
     * In order for this effect to do anything, you must have created a
     * stencil region with {@link StencilEffect#STAMP} or one of its variants.
     * This effect will process the drawing commands normally, but restrict all
     * drawing to the stencil region. This can be used to quickly draw
     * non-convex shapes by making a stencil and drawing a rectangle over
     * the stencil.
     *
     * This effect is the same as {@link StencilEffect#CLIP_JOIN} in that it respects
     * the union of the two halves of the stencil buffer.
     */
    CLIP = 2,
    
    /**
     * Prohibits all drawing to the unified stencil region.
     *
     * In order for this effect to do anything, you must have created a
     * stencil region with {@link StencilEffect#STAMP} or one of its variants.
     * This effect will process the drawing commands normally, but reject any
     * attempts to draw to the stencil region. This can be used to quickly
     * draw shape borders on top of a solid shape.
     *
     * This effect is the same as {@link StencilEffect#MASK_JOIN} in that it respects
     * the union of the two halves of the stencil buffer.
     */
    MASK = 3,
    
    /**
     * Restrict all drawing to the unified stencil region.
     *
     * In order for this effect to do anything, you must have created a
     * stencil region with {@link StencilEffect#STAMP} or one of its variants.
     * This effect will process the drawing commands normally, but restrict all
     * drawing to the stencil region. This can be used to quickly draw
     * non-convex shapes by making a stencil and drawing a rectangle over
     * the stencil.
     *
     * This effect is different from {@link StencilEffect#CLIP} in that it will
     * zero out the pixels it draws in the stencil buffer, effectively removing
     * them from the stencil region. In many applications, this is a fast
     * way to clear the stencil buffer once it is no longer needed.
     *
     * This effect is the same as {@link StencilEffect#FILL_JOIN} in that it respects
     * the union of the two halves of the stencil buffer.
     */
    FILL = 4,
    
    /**
     * Erases from the unified stencil region.
     *
     * This effect will not draw anything to the screen. Instead, it will
     * only draw to the stencil buffer directly. Any pixel drawn will be
     * zeroed in the buffer, removing it from the stencil region. The
     * effect {@link StencilEffect#FILL} is a combination of this and
     * {@link StencilEffect#CLIP}. Again, this is a potential optimization
     * for clearing the stencil buffer. However, on most tiled-based GPUs,
     * it is probably faster to simply clear the whole buffer.
     */
    WIPE = 5,
    
    /**
     * Adds a stencil region the unified buffer
     *
     * This effect will not have any immediate visible effects. Instead it
     * creates a stencil region for the effects such as {@link StencilEffect#CLIP},
     * {@link StencilEffect#MASK}, and the like.
     *
     * The shapes are drawn to the stencil buffer using a nonzero fill
     * rule. This has the advantage that (unlike an even-odd fill rule)
     * stamps are additive and can be drawn on top of each other. However,
     * it has the disadvantage that it requires both halves of the stencil
     * buffer to store the stamp (which part of the stamp is in which half
     * is undefined).
     *
     * While this effect implements a nonzero fill rule faithfully, there
     * are technical limitations. The size of the stencil buffer means
     * that more than 256 overlapping polygons of the same orientation
     * will cause unpredictable effects. If this is a problem, use an
     * even odd fill rule instead like {@link StencilEffect#STAMP_NONE}
     * (which has no such limitations).
     */
    STAMP = 6,
    
    /**
     * Adds a stencil region the lower buffer
     *
     * This effect will not have any immediate visible effects. Instead it
     * creates a stencil region for the effects such as {@link StencilEffect#CLIP},
     * {@link StencilEffect#MASK}, and the like.
     *
     * Like {@link StencilEffect#STAMP}, shapes are drawn to the stencil buffer
     * instead of the screen. But unlike stamp, this effect is always additive.
     * It ignores path orientation, and does not support holes. This allows
     * the effect to implement a nonzero fill rule while using only half
     * of the buffer. This effect is equivalent to {@link StencilEffect#CARVE_NONE}
     * in that it uses only the lower half.
     *
     * The primary application of this effect is to create stencils from
     * extruded paths so that overlapping sections are not drawn twice
     * (which has negative effects on alpha blending).
     */
    CARVE = 7,
    
    /**
     * Limits drawing so that each pixel is updated once.
     *
     * This effect is a variation of {@link StencilEffect#CARVE} that also draws
     * as it writes to the stencil buffer.  This guarantees that each pixel is
     * updated exactly once. This is used by extruded paths so that
     * overlapping sections are not drawn twice (which has negative
     * effects on alpha blending).
     *
     * This effect is equivalent to {@link StencilEffect#CLAMP_NONE} in that
     * it uses only the lower half.
     */
    CLAMP = 8,
    
    /**
     * Applies {@link StencilEffect#CLIP} using the upper stencil buffer only.
     *
     * As with {@link StencilEffect#CLIP}, this effect restricts drawing to
     * the stencil region. However, this effect only uses the stencil region
     * present in the upper stencil buffer.
     *
     * This effect is designed to be used with stencil regions created by
     * {@link StencilEffect#NONE_STAMP}. While it can be used by a stencil
     * region created by {@link StencilEffect#STAMP}, the lower stencil buffer
     * is ignored, and hence the results are unpredictable.
     */
    NONE_CLIP = 9,
    
    /**
     * Applies {@link StencilEffect#MASK} using the upper stencil buffer only.
     *
     * As with {@link StencilEffect#MASK}, this effect prohibits drawing to
     * the stencil region. However, this effect only uses the stencil region
     * present in the upper stencil buffer.
     *
     * This effect is designed to be used with stencil regions created by
     * {@link StencilEffect#NONE_STAMP}. While it can be used by a stencil
     * region created by {@link StencilEffect#STAMP}, the lower stencil buffer
     * is ignored, and hence the results are unpredictable.
     */
    NONE_MASK = 10,
    
    /**
     * Applies {@link StencilEffect#FILL} using the upper stencil buffer only.
     *
     * As with {@link StencilEffect#FILL}, this effect limits drawing to
     * the stencil region. However, this effect only uses the stencil region
     * present in the upper stencil buffer.  It also only zeroes out the upper
     * stencil buffer.
     *
     * This effect is designed to be used with stencil regions created by
     * {@link StencilEffect#NONE_STAMP}. While it can be used by a stencil
     * region created by {@link StencilEffect#STAMP}, the lower stencil buffer
     * is ignored, and hence the results are unpredictable.
     */
    NONE_FILL = 11,
    
    /**
     * Applies {@link StencilEffect#WIPE} using the upper stencil buffer only.
     *
     * As with {@link StencilEffect#WIPE}, this effect zeroes out the stencil
     * region, erasing parts of it. However, its effects are limited to the upper
     * stencil region.
     *
     * This effect is designed to be used with stencil regions created by
     * {@link StencilEffect#NONE_STAMP}. While it can be used by a stencil
     * region created by {@link StencilEffect#STAMP}, the lower stencil buffer
     * is ignored, and hence the results are unpredictable.
     */
    NONE_WIPE = 12,
    
    /**
     * Adds a stencil region to the upper buffer
     *
     * This effect will not have any immediate visible effect on the screen
     * screen. Instead, it creates a stencil region for the effects such as
     * {@link StencilEffect#CLIP}, {@link StencilEffect#MASK}, and the like.
     *
     * Unlike {@link StencilEffect#STAMP}, the region created is limited to
     * the upper half of the stencil buffer. That is because the shapes are
     * drawn to the buffer with an even-odd fill rule (which does not require
     * the full stencil buffer to implement). This has the disadvantage
     * that stamps drawn on top of each other have an "erasing" effect.
     * However, it has the advantage that the this stamp supports a wider
     * array of effects than the simple stamp effect.
     *
     * Use {@link StencilEffect#NONE_CLAMP} if you have an simple stencil with
     * no holes that you wish to write to the upper half of the buffer.
     */
    NONE_STAMP = 13,
    
    /**
     * Adds a stencil region to the upper buffer
     *
     * This value will not have any immediate visible effect on the screen.
     * Instead, it creates a stencil region for the effects such as
     * {@link StencilEffect#CLIP}, {@link StencilEffect#MASK}, and the like.
     
     * Like {@link StencilEffect#STAMP}, shapes are drawn to the stencil buffer
     * instead of the screen. But unlike stamp, this effect is always additive.
     * It ignores path orientation, and does not support holes. This allows
     * the effect to implement a nonzero fill rule while using only the
     * upper half of the buffer.
     *
     * The primary application of this effect is to create stencils from
     * extruded paths so that overlapping sections are not drawn twice
     * (which has negative effects on alpha blending).
     */
    NONE_CARVE = 14,
    
    /**
     * Uses the upper buffer to limit each pixel to single update.
     *
     * This effect is a variation of {@link StencilEffect#NONE_CARVE} that
     * also draws as it writes to the upper stencil buffer. This guarantees
     * that each pixel is updated exactly once. This is used by extruded paths
     * so that overlapping sections are not drawn twice (which has negative
     * effects on alpha blending).
     */
    NONE_CLAMP = 15,
    
    /**
     * Restrict all drawing to the unified stencil region.
     *
     * This effect is the same as {@link StencilEffect#CLIP} in that it respects
     * the union of the two halves of the stencil buffer.
     */
    CLIP_JOIN = 16,
    
    /**
     * Restrict all drawing to the intersecting stencil region.
     *
     * This effect is the same as {@link StencilEffect#CLIP}, except that
     * it limits drawing to the intersection of the stencil regions in the
     * two halves of the stencil buffer. If a unified stencil region was
     * created by {@link StencilEffect#STAMP}, then the results of this
     * effect are unpredictable.
     */
    CLIP_MEET = 17,
    
    /**
     * Applies {@link StencilEffect#CLIP} using the lower stencil buffer only.
     *
     * As with {@link StencilEffect#CLIP}, this effect restricts drawing to
     * the stencil region. However, this effect only uses the stencil region
     * present in the lower stencil buffer.
     *
     * This effect is designed to be used with stencil regions created by
     * {@link StencilEffect#NONE_STAMP}. While it can be used by a stencil
     * region created by {@link StencilEffect#STAMP}, the lower stencil buffer
     * is ignored, and hence the results are unpredictable.
     */
    CLIP_NONE = 18,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CLIP} with an upper {@link StencilEffect#MASK}.
     *
     * This command restricts drawing to the stencil region in the lower
     * buffer while prohibiting any drawing to the stencil region in the
     * upper buffer. If this effect is applied to a unified stencil region
     * created by {@link StencilEffect#STAMP}, then the results are unpredictable.
     */
    CLIP_MASK = 19,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CLIP} with an upper {@link StencilEffect#FILL}.
     *
     * This command restricts drawing to the stencil region in the unified
     * stencil region of the two buffers. However, it only zeroes pixels in
     * the stencil region of the upper buffer; the lower buffer is untouched.
     * If this effect is applied to a unified stencil region created by
     * {@link StencilEffect#STAMP}, then the results are unpredictable.
     */
    CLIP_FILL = 20,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CLIP} with an upper {@link StencilEffect#WIPE}.
     *
     * As with {@link StencilEffect#WIPE}, this command does not do any drawing
     * on screen. Instead, it zeroes out the upper stencil buffer. However, it is
     * clipped by the stencil region in the lower buffer, so that it does not zero
     * out any pixel outside this region. Hence this is a way to erase the lower
     * buffer stencil region from the upper buffer stencil region.
     */
    CLIP_WIPE = 21,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CLIP} with an upper {@link StencilEffect#STAMP}.
     *
     * As with {@link StencilEffect#NONE_CLAMP}, this writes a shape to the upper
     * stencil buffer using an even-odd fill rule. This means that adding a shape
     * on top of existing shape has an erasing effect. However, it also restricts
     * its operation to the stencil region in the lower stencil buffer. Note
     * that if a pixel is clipped while drawing, it will not be added the
     * stencil region in the upper buffer.
     */
    CLIP_STAMP = 22,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CLIP} with an upper {@link StencilEffect#CARVE}.
     *
     * As with {@link StencilEffect#NONE_CARVE}, this writes an additive shape
     * to the upper stencil buffer. However, it also restricts its operation to
     * the stencil region in the lower stencil buffer. Note that if a pixel
     * is clipped while drawing, it will not be added the stencil region in
     * the upper buffer. Hence this is a way to copy the lower buffer stencil
     * region into the upper buffer.
     */
    CLIP_CARVE = 23,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CLIP} with an upper {@link StencilEffect#CLAMP}.
     *
     * As with {@link StencilEffect#NONE_CLAMP}, this draws a nonoverlapping
     * shape using the upper stencil buffer. However, it also restricts its
     * operation to the stencil region in the lower stencil buffer. Note that
     * if a pixel is clipped while drawing, it will not be added the stencil
     * region in the upper buffer.
     */
    CLIP_CLAMP = 24,
    
    /**
     * Prohibits all drawing to the unified stencil region.
     *
     * This effect is the same as {@link StencilEffect#MASK} in that it respects
     * the union of the two halves of the stencil buffer.
     */
    MASK_JOIN = 25,
    
    /**
     * Prohibits all drawing to the intersecting stencil region.
     *
     * This effect is the same as {@link StencilEffect#MASK}, except that
     * it limits drawing to the intersection of the stencil regions in the
     * two halves of the stencil buffer. If a unified stencil region was
     * created by {@link StencilEffect#STAMP}, then the results of this effect
     * are unpredictable.
     */
    MASK_MEET = 26,
    
    /**
     * Applies {@link StencilEffect#MASK} using the lower stencil buffer only.
     *
     * As with {@link StencilEffect#MASK}, this effect prohibits drawing to
     * the stencil region. However, this effect only uses the stencil region
     * present in the lower stencil buffer.
     *
     * This effect is designed to be used with stencil regions created by
     * {@link StencilEffect#STAMP_NONE}. While it can be used by a stencil
     * region created by {@link StencilEffect#STAMP}, the upper stencil buffer
     * is ignored, and hence the results are unpredictable.
     */
    MASK_NONE = 27,
    
    /**
     * Applies a lower buffer {@link StencilEffect#MASK} with an upper {@link StencilEffect#CLIP}.
     *
     * This command restricts drawing to the stencil region in the upper
     * buffer while prohibiting any drawing to the stencil region in the
     * lower buffer. If this effect is applied to a unified stencil region
     * created by {@link StencilEffect#STAMP}, then the results are unpredictable.
     */
    MASK_CLIP = 28,
    
    /**
     * Applies a lower buffer {@link StencilEffect#MASK} with an upper {@link StencilEffect#FILL}.
     *
     * This command restricts drawing to the stencil region in the upper
     * buffer while prohibiting any drawing to the stencil region in the
     * lower buffer. However, it only zeroes the stencil region in the
     * upper buffer; the lower buffer is untouched. In addition, it will
     * only zero those pixels that were drawn.
     *
     * If this effect is applied to a unified stencil region created by
     * {@link StencilEffect#STAMP}, then the results are unpredictable.
     */
    MASK_FILL = 29,
    
    /**
     * Applies a lower buffer {@link StencilEffect#MASK} with an upper {@link StencilEffect#WIPE}.
     *
     * As with {@link StencilEffect#WIPE}, this command does not do any drawing
     * on screen. Instead, it zeroes out the upper stencil buffer. However, it
     * is masked by the stencil region in the lower buffer, so that it does not
     * zero out any pixel inside this region.
     */
    MASK_WIPE = 30,
    
    /**
     * Applies a lower buffer {@link StencilEffect#MASK} with an upper {@link StencilEffect#STAMP}.
     *
     * As with {@link StencilEffect#NONE_STAMP}, this writes a shape to the
     * upper stencil buffer using an even-odd fill rule. This means that adding
     * a shape on top of existing shape has an erasing effect. However, it also
     * masks its operation by the stencil region in the lower stencil buffer. Note
     * that if a pixel is masked while drawing, it will not be added the
     * stencil region in the upper buffer.
     */
    MASK_STAMP = 31,
    
    /**
     * Applies a lower buffer {@link StencilEffect#MASK} with an upper {@link StencilEffect#CARVE}.
     *
     * As with {@link StencilEffect#NONE_CARVE}, this writes an additive shape
     * to the upper stencil buffer. However, it also prohibits any drawing to
     * the stencil region in the lower stencil buffer. Note that if a pixel is
     * masked while drawing, it will not be added the stencil region in
     * the upper buffer.
     */
    MASK_CARVE = 32,
    
    /**
     * Applies a lower buffer {@link StencilEffect#MASK} with an upper {@link StencilEffect#CLAMP}.
     *
     * As with {@link StencilEffect#NONE_CLAMP}, this draws a nonoverlapping
     * shape using the upper stencil buffer. However, it also prohibits any
     * drawing to the stencil region in the lower stencil buffer. Note that
     * if a pixel is masked while drawing, it will not be added the stencil
     * region in the upper buffer.
     */
    MASK_CLAMP = 33,
    
    /**
     * Restrict all drawing to the unified stencil region.
     *
     * This effect is the same as {@link StencilEffect#FILL} in that it respects
     * the union of the two halves of the stencil buffer.
     */
    FILL_JOIN = 34,
    
    /**
     * Restrict all drawing to the intersecting stencil region.
     *
     * This effect is the same as {@link StencilEffect#FILL}, except that it
     * limits drawing to the intersection of the stencil regions in the two
     * halves of the stencil buffer.
     *
     * When zeroing out pixels, this operation zeroes out both halves of
     * the stencil buffer. If a unified stencil region was created by
     * {@link StencilEffect#STAMP}, the results of this effect are unpredictable.
     */
    FILL_MEET = 35,
    
    /**
     * Applies {@link StencilEffect#FILL} using the lower stencil buffer only.
     *
     * As with {@link StencilEffect#FILL}, this effect restricts drawing to
     * the stencil region. However, this effect only uses the stencil region
     * present in the lower stencil buffer. It also only zeroes the stencil
     * region in this lower buffer.
     *
     * This effect is designed to be used with stencil regions created by
     * {@link StencilEffect#NONE_STAMP}. While it can be used by a stencil
     * region created by {@link StencilEffect#STAMP}, the lower stencil buffer
     * is ignored, and hence the results are unpredictable.
     */
    FILL_NONE = 36,
    
    /**
     * Applies a lower buffer {@link StencilEffect#FILL} with an upper {@link StencilEffect#MASK}.
     *
     * This command restricts drawing to the stencil region in the lower
     * buffer while prohibiting any drawing to the stencil region in the
     * upper buffer.
     *
     * When zeroing out the stencil region, this part of the effect is only
     * applied to the lower buffer. If this effect is applied to a unified
     * stencil region created by {@link StencilEffect#STAMP}, then the results
     * are unpredictable.
     */
    FILL_MASK = 37,
    
    /**
     * Applies a lower buffer {@link StencilEffect#FILL} with an upper {@link StencilEffect#CLIP}.
     *
     * This command restricts drawing to the stencil region in the unified
     * stencil region of the two buffers. However, it only zeroes pixels in
     * the stencil region of the lower buffer; the lower buffer is untouched.
     * If this effect is applied to a unified stencil region created by
     * {@link StencilEffect#STAMP}, then the results are unpredictable.
     */
    FILL_CLIP = 38,
    
    /**
     * Applies {@link StencilEffect#WIPE} using the lower stencil buffer only.
     *
     * As with {@link StencilEffect#WIPE}, this effect zeroes out the stencil
     * region, erasing parts of it. However, its effects are limited to the lower
     * stencil region.
     *
     * This effect is designed to be used with stencil regions created by
     * {@link StencilEffect#NONE_STAMP}. While it can be used by a stencil
     * region created by {@link StencilEffect#STAMP}, the lower stencil buffer
     * is ignored, and hence the results are unpredictable.
     */
    WIPE_NONE = 39,
    
    /**
     * Applies a lower buffer {@link StencilEffect#WIPE} with an upper {@link StencilEffect#MASK}.
     *
     * This command erases from the stencil region in the lower buffer.
     * However, it limits its erasing to locations that are not masked by
     * the stencil region in the upper buffer. If this effect is applied
     * to a unified stencil region created by {@link StencilEffect#STAMP},
     * the results are unpredictable.
     */
    WIPE_MASK = 40,
    
    /**
     * Applies a lower buffer {@link StencilEffect#WIPE} with an upper {@link StencilEffect#CLIP}.
     *
     * This command erases from the stencil region in the lower buffer.
     * However, it limits its erasing to locations that are contained in
     * the stencil region in the upper buffer. If this effect is applied
     * to a unified stencil region created by {@link StencilEffect#STAMP},
     * the results are unpredictable.
     */
    WIPE_CLIP = 41,
    
    /**
     * Adds a stencil region to the lower buffer
     *
     * This effect will not have any immediate visible effect on the screen
     * screen. Instead, it creates a stencil region for the effects such as
     * {@link StencilEffect#CLIP}, {@link StencilEffect#MASK}, and the like.
     *
     * Unlike {@link StencilEffect#STAMP}, the region created is limited to
     * the lower half of the stencil buffer. That is because the shapes are
     * drawn to the buffer with an even-odd fill rule (which does not require
     * the full stencil buffer to implement). This has the disadvantage
     * that stamps drawn on top of each other have an "erasing" effect.
     * However, it has the advantage that the this stamp supports a wider
     * array of effects than the simple stamp effect.
     */
    STAMP_NONE = 42,
    
    /**
     * Applies a lower buffer {@link StencilEffect#STAMP} with an upper {@link StencilEffect#CLIP}.
     *
     * As with {@link StencilEffect#STAMP_NONE}, this writes a shape to the
     * lower stencil buffer using an even-odd fill rule. This means that adding
     * a shape on top of existing shape has an erasing effect. However, it also
     * restricts its operation to the stencil region in the upper stencil buffer.
     * Note that if a pixel is clipped while drawing, it will not be added the
     * stencil region in the lower buffer.
     */
    STAMP_CLIP = 43,
    
    /**
     * Applies a lower buffer {@link StencilEffect#STAMP} with an upper {@link StencilEffect#MASK}.
     *
     * As with {@link StencilEffect#STAMP_NONE}, this writes a shape to the lower
     * stencil buffer using an even-odd fill rule. This means that adding a shape
     * on top of existing shape has an erasing effect. However, it also masks
     * its operation by the stencil region in the upper stencil buffer. Note
     * that if a pixel is masked while drawing, it will not be added the
     * stencil region in the lower buffer.
     */
    STAMP_MASK = 44,
    
    /**
     * Adds a stencil region to both the lower and the upper buffer
     *
     * This effect will not have any immediate visible effect on the screen
     * screen. Instead, it creates a stencil region for the effects such as
     * {@link StencilEffect#CLIP}, {@link StencilEffect#MASK}, and the like.
     *
     * Unlike {@link StencilEffect#STAMP}, the region is create twice and put in
     * both the upper and the lower stencil buffer. That is because the shapes
     * are drawn to the buffer with an even-odd fill rule (which does not require
     * the full stencil buffer to implement). This has the disadvantage that
     * stamps drawn on top of each other have an "erasing" effect. However, it
     * has the advantage that the this stamp supports a wider array of effects
     * than the simple stamp effect.
     *
     * The use of both buffers to provide a greater degree of flexibility.
     */
    STAMP_BOTH = 45,
    
    /**
     * Adds a stencil region to the lower buffer
     *
     * This effect is equivalent to {@link StencilEffect#CARVE}, since it only uses
     * half of the stencil buffer.
     */
    CARVE_NONE = 46,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CARVE} with an upper {@link StencilEffect#CLIP}.
     *
     * As with {@link StencilEffect#CARVE_NONE}, this writes an additive shape
     * to the lower stencil buffer. However, it also restricts its operation to
     * the stencil region in the upper stencil buffer. Note that if a pixel
     * is clipped while drawing, it will not be added the stencil region in
     * the lower buffer. Hence this is a way to copy the upper buffer stencil
     * region into the lower buffer.
     */
    CARVE_CLIP = 47,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CARVE} with an upper {@link StencilEffect#MASK}.
     *
     * As with {@link StencilEffect#CARVE_NONE}, this writes an additive shape
     * to the lower stencil buffer. However, it also prohibits any drawing to
     * the stencil region in the upper stencil buffer. Note that if a pixel is
     * masked while drawing, it will not be added the stencil region in
     * the lower buffer.
     */
    CARVE_MASK = 48,
    
    /**
     * Adds a stencil region to both the lower and upper buffer
     *
     * This effect is similar to {@link StencilEffect#CARVE}, except that it uses
     * both buffers. This is to give a wider degree of flexibility.
     */
    CARVE_BOTH = 49,
    
    /**
     * Uses the lower buffer to limit each pixel to single update.
     *
     * This effect is equivalent to {@link StencilEffect#CLAMP}, since it only uses
     * half of the stencil buffer.
     */
    CLAMP_NONE = 50,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CLAMP} with an upper {@link StencilEffect#CLIP}.
     *
     * As with {@link StencilEffect#CLAMP_NONE}, this draws a nonoverlapping
     * shape using the lower stencil buffer. However, it also restricts its
     * operation to the stencil region in the upper stencil buffer. Note that
     * if a pixel is clipped while drawing, it will not be added the stencil
     * region in the lower buffer.
     */
    CLAMP_CLIP = 51,
    
    /**
     * Applies a lower buffer {@link StencilEffect#CLAMP} with an upper {@link StencilEffect#MASK}.
     *
     * As with {@link StencilEffect#CLAMP_NONE}, this draws a nonoverlapping
     * shape using the lower stencil buffer. However, it also prohibits any
     * drawing to the stencil region in the upper stencil buffer. Note that
     * if a pixel is masked while drawing, it will not be added the stencil
     * region in the lower buffer.
     */
    CLAMP_MASK = 52
    
};

/**
 * Platform independent stencil functions.
 *
 * The functions in this apply a stencil affect to the given shader, wihout
 * using direct OpenGL calls. They are designed this way as part of our ongoing
 * transition to Vulkan.
 */
namespace stencil {
    /**
     * Clears the stencil buffer specified
     *
     * @param buffer    The stencil buffer (lower, upper, both)
     */
    void clearBuffer(GLenum buffer);

    /**
     * Configures the settings to apply the given effect.
     *
     * Note that the shader parameter is only relevant in Vulkan, as OpenGL stencil
     * operations are applied globally.
     *
     * @param effect    The stencil effect
     * @param shader    The shader to apply the stencil operations to.
     */
    void applyEffect(StencilEffect effect,
                     std::shared_ptr<cugl::graphics::Shader> shader=nullptr);
}

    }
}
#endif /* __CU_STENCIL_EFFECT_H__ */
