//
// Created by bcurtis on 04/06/18.
//

#include "skybox.hpp"

#include <iostream>
#include <cassert>

#include <osg/Geometry>
#include <osg/Depth>
#include <osg/TextureCubeMap>
#include <osg/TexEnv>
#include <osgDB/ReadFile>
#include <osgUtil/CullVisitor>

#include <cfloat>

#include <osg/Image>


/** Implementation of copyImage. */
template<typename T>
void copyDataImpl(const osg::Image* source,
                  const unsigned int x1, const unsigned int y1,
                  const unsigned int x2, const unsigned int y2,
                  osg::Image* destination,
                  const unsigned int xd = 0, const unsigned int yd = 0,
                  const bool clamp = false)
{
    if ((unsigned int)destination->s() >= xd + (x2 - x1) &&
        (unsigned int)destination->t() >= yd + (y2 - y1))
    {
        const unsigned int bpps =      source->getPixelSizeInBits() / (8 * sizeof(T));
        const unsigned int bppd = destination->getPixelSizeInBits() / (8 * sizeof(T));

        T* srcdata = (T*)source->data();
        T* dstdata = (T*)destination->data();

        for (unsigned int y = 0; y < y2 - y1; ++y)
        {
            for (unsigned int x = 0; x < x2 - x1; ++x)
            {
                T r = srcdata[(y + y1) * source->s() * bpps + (x + x1) * bpps + 0];
                T g = srcdata[(y + y1) * source->s() * bpps + (x + x1) * bpps + 1];
                T b = srcdata[(y + y1) * source->s() * bpps + (x + x1) * bpps + 2];

                if (clamp)
                {
                    r = osg::clampTo(r, (T)0, (T)1);
                    g = osg::clampTo(g, (T)0, (T)1);
                    b = osg::clampTo(b, (T)0, (T)1);
                }

                dstdata[(yd + y) * destination->s() * bppd + (xd + x) * bppd + 0] = r;
                dstdata[(yd + y) * destination->s() * bppd + (xd + x) * bppd + 1] = g;
                dstdata[(yd + y) * destination->s() * bppd + (xd + x) * bppd + 2] = b;
            }
        }
    }
    else
    {
        printf("DEBUG: %d %d %d %d\n", (unsigned int)destination->s(), (unsigned int)destination->t(), xd + (x2 - x1), yd + (y2 - y1));
        assert(false && "copyDataImpl: Incorrect image dimensions.");
    }
}

/** Copies a rectangle of corners (x1, y1), (x2, y2) from an image into
    another image starting at position (xd, yd). No scaling is done, the
    pixels are just copied, so the destination image must be at least
    (xd + (x2 - x1)) by (yd + (y2 - y1)) pixels. */
void copyData(const osg::Image* source,
              const unsigned int x1, const unsigned int y1,
              const unsigned int x2, const unsigned int y2,
              osg::Image* destination,
              const unsigned int xd, const unsigned int yd,
              const bool clamp)
{
    if (source->getDataType() == destination->getDataType())
    {
        if (source->getDataType() == GL_FLOAT)
        {
            copyDataImpl<float>(source, x1, y1, x2, y2,
                                destination, xd, yd, clamp);
        }
        else if (source->getDataType() == GL_UNSIGNED_BYTE)
        {
            copyDataImpl<unsigned char>(source, x1, y1, x2, y2,
                                        destination, xd, yd, clamp);
        }
        else
        {
            assert(false && "copyData not implemented for this data type");
        }
    }
    else
    {
        assert(false && "source and destination images must be of the same type.");
        return;
    }
}


/** Implementation of rotateImage. */
template<typename T>
osg::Image* rotateImageImpl(osg::Image* image)
{
    if (image->s() == image->t())
    {
        const unsigned int s = image->s();
        const unsigned int bpp = image->getPixelSizeInBits() / (8 * sizeof(T));

        osg::ref_ptr<osg::Image> destination  = new osg::Image;
        destination->allocateImage(s, s, 1,
                                   image->getPixelFormat(), image->getDataType(),
                                   image->getPacking());
        destination->setInternalTextureFormat(image->getInternalTextureFormat());

        T* srcdata = (T*)image->data();
        T* dstdata = (T*)destination->data();

        for (unsigned int y = 0; y < s; ++y)
        {
            for (unsigned int x = 0; x < s; ++x)
            {
                dstdata[y * s * bpp + x * bpp + 0] = srcdata[x * s * bpp + y * bpp + 0];
                dstdata[y * s * bpp + x * bpp + 1] = srcdata[x * s * bpp + y * bpp + 1];
                dstdata[y * s * bpp + x * bpp + 2] = srcdata[x * s * bpp + y * bpp + 2];
            }
        }

        return destination.release();
    }
    else
    {
        assert(false && "rotateImageImpl: Image must be square.");
        return 0;
    }
}

/** Rotates an osg::Image by 90 degrees. Returns a new osg::Image, be sure to
    store it in a ref_ptr so it will be freed correctly. */
