
#include "meshmanager.hpp"

#include <osg/Node>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/AlphaFunc>

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

    DFOSG::Mesh *mesh = DFOSG::MeshLoader::get().load(idx);

    osg::ref_ptr<osg::Geode> geode(new osg::Geode());
    for(auto iter = mesh->getPlanes().begin();iter != mesh->getPlanes().end();)
    {
        osg::ref_ptr<osg::Vec3Array> vtxs(new osg::Vec3Array());
        osg::ref_ptr<osg::Vec3Array> nrms(new osg::Vec3Array());
        osg::ref_ptr<osg::Vec2Array> texcrds(new osg::Vec2Array());
        osg::ref_ptr<osg::Vec4ubArray> colors(new osg::Vec4ubArray());
        osg::ref_ptr<osg::DrawElementsUShort> idxs(new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES));
        uint16_t texid = iter->getTextureId();

        osg::ref_ptr<osg::Texture> tex = TextureManager::get().get(texid);
        float width = tex->getTextureWidth();
        float height = tex->getTextureHeight();

        do {
            //iter->fixUVs(mesh->getPoints(), tex->getTextureWidth(), tex->getTextureHeight());

            const std::vector<DFOSG::MdlPlanePoint> &pts = iter->getPoints();
            size_t last_total = vtxs->size();

            vtxs->resize(last_total + pts.size());
            nrms->resize(last_total + pts.size());
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
        geometry->setTexCoordArray(0, texcrds, osg::Array::BIND_PER_VERTEX);
        geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(true);

        geometry->addPrimitiveSet(idxs);

        osg::StateSet *ss = geometry->getOrCreateStateSet();
        ss->setTextureAttributeAndModes(0, tex);

        geode->addDrawable(geometry);
    }

    mModelCache[idx] = osg::ref_ptr<osg::Node>(geode);
    return geode;
}

osg::ref_ptr<osg::Node> MeshManager::loadFlat(size_t texid, size_t *num_frames, osg::Matrixf *mtx)
{
    /* Nodes for flats are stored with an inverted texid as a lookup, to avoid
     * clashes with ARCH3D indices. */
    auto iter = mModelCache.find(~texid);
    if(iter != mModelCache.end())
    {
        osg::ref_ptr<osg::Node> node;
        if(iter->second.lock(node))
        {
            if(mtx || num_frames)
            {
                int16_t xoffset, yoffset;
                float xscale, yscale;
                osg::ref_ptr<osg::Texture> tex = TextureManager::get().get(
                    texid, &xoffset, &yoffset, &xscale, &yscale
                );
                if(num_frames)
                    *num_frames = tex->getTextureDepth();
                if(mtx)
                {
                    *mtx = osg::Matrixf::scale(xscale, yscale, 0.0f);
                    mtx->postMultTranslate(osg::Vec3(xoffset, yoffset, 0));
                }
            }
            return node;
        }
    }

    int16_t xoffset, yoffset;
    float xscale, yscale;
    osg::ref_ptr<osg::Texture> tex = TextureManager::get().get(
        texid, &xoffset, &yoffset, &xscale, &yscale
    );
    if(num_frames)
        *num_frames = tex->getTextureDepth();
    if(mtx)
    {
        *mtx = osg::Matrixf::scale(xscale, yscale, 0.0f);
        mtx->postMultTranslate(osg::Vec3(xoffset, yoffset, 0));
    }

    float width = tex->getTextureWidth();
    float height = tex->getTextureHeight();

    osg::ref_ptr<osg::Billboard> base(new osg::Billboard());
    base->setMode(osg::Billboard::AXIAL_ROT);
    base->setAxis(osg::Vec3(0.0f, 1.0f, 0.0f));
    base->setNormal(osg::Vec3(0.0f, 0.0f, -1.0f));

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
    osg::ref_ptr<osg::Vec3Array> nrms(new osg::Vec3Array());
    nrms->push_back(osg::Vec3(0.0f, 0.0f, -1.0f));
    osg::ref_ptr<osg::Vec4ubArray> colors(new osg::Vec4ubArray());
    colors->push_back(osg::Vec4ub(255, 255, 255, 255));
    colors->setNormalize(true);

    osg::ref_ptr<osg::VertexBufferObject> vbo(new osg::VertexBufferObject());
    vtxs->setVertexBufferObject(vbo);
    texcrds->setVertexBufferObject(vbo);

    osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry);
    geometry->setVertexArray(vtxs);
    geometry->setTexCoordArray(0, texcrds, osg::Array::BIND_PER_VERTEX);
    geometry->setNormalArray(nrms, osg::Array::BIND_OVERALL);
    geometry->setColorArray(colors, osg::Array::BIND_OVERALL);
    geometry->setUseDisplayList(false);
    geometry->setUseVertexBufferObjects(true);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::StateSet *ss = geometry->getOrCreateStateSet();
    ss->setTextureAttributeAndModes(0, tex);
    ss->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.5f));

    base->addDrawable(geometry);

    mModelCache[~texid] = osg::ref_ptr<osg::Node>(base);
    return base;
}


} // namespace Resource
