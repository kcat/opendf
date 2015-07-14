
#include "dblocks.hpp"
#include <log.hpp>

#include <osg/MatrixTransform>

#include "components/vfs/manager.hpp"
#include "components/dfosg/meshloader.hpp"


namespace DF
{

ObjectBase::ObjectBase(int x, int y, int z)
  : mXPos(x), mYPos(y), mZPos(z)
{
}


void ModelObject::load(std::istream &stream, const std::array<int,750> &mdlidx)
{
    mXRot = VFS::read_le32(stream);
    mYRot = VFS::read_le32(stream);
    mZRot = VFS::read_le32(stream);

    mModelIdx = mdlidx.at(VFS::read_le16(stream));
    mUnknown1 = VFS::read_le32(stream);
    mUnknown2 = stream.get();
    mActionOffset = VFS::read_le32(stream);
}

void ModelObject::buildNodes(osg::Group *root)
{
    osg::Matrix mat;
    mat.makeRotate(
        mXRot*3.14159f/1024.0f, osg::Vec3f(1.0f, 0.0f, 0.0f),
       -mYRot*3.14159f/1024.0f, osg::Vec3f(0.0f, 1.0f, 0.0f),
        mZRot*3.14159f/1024.0f, osg::Vec3f(0.0f, 0.0f, 1.0f)
    );
    mat.postMultTranslate(osg::Vec3(mXPos, mYPos, mZPos));

    mBaseNode = new osg::MatrixTransform(mat);
    mBaseNode->addChild(DFOSG::MeshLoader::get().load(mModelIdx));
    root->addChild(mBaseNode);
}

DBlockHeader::~DBlockHeader()
{
    detachNode();
}


void DBlockHeader::load(std::istream &stream)
{
    mUnknown1 = VFS::read_le32(stream);
    mWidth = VFS::read_le32(stream);
    mHeight = VFS::read_le32(stream);
    mobjectRootOffset = VFS::read_le32(stream);
    mUnknown2 = VFS::read_le32(stream);
    stream.read(mModelData[0].data(), sizeof(mModelData));
    for(uint32_t &val : mUnknown3)
        val = VFS::read_le32(stream);

    std::array<int,750> mdlidx;
    for(size_t i = 0;i < 750;++i)
    {
        if(mModelData[i][0] == -1)
            mdlidx[i] = -1;
        else
        {
            std::array<char,6> id{{ mModelData[i][0], mModelData[i][1], mModelData[i][2], mModelData[i][3], mModelData[i][4], 0 }};
            mdlidx[i] = strtol(id.data(), nullptr, 10);
        }
    }

    stream.seekg(mobjectRootOffset);

    std::vector<int32_t> rootoffsets(mWidth*mHeight);
    for(int32_t &val : rootoffsets)
        val = VFS::read_le32(stream);

    for(int32_t offset : rootoffsets)
    {
        while(offset > 0)
        {
            stream.seekg(offset);
            int32_t next = VFS::read_le32(stream);
            /*int32_t prev =*/ VFS::read_le32(stream);

            int32_t x = VFS::read_le32(stream);
            int32_t y = VFS::read_le32(stream);
            int32_t z = VFS::read_le32(stream);
            uint8_t type = stream.get();
            uint32_t objoffset = VFS::read_le32(stream);

            if(type == 0x01)
            {
                stream.seekg(objoffset);
                ref_ptr<ModelObject> mdl(new ModelObject(x, y, z));
                mdl->load(stream, mdlidx);

                auto ret = mObjectIds.insert(offset);
                if(ret.second)
                {
                    size_t pos = std::distance(mObjectIds.begin(), ret.first);
                    mObjects.resize(mObjectIds.size()-1);
                    mObjects.insert(mObjects.begin()+pos, mdl);
                }
            }

            offset = next;
        }
    }
}


void DBlockHeader::buildNodes(osg::Group *root, int x, int z)
{
    if(!mBaseNode)
    {
        osg::ref_ptr<osg::MatrixTransform> base(new osg::MatrixTransform());
        base->setMatrix(osg::Matrix::translate(x*2048.0f, 0.0f, z*2048.0f));
        mBaseNode = base;

        for(ref_ptr<ObjectBase> &obj : mObjects)
            obj->buildNodes(mBaseNode.get());
    }

    root->addChild(mBaseNode);
}

void DBlockHeader::detachNode()
{
    if(!mBaseNode) return;
    while(mBaseNode->getNumParents() > 0)
    {
        osg::Group *parent = mBaseNode->getParent(0);
        parent->removeChild(mBaseNode);
    }
}


} // namespace DF
