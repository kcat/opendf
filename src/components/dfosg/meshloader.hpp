#ifndef COMPONENTS_DFOSG_MESHLOADER_HPP
#define COMPONENTS_DFOSG_MESHLOADER_HPP

#include <vector>
#include <map>
#include <cstdint>
#include <istream>
#include <cstddef>


namespace osg
{
    class Matrixf;
    class Node;
}

namespace DFOSG
{

class MdlHeader {
    uint32_t mVersion; // This is a fourcc

    uint32_t mPointCount;
    uint32_t mPlaneCount;
    uint32_t mRadius;

    uint32_t mNullValue1[2];

    uint32_t mPlaneDataOffset;
    uint32_t mObjectDataOffset;
    uint32_t mObjectDataCount;

    uint32_t mUnknown1;

    uint32_t mNullValue2[2];

    uint32_t mPointListOffset;
    uint32_t mNormalListOffset;

    uint32_t mUnknown2;

    uint32_t mPlaneListOffset;

public:
    void load(std::istream &stream);

    uint32_t getVersion() const { return mVersion; }

    uint32_t getPointCount() const { return mPointCount; }
    uint32_t getPointListOffset() const { return mPointListOffset; }
    uint32_t getNormalListOffset() const { return mNormalListOffset; }

    uint32_t getPlaneCount() const { return mPlaneCount; }
    uint32_t getPlaneListOffset() const { return mPlaneListOffset; }
};

class MdlPoint {
    int32_t mX, mY, mZ;

public:
    void load(std::istream &stream);
    void set(int32_t x, int32_t y, int32_t z)
    {
        mX = x;
        mY = y;
        mZ = z;
    }

    int32_t x() const { return mX; }
    int32_t y() const { return mY; }
    int32_t z() const { return mZ; }
};

class MdlPlanePoint {
    uint32_t mIndex;
    float mU;
    float mV;

public:
    void load(std::istream &stream, uint32_t offset_scale);

    int32_t getIndex() const { return mIndex; }
    float& u() { return mU; }
    const float& u() const { return mU; }
    float& v() { return mV; }
    const float& v() const { return mV; }
};

class MdlPlane {
    uint8_t mPointCount;
    uint8_t mUnknown1;
    uint16_t mTextureId;
    uint32_t mUnknown2;

    std::vector<MdlPlanePoint> mPoints;
    MdlPoint mNormal;

    // Calculated on load
    MdlPoint mBinormal;

public:
    void load(std::istream &stream, uint32_t offset_scale);

    void loadNormal(std::istream &stream);

    void fixUVs(const std::vector<MdlPoint> &points);

    const std::vector<MdlPlanePoint> &getPoints() const { return mPoints; }
    uint16_t getTextureId() const { return mTextureId; }
    const MdlPoint &getNormal() const { return mNormal; }
    const MdlPoint &getBinormal() const { return mBinormal; }
};

class Mesh {
    MdlHeader mHeader;
    std::vector<MdlPoint> mPoints;
    std::vector<MdlPlane> mPlanes;

public:
    void load(std::istream &stream);

    const MdlHeader &getHeader() const { return mHeader; }
    const std::vector<MdlPoint> &getPoints() const { return mPoints; }
    const std::vector<MdlPlane> &getPlanes() const { return mPlanes; }
};


class MeshLoader {
    static MeshLoader sLoader;

    MeshLoader(const MeshLoader&) = delete;
    MeshLoader& operator=(const MeshLoader&) = delete;

    MeshLoader();

public:
    /* Loads a mesh by the given index (for ARCH3D.BSA), and returns a root
     * node for the object. */
    Mesh *load(size_t id);

    static MeshLoader &get()
    {
        return sLoader;
    }
};

} // namespace DFOSG

#endif /* COMPONENTS_DFOSG_MESHLOADER_HPP */
