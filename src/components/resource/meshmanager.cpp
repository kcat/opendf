
#include "meshmanager.hpp"

#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/AlphaFunc>
#include <osgDB/ReadFile>

#include "components/dfosg/meshloader.hpp"

#include "texturemanager.hpp"


namespace Resource
{

MeshManager MeshManager::sManager;

MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{
}


void MeshManager::initialize()
{
}

void MeshManager::deinitialize()
{
    mStateSetCache.clear();
    mTerrainCache.clear();
    mFlatCache.clear();
    mModelCache.clear();
    mTerrainProgram = nullptr;
    mFlatProgram = nullptr;
    mModelProgram = nullptr;
}


osg::ref_ptr<osg::Node> MeshManager::get(size_t idx)
{
    /* Not sure if this cache is a good idea since it shares the whole model
     * tree. OSG can parent the same sub-tree to multiple points, which should
     * be okay as long as the individual sub-trees don't need changing.
     */
    auto iter = mModelCache.find(idx);
    if(iter != mModelCache.end())
    {
        osg::ref_ptr<osg::Node> node;
        if(iter->second.lock(node))
            return node;
    }

    if(!mModelProgram)
    {
        mModelProgram = new osg::Program();
        mModelProgram->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "shaders/object.vert"));
        mModelProgram->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/object.frag"));
    }

    DFOSG::Mesh *mesh = DFOSG::MeshLoader::get().load(idx);

    osg::ref_ptr<osg::Geode> geode(new osg::Geode());
    for(auto iter = mesh->getPlanes().begin();iter != mesh->getPlanes().end();)
    {
        osg::ref_ptr<osg::Vec3Array> vtxs(new osg::Vec3Array());
        osg::ref_ptr<osg::Vec3Array> nrms(new osg::Vec3Array());
        osg::ref_ptr<osg::Vec3Array> binrms(new osg::Vec3Array());
        osg::ref_ptr<osg::Vec2Array> texcrds(new osg::Vec2Array());
        osg::ref_ptr<osg::Vec4ubArray> colors(new osg::Vec4ubArray());
        osg::ref_ptr<osg::DrawElementsUShort> idxs(new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES));
        uint16_t texid = iter->getTextureId();

        osg::ref_ptr<osg::Texture> tex = TextureManager::get().getTexture(texid);
        float width = tex->getTextureWidth();
        float height = tex->getTextureHeight();

        do {
            const std::vector<DFOSG::MdlPlanePoint> &pts = iter->getPoints();
            size_t last_total = vtxs->size();

            vtxs->resize(last_total + pts.size());
            nrms->resize(last_total + pts.size());
            binrms->resize(last_total + pts.size());
            texcrds->resize(last_total + pts.size());
            colors->resize(last_total + pts.size());
            idxs->resize((last_total + pts.size() - 2) * 3);

            size_t j = last_total;
            for(const DFOSG::MdlPlanePoint &pt : pts)
            {
                uint32_t vidx = pt.getIndex();

                (*vtxs)[j].x() = mesh->getPoints()[vidx].x() / 256.0f;
                (*vtxs)[j].y() = mesh->getPoints()[vidx].y() / 256.0f;
                (*vtxs)[j].z() = mesh->getPoints()[vidx].z() / 256.0f;

                (*nrms)[j].x() = iter->getNormal().x() / 256.0f;
                (*nrms)[j].y() = iter->getNormal().y() / 256.0f;
                (*nrms)[j].z() = iter->getNormal().z() / 256.0f;

                (*binrms)[j].x() = iter->getBinormal().x() / 256.0f;
                (*binrms)[j].y() = iter->getBinormal().y() / 256.0f;
                (*binrms)[j].z() = iter->getBinormal().z() / 256.0f;

                (*texcrds)[j].x() = pt.u() / width;
                (*texcrds)[j].y() = pt.v() / height;

                (*colors)[j] = osg::Vec4ub(255, 255, 255, 255);

                if(j >= last_total+2)
                {
                    (*idxs)[(j-2)*3 + 0] = last_total;
                    (*idxs)[(j-2)*3 + 1] = j-1;
                    (*idxs)[(j-2)*3 + 2] = j;
                }

                ++j;
            }
        } while(++iter != mesh->getPlanes().end() && iter->getTextureId() == texid);

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
        geometry->setTexCoordArray(1, binrms, osg::Array::BIND_PER_VERTEX);
        geometry->setTexCoordArray(0, texcrds, osg::Array::BIND_PER_VERTEX);
        geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(true);

        geometry->addPrimitiveSet(idxs);

        /* Cache the stateset used for this texture, so it can be reused for
         * multiple models (should help OSG batch together objects with similar
         * state).
         */
        auto &stateiter = mStateSetCache[texid];
        osg::ref_ptr<osg::StateSet> ss;
        if(stateiter.lock(ss) && ss)
            geometry->setStateSet(ss);
        else
        {
            ss = geometry->getOrCreateStateSet();
            ss->setAttributeAndModes(mModelProgram);
            ss->addUniform(new osg::Uniform("diffuseTex", 0));
            ss->setTextureAttributeAndModes(0, tex);
            stateiter = ss;
        }

        geode->addDrawable(geometry);
    }

    mModelCache[idx] = osg::ref_ptr<osg::Node>(geode);
    return geode;
}

