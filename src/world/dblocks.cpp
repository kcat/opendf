
#include "dblocks.hpp"

#include <iomanip>

#include <osg/MatrixTransform>

#include "iface.hpp"
#include "log.hpp"

#include "components/vfs/manager.hpp"
#include "components/resource/meshmanager.hpp"


namespace DF
{

ObjectBase::ObjectBase(uint8_t type, int x, int y, int z)
  : mType(type), mXPos(x), mYPos(y), mZPos(z)
{
}

void ObjectBase::print(LogStream &stream) const
{
    stream<<std::setfill('0');
    stream<< "   Type: 0x"<<std::hex<<std::setw(2)<<(int)mType<<std::dec<<std::setw(0)<<"\n";
    stream<< "   Pos: "<<mXPos<<" "<<mYPos<<" "<<mZPos<<"\n";
    stream<<std::setfill(' ');
}

void ObjectBase::print(std::ostream &stream) const
{
    stream<< "Type: 0x"<<std::hex<<std::setw(2)<<(int)mType<<std::dec<<std::setw(0)<<"\n";
    stream<< "Pos: "<<mXPos<<" "<<mYPos<<" "<<mZPos<<"\n";
}


void ModelObject::load(std::istream &stream, const std::array<std::array<char,8>,750> &mdldata)
{
    mXRot = VFS::read_le32(stream);
    mYRot = VFS::read_le32(stream);
    mZRot = VFS::read_le32(stream);

    mModelIdx = VFS::read_le16(stream);
    mUnknown1 = VFS::read_le32(stream);
    mUnknown2 = stream.get();
    mActionOffset = VFS::read_le32(stream);

    mModelData = mdldata.at(mModelIdx);
}

void ModelObject::buildNodes(osg::Group *root, size_t objid)
{
    if(mModelData[0] == -1)
        return;

    osg::Matrix mat;
    mat.makeRotate(
        mXRot*3.14159f/1024.0f, osg::Vec3f(1.0f, 0.0f, 0.0f),
       -mYRot*3.14159f/1024.0f, osg::Vec3f(0.0f, 1.0f, 0.0f),
        mZRot*3.14159f/1024.0f, osg::Vec3f(0.0f, 0.0f, 1.0f)
    );
    mat.postMultTranslate(osg::Vec3(mXPos, mYPos, mZPos));

    std::array<char,6> id{{ mModelData[0], mModelData[1], mModelData[2],
                            mModelData[3], mModelData[4], 0 }};
    size_t mdlidx = strtol(id.data(), nullptr, 10);

    mBaseNode = new osg::MatrixTransform(mat);
    mBaseNode->setNodeMask(WorldIface::Mask_Static);
    mBaseNode->setUserData(new ObjectRef(objid));
    mBaseNode->addChild(Resource::MeshManager::get().get(mdlidx));
    root->addChild(mBaseNode);
}

void ModelObject::print(LogStream &stream) const
{
    ObjectBase::print(stream);

    stream<<std::setfill('0');
    stream<< "   Rotation: "<<mXRot<<" "<<mYRot<<" "<<mZRot<<"\n";
    stream<< "   ModelIdx: "<<mModelIdx<<"\n";
    stream<< "   Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown1<<std::dec<<std::setw(0)<<"\n";
    stream<< "   Unknown: 0x"<<std::hex<<std::setw(2)<<(int)mUnknown2<<std::dec<<std::setw(0)<<"\n";
    stream<< "   ActionOffset: 0x"<<std::hex<<std::setw(8)<<mActionOffset<<std::dec<<std::setw(0)<<"\n";
    stream<<std::setfill(' ');
}

void ModelObject::print(std::ostream &stream) const
{
    DF::ObjectBase::print(stream);

    std::array<char,9> id{{ mModelData[0], mModelData[1], mModelData[2], mModelData[3],
                            mModelData[4], mModelData[5], mModelData[6], mModelData[7], 0 }};

    stream<< "Rotation: "<<mXRot<<" "<<mYRot<<" "<<mZRot<<"\n";
    stream<< "ModelIdx: "<<mModelIdx<<" ("<<id.data()<<")\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown1<<std::dec<<std::setw(0)<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(2)<<(int)mUnknown2<<std::dec<<std::setw(0)<<"\n";
    stream<< "ActionOffset: 0x"<<std::hex<<std::setw(8)<<mActionOffset<<std::dec<<std::setw(0)<<"\n";
}


void FlatObject::load(std::istream &stream)
{
    mTexture = VFS::read_le16(stream);
    mGender = VFS::read_le16(stream);
    mFactionId = VFS::read_le16(stream);
    stream.read(reinterpret_cast<char*>(mUnknown), sizeof(mUnknown));
}

void FlatObject::buildNodes(osg::Group *root, size_t objid)
{
    mBaseNode = new osg::MatrixTransform(osg::Matrix::translate(mXPos, mYPos, mZPos));
    mBaseNode->setNodeMask(WorldIface::Mask_Flat);
    mBaseNode->setUserData(new ObjectRef(objid));
    mBaseNode->addChild(Resource::MeshManager::get().loadFlat(mTexture));
    root->addChild(mBaseNode);
}

void FlatObject::print(LogStream &stream) const
{
    ObjectBase::print(stream);

    stream<<std::setfill('0');
    stream<< "   Texture: 0x"<<std::hex<<std::setw(4)<<mTexture<<std::dec<<std::setw(0)<<"\n";
    stream<< "   Gender: 0x"<<std::hex<<std::setw(4)<<mGender<<std::dec<<std::setw(0)<<"\n";
    stream<< "   FactionId: "<<mFactionId<<"\n";
    stream<< "   Unknown:";
    for(int unk : mUnknown)
        stream<< " 0x"<<std::hex<<std::setw(2)<<unk;
    stream<<std::setw(0)<<std::dec<<"\n";
    stream<<std::setfill(' ');
}

void FlatObject::print(std::ostream &stream) const
{
    DF::ObjectBase::print(stream);

    stream<< "Texture: 0x"<<std::hex<<std::setw(4)<<mTexture<<std::dec<<std::setw(0)<<"\n";
    stream<< "Gender: 0x"<<std::hex<<std::setw(4)<<mGender<<std::dec<<std::setw(0)<<"\n";
    stream<< "FactionId: "<<mFactionId<<"\n";
    stream<< "Unknown:";
    for(int unk : mUnknown)
        stream<< " 0x"<<std::hex<<std::setw(2)<<unk;
    stream<<std::setw(0)<<std::dec<<"\n";
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

            if(type == ObjectType_Model)
            {
                stream.seekg(objoffset);
                ref_ptr<ModelObject> mdl(new ModelObject(x, y, z));
                mdl->load(stream, mModelData);

                auto ret = mObjectIds.insert(offset);
                if(ret.second)
                {
                    size_t pos = std::distance(mObjectIds.begin(), ret.first);
                    mObjects.resize(mObjectIds.size()-1);
                    mObjects.insert(mObjects.begin()+pos, mdl);
                }
            }
            else if(type == ObjectType_Flat)
            {
                stream.seekg(objoffset);
                ref_ptr<FlatObject> flat(new FlatObject(x, y, z));
                flat->load(stream);

                auto ret = mObjectIds.insert(offset);
                if(ret.second)
                {
                    size_t pos = std::distance(mObjectIds.begin(), ret.first);
                    mObjects.resize(mObjectIds.size()-1);
                    mObjects.insert(mObjects.begin()+pos, flat);
                }
            }

            offset = next;
        }
    }
}


