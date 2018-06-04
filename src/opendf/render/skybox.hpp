//
// Created by bcurtis on 04/06/18.
//

#ifndef OPENDF_SKYBOX_HPP
#define OPENDF_SKYBOX_HPP

#include <string>
#include <osg/Image>
#include <osg/Node>

osg::Node* createSkyBoxCubeMap(const std::string& cubemap, osg::StateSet* stateset = 0);

/** Copies a rectangle of corners (x1, y1), (x2, y2) from an image into
    another image starting at position (xd, yd). No scaling is done, the
    pixels are just copied, so the destination image must be at least
    (xd + (x2 - x1)) by (yd + (y2 - y1)) pixels. */
void copyData(const osg::Image* source,
              const unsigned int x1, const unsigned int y1,
              const unsigned int x2, const unsigned int y2,
              osg::Image* destination,
              const unsigned int xd = 0, const unsigned int yd = 0,
              const bool clamp = false);

/** Rotates an osg::Image by 90 degrees. Returns a new osg::Image, be sure to
    store it in a ref_ptr so it will be freed correctly. */
osg::Image* rotateImage(osg::Image* image);

/** Returns the maximum value in the image for tone mapping purposes. Only
    really makes sense for HDR images, and as such if the image's pixel
    format is not float it will always return 1.0. */
float getMaxValue(osg::Image* image);


#endif //OPENDF_SKYBOX_HPP
