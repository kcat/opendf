
#include "meshloader.hpp"

#include <algorithm>

#include <osg/Geode>
#include <osg/Geometry>

#include "components/vfs/manager.hpp"


namespace
{

const uint32_t VER_2_5 = ('v' | ('2'<<8) | ('.'<<16) | ('5'<<24));
//const uint32_t VER_2_6 = ('v' | ('2'<<8) | ('.'<<16) | ('6'<<24));
//const uint32_t VER_2_7 = ('v' | ('2'<<8) | ('.'<<16) | ('7'<<24));

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
    void load(std::istream &stream)
    {
        mVersion = VFS::read_le32(stream);

        mPointCount = VFS::read_le32(stream);
        mPlaneCount = VFS::read_le32(stream);
        mRadius = VFS::read_le32(stream);

        mNullValue1[0] = VFS::read_le32(stream);
        mNullValue1[1] = VFS::read_le32(stream);

        mPlaneDataOffset = VFS::read_le32(stream);
        mObjectDataOffset = VFS::read_le32(stream);
        mObjectDataCount = VFS::read_le32(stream);

        mUnknown1 = VFS::read_le32(stream);

        mNullValue2[0] = VFS::read_le32(stream);
        mNullValue2[1] = VFS::read_le32(stream);

        mPointListOffset = VFS::read_le32(stream);
        mNormalListOffset = VFS::read_le32(stream);

        mUnknown2 = VFS::read_le32(stream);

        mPlaneListOffset = VFS::read_le32(stream);
    }

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
    void load(std::istream &stream)
    {
        mX = VFS::read_le32(stream);
        mY = VFS::read_le32(stream);
        mZ = VFS::read_le32(stream);
    }

    int32_t x() const { return mX; }
    int32_t y() const { return mY; }
    int32_t z() const { return mZ; }
};

class MdlPlanePoint {
    uint32_t mIndex;
    int32_t mU;
    int32_t mV;

public:
    void load(std::istream &stream, uint32_t offset_scale)
    {
        mIndex = VFS::read_le32(stream) / offset_scale;
        mU = ((VFS::read_le16(stream)&0x0fff) ^ 0x0800) - 0x0800;
        mV = ((VFS::read_le16(stream)&0x0fff) ^ 0x0800) - 0x0800;
    }

    int32_t getIndex() const { return mIndex; }
    int32_t& u() { return mU; }
    const int32_t& u() const { return mU; }
    int32_t& v() { return mV; }
    const int32_t& v() const { return mV; }
};

class MdlPlane {
    uint8_t mPointCount;
    uint8_t mUnknown1;
    uint16_t mTextureId;
    uint32_t mUnknown2;

    std::vector<MdlPlanePoint> mPoints;
    MdlPoint mNormal;

public:
    void load(std::istream &stream, uint32_t offset_scale)
    {
        mPointCount = stream.get();
        mUnknown1 = stream.get();
        mTextureId = VFS::read_le16(stream);
        mUnknown2 = VFS::read_le32(stream);

        mPoints.resize(mPointCount);
        for(MdlPlanePoint &pt : mPoints)
            pt.load(stream, offset_scale);
    }

    void loadNormal(std::istream &stream)
    {
        mNormal.load(stream);
    }

