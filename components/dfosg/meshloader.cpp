
#include "meshloader.hpp"

#include <algorithm>

#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/AlphaFunc>

#include "components/vfs/manager.hpp"
#include "components/resource/texturemanager.hpp"


namespace
{

const uint32_t VER_2_5 = ('v' | ('2'<<8) | ('.'<<16) | ('5'<<24));
//const uint32_t VER_2_6 = ('v' | ('2'<<8) | ('.'<<16) | ('6'<<24));
//const uint32_t VER_2_7 = ('v' | ('2'<<8) | ('.'<<16) | ('7'<<24));

}


namespace DFOSG
{

void MdlHeader::load(std::istream& stream)
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


void MdlPoint::load(std::istream& stream)
{
    mX = VFS::read_le32(stream);
    mY = VFS::read_le32(stream);
    mZ = VFS::read_le32(stream);
}


void MdlPlanePoint::load(std::istream& stream, uint32_t offset_scale)
{
    mIndex = VFS::read_le32(stream) / offset_scale;
    mU = (int16_t)VFS::read_le16(stream) / 16.0f;
    mV = (int16_t)VFS::read_le16(stream) / 16.0f;
}


void MdlPlane::load(std::istream& stream, uint32_t offset_scale)
{
    mPointCount = stream.get();
    mUnknown1 = stream.get();
    mTextureId = VFS::read_le16(stream);
    mUnknown2 = VFS::read_le32(stream);

    mPoints.resize(mPointCount);
    for(MdlPlanePoint &pt : mPoints)
        pt.load(stream, offset_scale);
}

void MdlPlane::loadNormal(std::istream& stream)
{
    mNormal.load(stream);
}

void MdlPlane::fixUVs(const std::vector<MdlPoint> &points)
{
    /* Convert delta coords to absolute. */
    for(size_t i = 1;i < mPoints.size() && i < 3;++i)
    {
        mPoints[i].u() += mPoints[i-1].u();
        mPoints[i].v() += mPoints[i-1].v();
    }

    /* Daggerfall does not use the provided UV coords for the 4th point and
     * beyond, so we can't rely on them. Check if they need to be calculated.
     */
    if(mPoints.size() >= 4)
    {
        /* Basing information here from http://uesp.net/wiki/Daggerfall:UV_texture_coordinates
         *
         * "Experiments show the Daggerfall rendering engine only uses the UV
         * coordinates for the first three PlanePoint records."
         *
         * This means with the first three UV coordinates and point positions,
         * Daggerfall must be able to work out the U and V stepping values that
         * it applies over the whole plane. This gives us a "simple" solution:
         *
         * Given the same 3 points Daggerfall uses to calculate U and V
         * stepping, we can find the tangent (T) and binormal (B) vectors for
         * the plane, which can then be used to work out UV coordinates for any
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
        osg::Vec3 tangent = (P*t2 - Q*t1) * scale;
        osg::Vec3 binormal = (Q*s1 - P*s2) * scale;

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


void Mesh::load(std::istream &stream)
{
    mHeader.load(stream);

    mPoints.resize(mHeader.getPointCount());
    mPlanes.resize(mHeader.getPlaneCount());

    // points
    if(!stream.seekg(mHeader.getPointListOffset()))
        throw std::runtime_error("Failed to seek to point list");

    for(MdlPoint &pt : mPoints)
        pt.load(stream);

    // planes
    if(!stream.seekg(mHeader.getPlaneListOffset()))
        throw std::runtime_error("Failed to seek to plane list");

    uint32_t offset_scale = (mHeader.getVersion() != VER_2_5) ? (4*3) : 4;
    for(MdlPlane &plane : mPlanes)
        plane.load(stream, offset_scale);

    // normals
    if(!stream.seekg(mHeader.getNormalListOffset()))
        throw std::runtime_error("Failed to seek to normal list");

    for(MdlPlane &plane : mPlanes)
        plane.loadNormal(stream);

    // Fix UV coords, converting from delta to absolute values and generate the
    // missing coords
    for(MdlPlane &plane : mPlanes)
        plane.fixUVs(mPoints);

    // Sort planes to combine textures (for more efficient geometry)
    std::sort(mPlanes.begin(), mPlanes.end(),
        [](const MdlPlane &lhs, const MdlPlane &rhs)
        {
            return lhs.getTextureId() < rhs.getTextureId();
        }
    );
}


MeshLoader MeshLoader::sLoader;

MeshLoader::MeshLoader()
{
}


Mesh *MeshLoader::load(size_t id)
{
    VFS::IStreamPtr stream = VFS::Manager::get().openArchId(id);
    if(!stream) throw std::runtime_error("Failed to open ARCH3D ID "+std::to_string(id));

    std::unique_ptr<Mesh> mesh(new Mesh());
    mesh->load(*stream);

    return mesh.release();
}

} // namespace DFOSG