void DBlockHeader::buildNodes(osg::Group *root, size_t blockid, int x, int z)
{
    if(!mBaseNode)
    {
        osg::ref_ptr<osg::MatrixTransform> base(new osg::MatrixTransform());
        base->setMatrix(osg::Matrix::translate(x*2048.0f, 0.0f, z*2048.0f));
        mBaseNode = base;

        auto iditer = mObjectIds.begin();
        for(ref_ptr<ObjectBase> &obj : mObjects)
            obj->buildNodes(mBaseNode.get(), (blockid<<24) | *(iditer++));
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


ObjectBase *DBlockHeader::getObject(size_t id)
{
    auto iter = mObjectIds.find(id);
    if(iter == mObjectIds.end())
        return nullptr;
    return mObjects[std::distance(mObjectIds.begin(), iter)].get();
}


FlatObject *DBlockHeader::getFlatByTexture(size_t texid) const
{
    for(ref_ptr<ObjectBase> obj : mObjects)
    {
        if(obj->mType == ObjectType_Flat)
        {
            FlatObject *flat = static_cast<FlatObject*>(obj.get());
            if(flat->mTexture == texid) return flat;
        }
    }
    std::stringstream sstr;
    sstr<< "Failed to find Flat with texture 0x"<<std::setfill('0')<<std::setw(4)<<std::hex<<texid;
    throw std::runtime_error(sstr.str());
}


void DBlockHeader::print(LogStream &stream, int objtype) const
{
    stream<<std::setfill('0');
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown1<<std::dec<<std::setw(0)<<"\n";
    stream<< "  Width: "<<mWidth<<"\n";
    stream<< "  Height: "<<mHeight<<"\n";
    stream<< "  ObjectRootOffset: 0x"<<std::hex<<std::setw(8)<<mobjectRootOffset<<std::dec<<std::setw(0)<<"\n";
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown2<<std::dec<<std::setw(0)<<"\n";
    stream<< "  ModelData:"<<"\n";
    const uint32_t *unknown = mUnknown3.data();
    int idx = 0;
    for(const auto &id : mModelData)
    {
        if(id[0] != -1)
        {
            std::array<char,9> disp{{id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], 0}};
            stream<< "   "<<idx<<": "<<disp.data()<<" 0x"<<std::hex<<std::setw(8)<<*unknown<<std::dec<<std::setw(0)<<"\n";
        }
        ++unknown;
        ++idx;
    }
    stream<<std::setfill(' ');

    auto iditer = mObjectIds.begin();
    for(ref_ptr<ObjectBase> obj : mObjects)
    {
        stream<< "  Object 0x"<<std::setfill('0')<<std::hex<<std::setw(8)<<*iditer<<std::dec<<std::setw(0)<<std::setfill(' ')<<"\n";
        obj->print(stream);
        ++iditer;
    }
}


} // namespace DF