    void fixUVs(const std::vector<MdlPoint> &points, uint32_t width, uint32_t height)
    {
        /* Convert delta coords to absolute. */
        for(size_t i = 1;i < mPoints.size() && i < 3;++i)
        {
            mPoints[i].u() += mPoints[i-1].u();
            mPoints[i].v() += mPoints[i-1].v();
        }

        /* Normalize the coords into 16.16 fixed point. */
        for(size_t i = 0;i < mPoints.size() && i < 3;++i)
        {
            mPoints[i].u() = (mPoints[i].u()<<16) / width;
            mPoints[i].v() = (mPoints[i].v()<<16) / height;
        }

        /* Daggerfall does not use the provided UV coords for the 4th point and beyond, so we can't rely on them.
         * Check if they need to be calculated. */
        if(mPoints.size() >= 4)
        {
            /* Basing information here from http://uesp.net/wiki/Daggerfall:UV_texture_coordinates
             *
             * "Experiments show the Daggerfall rendering engine only uses the UV coordinates for the first three
             * PlanePoint records."
             *
             * This means, using the first three UV coordinates and point positions, Daggerfall must be able to work
             * out the U and V stepping values that it applies over the whole plane. This affords us a "simple"
             * solution:
             *
             * Given the same 3 points Daggerfall uses to calculate U and V stepping, we can find the tangent (T)
             * and binormal (B) vectors for the plane, which can then be used to work out UV coordinates for any
             * point on the plane.
             */
            osg::Vec3 p0(points[mPoints[0].getIndex()].x(), points[mPoints[0].getIndex()].y(), points[mPoints[0].getIndex()].z());
            osg::Vec3 p1(points[mPoints[1].getIndex()].x(), points[mPoints[1].getIndex()].y(), points[mPoints[1].getIndex()].z());
            osg::Vec3 p2(points[mPoints[2].getIndex()].x(), points[mPoints[2].getIndex()].y(), points[mPoints[2].getIndex()].z());
            osg::Vec2 uv0(mPoints[0].u(), mPoints[0].v());
            osg::Vec2 uv1(mPoints[1].u(), mPoints[1].v());
            osg::Vec2 uv2(mPoints[2].u(), mPoints[2].v());

            // Let P = Edge 1
            osg::Vec3 P = p1 - p0;
            // Let Q = Edge 2
            osg::Vec3 Q = p2 - p0;

            // Get UV deltas for the above edges
            float s1 = uv1.x() - uv0.x();
            float t1 = uv1.y() - uv0.y();
            float s2 = uv2.x() - uv0.x();
            float t2 = uv2.y() - uv0.y();

            // We need to solve for T and B:
            // P = s1*T + t1*B
            // Q = s2*T + t2*B

            // This is a linear system with six unknowns and six equations, for TxTyTz BxByBz
            // [px,py,pz] = [s1,t1] * [Tx,Ty,Tz]
            //  qx,qy,qz     s2,t2     Bx,By,Bz

            // Multiplying both sides by the inverse of the s,t matrix gives
            // [Tx,Ty,Tz] = 1/(s1t2-s2t1) * [ t2,-t1] * [px,py,pz]
            //  Bx,By,Bz                     -s2, s1     qx,qy,qz

            // Solve this to get the unormalized T and B vectors.

            float scale = 1.0f / (s1*t2 - s2*t1);
            osg::Vec3 tangent = (P*t2 - Q*t1) * scale; tangent.normalize();
            osg::Vec3 binormal = (Q*s1 - P*s2) * scale; binormal.normalize();

            // Find the U and V scales. Without this, we would have to assume 1 world unit = 1 UV unit.
            float uscale = ((fabs(tangent * P) > fabs(tangent * Q)) ?
                            (s1/(tangent*P)) : (s2/(tangent*Q)));
            float vscale = ((fabs(binormal * P) > fabs(binormal * Q)) ?
                            (t1/(binormal*P)) : (t2/(binormal*Q)));

            // Now that we have the T and B vectors, we can get the missing UV coordinates,
            for(size_t i = 3;i < mPoints.size();++i)
            {
                osg::Vec3 p(points[mPoints[i].getIndex()].x() - p0.x(),
                            points[mPoints[i].getIndex()].y() - p0.y(),
                            points[mPoints[i].getIndex()].z() - p0.z());
                mPoints[i].u() = (tangent*p)*uscale + uv0.x();
                mPoints[i].v() = (binormal*p)*vscale + uv0.y();
            }
        }
    }

    const std::vector<MdlPlanePoint> &getPoints() const { return mPoints; }
    uint16_t getTextureId() const { return mTextureId; }
    const MdlPoint &getNormal() const { return mNormal; }
};

}