osg::Image* rotateImage(osg::Image* image)
{
    if (image->getDataType() == GL_FLOAT)
    {
        return rotateImageImpl<float>(image);
    }
    else if (image->getDataType() == GL_UNSIGNED_BYTE)
    {
        return rotateImageImpl<unsigned char>(image);
    }
    else
    {
        assert(false && "rotateImage not implemented for this data type");
        return 0;
    }
}


template<typename T>
float getMaxValueImpl(const osg::Image* image)
{
    const unsigned int size = image->getImageSizeInBytes() / sizeof(T);
    T* data = (T*)image->data();

    T maxValue = FLT_MIN;
    for (unsigned int i = 0; i < size; i++)
    {
        if (data[i] > maxValue)
            maxValue = data[i];
    }

    return (T)maxValue;
}

float getMaxValue(osg::Image* image)
{
    if (image->getDataType() == GL_FLOAT)
    {
        return getMaxValueImpl<float>(image);
    }
    else if (image->getDataType() == GL_UNSIGNED_BYTE)
    {
        return 1.0f;
    }
    else
    {
        assert(false && "getMaxValue not implemented for this data type");
        return 0;
    }
}

osg::TextureCubeMap* loadVerticalCrossCubeMap(const std::string& filename)
{
    const osg::ref_ptr<osg::Image> cross = osgDB::readImageFile(filename);
    if (!cross.valid())
    {
        std::cout << "Image file " << filename << " could not be loaded." << std::endl;
        return 0;
    }

    const GLenum pixelFormat   = cross->getPixelFormat();
    const GLenum dataType      = cross->getDataType();
    const GLint internalFormat = cross->getInternalTextureFormat();
    unsigned int packing       = cross->getPacking();
    const unsigned int s = (unsigned int)cross->s();
    const unsigned int t = (unsigned int)cross->t();
    const unsigned int one_third_s = (unsigned int)(float(s) * 1/3);
    const unsigned int two_thirds_s = one_third_s * 2;
    const unsigned int one_quarter_t = (unsigned int)(float(t) * 1/4);
    const unsigned int one_half_t = one_quarter_t * 2;
    const unsigned int three_quarters_t = one_quarter_t * 3;
    printf("s:%d, t%d, one_third_s:%d, two_thirds_s:%d, one_quarter_t:%d, one_half_t:%d three_quarters_t:%d\n",
    s, t, one_third_s, two_thirds_s, one_quarter_t, one_half_t, three_quarters_t);


    osg::ref_ptr<osg::TextureCubeMap> cubeMap = new osg::TextureCubeMap;
    cubeMap->setTextureSize(one_third_s, one_quarter_t);


    osg::ref_ptr<osg::Image> zplus  = new osg::Image;
    zplus->allocateImage(one_third_s, one_quarter_t, 1,
                         pixelFormat, dataType, packing);
    zplus->setInternalTextureFormat(internalFormat);
    copyData(cross.get(), one_third_s, one_quarter_t, two_thirds_s, one_half_t, zplus.get());
    cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Z, zplus.get());

    osg::ref_ptr<osg::Image> zminus = new osg::Image;
    zminus->allocateImage(one_third_s, one_quarter_t, 1,
                          pixelFormat, dataType, packing);
    zminus->setInternalTextureFormat(internalFormat);
    copyData(cross.get(), one_third_s, three_quarters_t, two_thirds_s, t, zminus.get());
    zminus->flipVertical();
    zminus->flipHorizontal();
    cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Z, zminus.get());

    osg::ref_ptr<osg::Image> yplus  = new osg::Image;
    yplus->allocateImage(one_third_s, one_quarter_t, 1,
                         pixelFormat, dataType, packing);
    yplus->setInternalTextureFormat(internalFormat);
    copyData(cross.get(), one_third_s, 0, two_thirds_s, one_quarter_t, yplus.get());
    cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Y, yplus.get());

    osg::ref_ptr<osg::Image> yminus = new osg::Image;
    yminus->allocateImage(one_third_s, one_quarter_t, 1,
                          pixelFormat, dataType, packing);
    yminus->setInternalTextureFormat(internalFormat);
    copyData(cross.get(), one_third_s, one_half_t, two_thirds_s, three_quarters_t, yminus.get());
    cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Y, yminus.get());

    osg::ref_ptr<osg::Image> xplus  = new osg::Image;
    xplus->allocateImage(one_third_s, one_quarter_t, 1,
                         pixelFormat, dataType, packing);
    xplus->setInternalTextureFormat(internalFormat);
    copyData(cross.get(), two_thirds_s, one_half_t, s, three_quarters_t, xplus.get());
    xplus = rotateImage(xplus.get());
    xplus->flipVertical();
    cubeMap->setImage(osg::TextureCubeMap::POSITIVE_X, xplus.get());

    osg::ref_ptr<osg::Image> xminus = new osg::Image;
    xminus->allocateImage(one_third_s, one_quarter_t, 1,
                          pixelFormat, dataType, packing);
    xminus->setInternalTextureFormat(internalFormat);
    copyData(cross.get(), 0, one_half_t, one_third_s, three_quarters_t, xminus.get(), 0, 0, true);
    xminus = rotateImage(xminus.get());
    xminus->flipHorizontal();
    cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_X, xminus.get());

    return cubeMap.release();
}