osg::ref_ptr<osg::Node> MeshManager::loadFlat(size_t texid, bool centered, size_t *num_frames)
{
    auto iter = mFlatCache.find(std::make_pair(texid, centered));
    if(iter != mFlatCache.end())
    {
        osg::ref_ptr<osg::Node> node;
        if(iter->second.lock(node))
        {
            if(num_frames)
            {
                osg::ref_ptr<osg::Texture> tex = TextureManager::get().getTexture(texid);
                *num_frames = tex->getTextureDepth();
            }
            return node;
        }
    }

    if(!mFlatProgram)
    {
        mFlatProgram = new osg::Program();
        mFlatProgram->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "shaders/sprite.vert"));
        mFlatProgram->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/sprite.frag"));
    }

    int16_t xoffset, yoffset;
    float xscale, yscale;
    osg::ref_ptr<osg::Texture> tex = TextureManager::get().getTexture(
        texid, &xoffset, &yoffset, &xscale, &yscale
    );
    if(num_frames)
        *num_frames = tex->getTextureDepth();

    float width = tex->getTextureWidth();
    float height = tex->getTextureHeight();

    osg::Matrix mat(osg::Matrixf::scale(xscale, yscale, xscale));
    //mat.postMultTranslate(osg::Vec3(xoffset, yoffset, 0)); seems to be incorrect values??
    osg::ref_ptr<osg::MatrixTransform> base(new osg::MatrixTransform(mat));

    osg::ref_ptr<osg::Billboard> bb(new osg::Billboard());
    bb->setMode(osg::Billboard::AXIAL_ROT);
    bb->setAxis(osg::Vec3(0.0f, 1.0f, 0.0f));
    bb->setNormal(osg::Vec3(0.0f, 0.0f, -1.0f));

    osg::ref_ptr<osg::Vec3Array> vtxs(new osg::Vec3Array(4));
    (*vtxs)[0] = osg::Vec3(width* 0.5f, height*-0.5f, 0.0f);
    (*vtxs)[1] = osg::Vec3(width*-0.5f, height*-0.5f, 0.0f);
    (*vtxs)[2] = osg::Vec3(width*-0.5f, height* 0.5f, 0.0f);
    (*vtxs)[3] = osg::Vec3(width* 0.5f, height* 0.5f, 0.0f);
    osg::ref_ptr<osg::Vec2Array> texcrds(new osg::Vec2Array(4));
    (*texcrds)[0] = osg::Vec2(1.0f, 0.0f);
    (*texcrds)[1] = osg::Vec2(0.0f, 0.0f);
    (*texcrds)[2] = osg::Vec2(0.0f, 1.0f);
    (*texcrds)[3] = osg::Vec2(1.0f, 1.0f);
    osg::ref_ptr<osg::Vec3Array> nrms(new osg::Vec3Array(4));
    (*nrms)[0] = osg::Vec3(0.0f, 0.0f, -1.0f);
    (*nrms)[1] = osg::Vec3(0.0f, 0.0f, -1.0f);
    (*nrms)[2] = osg::Vec3(0.0f, 0.0f, -1.0f);
    (*nrms)[3] = osg::Vec3(0.0f, 0.0f, -1.0f);
    osg::ref_ptr<osg::Vec4ubArray> colors(new osg::Vec4ubArray(4));
    (*colors)[0] = osg::Vec4ub(255, 255, 255, 255);
    (*colors)[1] = osg::Vec4ub(255, 255, 255, 255);
    (*colors)[2] = osg::Vec4ub(255, 255, 255, 255);
    (*colors)[3] = osg::Vec4ub(255, 255, 255, 255);
    colors->setNormalize(true);

    osg::ref_ptr<osg::VertexBufferObject> vbo(new osg::VertexBufferObject());
    vtxs->setVertexBufferObject(vbo);
    texcrds->setVertexBufferObject(vbo);
    nrms->setVertexBufferObject(vbo);
    colors->setVertexBufferObject(vbo);

    osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry);
    geometry->setVertexArray(vtxs);
    geometry->setTexCoordArray(0, texcrds, osg::Array::BIND_PER_VERTEX);
    geometry->setNormalArray(nrms, osg::Array::BIND_PER_VERTEX);
    geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
    geometry->setUseDisplayList(false);
    geometry->setUseVertexBufferObjects(true);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::StateSet *ss = geometry->getOrCreateStateSet();
    ss->setAttributeAndModes(mFlatProgram);
    // Alpha test is reversed, because the shader will set alpha=0 for texels
    // that should be kept, and consequently have no specular, and alpha=1 for
    // texels that should be dropped.
    ss->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::LESS, 0.5f));
    ss->addUniform(new osg::Uniform("diffuseTex", 0));
    ss->setTextureAttributeAndModes(0, tex);

    if(centered)
        bb->addDrawable(geometry);
    else
        bb->addDrawable(geometry, osg::Vec3(0.0f, height*-0.5f, 0.0f));
    base->addChild(bb);

    mFlatCache[std::make_pair(texid, centered)] = osg::ref_ptr<osg::Node>(base);
    return base;
}