namespace DFOSG
{

MeshLoader::MeshLoader()
{
}

osg::ref_ptr<osg::Node> MeshLoader::load(size_t id)
{
    /* Not sure if this cache is a good idea since it shares the whole model
     * tree. OSG can parent the same sub-tree to multiple points, which should
     * be okay as long as the individual sub-trees don't need changing.
     */
    auto iter = mModelCache.find(id);
    if(iter != mModelCache.end())
    {
        osg::ref_ptr<osg::Node> node;
        if(iter->second.lock(node))
            return node;
    }

    VFS::IStreamPtr stream = VFS::Manager::get().openArchId(id);
    if(!stream) throw std::runtime_error("Failed to open ARCH3D ID "+std::to_string(id));

    MdlHeader header;
    std::vector<MdlPoint> points;
    std::vector<MdlPlane> planes;

    header.load(*stream);

    points.resize(header.getPointCount());
    planes.resize(header.getPlaneCount());

    // points
    if(!stream->seekg(header.getPointListOffset()))
        throw std::runtime_error("Failed to seek to point list for ARCH3D ID "+std::to_string(id));

    for(MdlPoint &pt : points)
        pt.load(*stream);

    // planes
    if(!stream->seekg(header.getPlaneListOffset()))
        throw std::runtime_error("Failed to seek to plane list for ARCH3D ID "+std::to_string(id));

    uint32_t offset_scale = (header.getVersion() != VER_2_5) ? (4*3) : 4;
    for(MdlPlane &plane : planes)
        plane.load(*stream, offset_scale);

    // normals
    if(!stream->seekg(header.getNormalListOffset()))
        throw std::runtime_error("Failed to seek to normal list for ARCH3D ID "+std::to_string(id));

    for(MdlPlane &plane : planes)
        plane.loadNormal(*stream);

    // Sort planes to combine textures (for more efficient geometry)
    std::sort(planes.begin(), planes.end(),
        [](const MdlPlane &lhs, const MdlPlane &rhs)
        {
            return lhs.getTextureId() < rhs.getTextureId();
        }
    );

    osg::ref_ptr<osg::Geode> geode(new osg::Geode());

    for(auto iter = planes.begin();iter != planes.end();)
    {
        osg::ref_ptr<osg::Vec3Array> vtxs(new osg::Vec3Array());
        osg::ref_ptr<osg::Vec3Array> nrms(new osg::Vec3Array());
        osg::ref_ptr<osg::Vec2Array> texcrds(new osg::Vec2Array());
        osg::ref_ptr<osg::Vec4ubArray> colors(new osg::Vec4ubArray());
        osg::ref_ptr<osg::DrawElementsUShort> idxs(new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES));
        uint16_t texid = iter->getTextureId();

        do {
            // TODO: Fixup plane UVs according to the texture's size.
            iter->fixUVs(points, 1, 1);

            const std::vector<MdlPlanePoint> &pts = iter->getPoints();
            size_t last_total = vtxs->size();

            vtxs->resize(last_total + pts.size());
            nrms->resize(last_total + pts.size());
            texcrds->resize(last_total + pts.size());
            colors->resize(last_total + pts.size());
            idxs->resize((last_total + pts.size() - 2) * 3);

            size_t j = last_total;
            for(const MdlPlanePoint &pt : pts)
            {
                uint32_t vidx = pt.getIndex();

                (*vtxs)[j].x() = points[vidx].x() / 256.0f;
                (*vtxs)[j].y() = points[vidx].y() / 256.0f;
                (*vtxs)[j].z() = points[vidx].z() / 256.0f;

                (*nrms)[j].x() = iter->getNormal().x() / 256.0f;
                (*nrms)[j].y() = iter->getNormal().y() / 256.0f;
                (*nrms)[j].z() = iter->getNormal().z() / 256.0f;

                (*texcrds)[j].x() = pt.u() / 65536.0f;
                (*texcrds)[j].y() = pt.v() / 65536.0f;

                (*colors)[j] = osg::Vec4ub(255, 255, 255, 255);

                if(j >= last_total+2)
                {
                    (*idxs)[(j-2)*3 + 0] = last_total;
                    (*idxs)[(j-2)*3 + 1] = j-1;
                    (*idxs)[(j-2)*3 + 2] = j;
                }

                ++j;
            }
        } while(++iter != planes.end() && iter->getTextureId() == texid);

        osg::ref_ptr<osg::VertexBufferObject> vbo(new osg::VertexBufferObject());
        vtxs->setVertexBufferObject(vbo);
        nrms->setVertexBufferObject(vbo);
        texcrds->setVertexBufferObject(vbo);
        colors->setVertexBufferObject(vbo);
        colors->setNormalize(true);

        osg::ref_ptr<osg::ElementBufferObject> ebo(new osg::ElementBufferObject());
        idxs->setElementBufferObject(ebo);

        osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry);
        geometry->setVertexArray(vtxs);
        geometry->setNormalArray(nrms, osg::Array::BIND_PER_VERTEX);
        geometry->setTexCoordArray(0, texcrds, osg::Array::BIND_PER_VERTEX);
        geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(true);

        geometry->addPrimitiveSet(idxs);

        geode->addDrawable(geometry);
    }

    mModelCache[id] = osg::ref_ptr<osg::Node>(geode);
    return geode;
}

} // namespace DFOSG