class SkyboxTransform : public osg::Transform
{
public:
    // Get the transformation matrix which moves from local coords to world coords.
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,
                                           osg::NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.preMult(osg::Matrix::translate(eyePointLocal));
        }

        return true;
    }

    // Get the transformation matrix which moves from world coords to local coords.
    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,
                                           osg::NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.postMult(osg::Matrix::translate(-eyePointLocal));
        }

        return true;
    }
};

osg::Node* createSkyBoxCubeMap(const std::string& cubemap, osg::StateSet* stateset)
{
    float radius = 100.0f;

    if (!stateset)
        stateset = new osg::StateSet;

    // Set texture mode to REPLACE
    osg::TexEnv* te = new osg::TexEnv;
    te->setMode(osg::TexEnv::REPLACE);
    stateset->setTextureAttributeAndModes(0, te, osg::StateAttribute::ON);

    // Turn off lighting and cull face
    stateset->setMode(GL_LIGHTING,  osg::StateAttribute::OFF);
    stateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

    // Basic principle of skyboxes: render it last, and with a depth func
    // that only sets pixels at the far plane. That way, if the sky is not
    // visible at all for example, the sky won't be drawn (and possibly
    // fragment shaders will not be called) whereas rendering it first will
    // cause all pixels to be written, then overwritten by objects, and
    // possibly fragment shaders will have been called for nothing.

    // Clear the depth to the far plane.
    osg::Depth* depth = new osg::Depth(osg::Depth::LEQUAL, 1.0, 1.0);
    stateset->setAttributeAndModes(depth, osg::StateAttribute::ON);

    // Make sure it is drawn last
    stateset->setRenderBinDetails(1000, "RenderBin");

    // Create a drawable for the skybox
    osg::Geometry* drawable = new osg::Geometry;


    // Create vertices for box
    osg::Vec3Array *verts = new osg::Vec3Array;
    verts->push_back(osg::Vec3(-radius, -radius,  radius));
    verts->push_back(osg::Vec3(-radius,  radius,  radius));
    verts->push_back(osg::Vec3( radius,  radius,  radius));
    verts->push_back(osg::Vec3( radius, -radius,  radius));
    verts->push_back(osg::Vec3(-radius, -radius, -radius));
    verts->push_back(osg::Vec3(-radius,  radius, -radius));
    verts->push_back(osg::Vec3( radius,  radius, -radius));
    verts->push_back(osg::Vec3( radius, -radius, -radius));
    drawable->setVertexArray(verts);

    // Create texture coordinates for cubemaps
    osg::Vec3Array *coords = new osg::Vec3Array;
    coords->push_back(osg::Vec3(-1,  1, -1));
    coords->push_back(osg::Vec3(-1, -1, -1));
    coords->push_back(osg::Vec3( 1, -1, -1));
    coords->push_back(osg::Vec3( 1,  1, -1));
    coords->push_back(osg::Vec3(-1,  1,  1));
    coords->push_back(osg::Vec3(-1, -1,  1));
    coords->push_back(osg::Vec3( 1, -1,  1));
    coords->push_back(osg::Vec3( 1,  1,  1));

    drawable->setTexCoordArray(0,coords);

    // Create an index array for the box
    osg::ref_ptr<osg::UIntArray> indices = new osg::UIntArray;

    // Front face
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(2);
    indices->push_back(3);

    // Back face
    indices->push_back(4);
    indices->push_back(5);
    indices->push_back(6);
    indices->push_back(7);

    // Right face
    indices->push_back(6);
    indices->push_back(7);
    indices->push_back(3);
    indices->push_back(2);

    // Left face
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(5);
    indices->push_back(4);

    // Top face
    indices->push_back(1);
    indices->push_back(2);
    indices->push_back(6);
    indices->push_back(5);

    // Bottom face
    indices->push_back(0);
    indices->push_back(3);
    indices->push_back(7);
    indices->push_back(4);

    drawable->addPrimitiveSet(
            new osg::DrawElementsUInt(GL_QUADS,
                                      indices->size(), &(indices->front())));


    // Create a geode for the skybox
    osg::Geode* geode = new osg::Geode;
    geode->setName("Skybox");

    // Disable culling
    geode->setCullingActive(false);

    // Set the stateset
    geode->setStateSet(stateset);

    // Add the skybox
    geode->addDrawable(drawable);


    // Load the texture
    osg::ref_ptr<osg::TextureCubeMap> texture = loadVerticalCrossCubeMap(cubemap);
    if (texture.valid())
    {
        texture->setResizeNonPowerOfTwoHint(false);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
        stateset->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);
    }

    // Create a transform and set it to absolute so it does not influence
    // the viewer's bounds computation (for initial camera settings).
    osg::Transform* transform = new SkyboxTransform;
    transform->setCullingActive(false);
    transform->addChild(geode);
    transform->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    transform->setName("SkyboxTransform");

    return transform;
}