osg::ref_ptr<osg::Node> MeshManager::getTerrain(float size)
{
    auto iter = mTerrainCache.find(size);
    if(iter != mTerrainCache.end())
    {
        osg::ref_ptr<osg::Node> node;
        if(iter->second.lock(node))
            return node;
    }

    if(!mTerrainProgram)
    {
        mTerrainProgram = new osg::Program();
        mTerrainProgram->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "shaders/terrain.vert"));
        mTerrainProgram->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "shaders/terrain.frag"));
    }

    osg::ref_ptr<osg::Vec3Array> vtxs(new osg::Vec3Array(4));
    (*vtxs)[0] = osg::Vec3(0.0f, 0.0f, 0.0f);
    (*vtxs)[1] = osg::Vec3(0.0f, 0.0f, -size);
    (*vtxs)[2] = osg::Vec3(size, 0.0f, -size);
    (*vtxs)[3] = osg::Vec3(size, 0.0f, 0.0f);
    osg::ref_ptr<osg::Vec2Array> texcrds(new osg::Vec2Array(4));
    (*texcrds)[0] = osg::Vec2(       0.0f, 0.0f);
    (*texcrds)[1] = osg::Vec2(       0.0f, size/256.0f);
    (*texcrds)[2] = osg::Vec2(size/256.0f, size/256.0f);
    (*texcrds)[3] = osg::Vec2(size/256.0f, 0.0f);
    osg::ref_ptr<osg::Vec3Array> nrms(new osg::Vec3Array(4));
    (*nrms)[0] = osg::Vec3(0.0f, -1.0f, 0.0f);
    (*nrms)[1] = osg::Vec3(0.0f, -1.0f, 0.0f);
    (*nrms)[2] = osg::Vec3(0.0f, -1.0f, 0.0f);
    (*nrms)[3] = osg::Vec3(0.0f, -1.0f, 0.0f);

    osg::ref_ptr<osg::VertexBufferObject> vbo(new osg::VertexBufferObject());
    vtxs->setVertexBufferObject(vbo);
    texcrds->setVertexBufferObject(vbo);
    nrms->setVertexBufferObject(vbo);

    osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry);
    geometry->setVertexArray(vtxs);
    geometry->setTexCoordArray(0, texcrds, osg::Array::BIND_PER_VERTEX);
    geometry->setNormalArray(nrms, osg::Array::BIND_PER_VERTEX);
    geometry->setUseDisplayList(false);
    geometry->setUseVertexBufferObjects(true);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::StateSet *ss = geometry->getOrCreateStateSet();
    ss->setAttributeAndModes(mTerrainProgram);
    ss->addUniform(new osg::Uniform("diffuseTex", 0));
    ss->addUniform(new osg::Uniform("tilemapTex", 1));

    osg::ref_ptr<osg::Geode> base(new osg::Geode());
    base->addDrawable(geometry);

    mTerrainCache[size] = osg::ref_ptr<osg::Node>(base);
    return base;
}

} // namespace Resource
