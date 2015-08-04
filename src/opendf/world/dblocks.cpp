
#include "dblocks.hpp"

#include <algorithm>
#include <iomanip>

#include <osg/MatrixTransform>

#include "world.hpp"
#include "log.hpp"

#include "components/vfs/manager.hpp"
#include "components/resource/meshmanager.hpp"

#include "render/renderer.hpp"
#include "class/placeable.hpp"
#include "class/activator.hpp"
#include "class/linker.hpp"
#include "class/mover.hpp"
#include "class/door.hpp"
#include "class/exitdoor.hpp"


namespace DF
{

ObjectBase::ObjectBase(size_t id, uint8_t type, int x, int y, int z)
  : mId(id), mType(type)
  , mXPos(x), mYPos(y), mZPos(z)
  , mXRot(0), mYRot(0), mZRot(0)
  , mActionFlags(0)
  , mActionOffset(0)
{
}
ObjectBase::~ObjectBase()
{
    Activator::get().deallocate(mId);
}

void ObjectBase::loadAction(std::istream &stream)
{
    if(mActionOffset <= 0)
        return;

    std::array<uint8_t,5> adata;
    stream.seekg(mActionOffset);
    stream.read(reinterpret_cast<char*>(adata.data()), adata.size());
    int32_t target = VFS::read_le32(stream);
    uint8_t type = stream.get();

    if(target > 0)
        target |= mId&0xff000000;

    if(type == Action_Translate)
    {
        osg::Vec3 amount;
        if(adata[0] == ActionTranslate::Axis_X)
            amount.x() += adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionTranslate::Axis_NegX)
            amount.x() -= adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionTranslate::Axis_Y)
            amount.y() += adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionTranslate::Axis_NegY)
            amount.y() -= adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionTranslate::Axis_Z)
            amount.z() += adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionTranslate::Axis_NegZ)
            amount.z() -= adata[3] | (adata[4]<<8);
        float duration = (adata[1] | (adata[2]<<8)) / 16.0f;

        Mover::get().allocateTranslate(mId, mSoundId, osg::Vec3f(mXPos, mYPos, mZPos), amount, duration);
        Activator::get().allocate(mId, mActionFlags, Mover::activateTranslateFunc,
                                  ((target > 0) ? target : ~static_cast<size_t>(0)),
                                  Mover::deallocateTranslateFunc);
    }
    else if(type == Action_Rotate)
    {
        osg::Vec3 amount;
        if(adata[0] == ActionRotate::Axis_X)
            amount.x() += adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionRotate::Axis_NegX)
            amount.x() -= adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionRotate::Axis_Y)
            amount.y() += adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionRotate::Axis_NegY)
            amount.y() -= adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionRotate::Axis_Z)
            amount.z() += adata[3] | (adata[4]<<8);
        else if(adata[0] == ActionRotate::Axis_NegZ)
            amount.z() -= adata[3] | (adata[4]<<8);
        float duration = (adata[1] | (adata[2]<<8)) / 16.0f;

        Mover::get().allocateRotate(mId, mSoundId, osg::Vec3f(0.0f, 0.0f, 0.0f), amount, duration);
        Activator::get().allocate(mId, mActionFlags, Mover::activateRotateFunc,
                                  ((target > 0) ? target : ~static_cast<size_t>(0)),
                                  Mover::deallocateRotateFunc);
    }
    else if(type == Action_Linker)
    {
        Activator::get().allocate(mId, mActionFlags, Linker::activateFunc,
                                  ((target > 0) ? target : ~static_cast<size_t>(0)),
                                  Linker::deallocateFunc);
    }
    else
    {
        Log::get().stream(Log::Level_Error)<< "Unhandled action type: 0x"<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)type;
    }
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
    mActionFlags = VFS::read_le32(stream);
    mSoundId = stream.get();
    mActionOffset = VFS::read_le32(stream);

    mModelData = mdldata.at(mModelIdx);
}

void ModelObject::buildNodes(osg::Group *root, size_t regnum, size_t locnum)
{
    if(mModelData[0] == -1)
        return;

    std::array<char,6> id{{ mModelData[0], mModelData[1], mModelData[2],
                            mModelData[3], mModelData[4], 0 }};
    size_t mdlidx = strtol(id.data(), nullptr, 10);

    osg::ref_ptr<osg::MatrixTransform> node(new osg::MatrixTransform());
    node->setNodeMask(Renderer::Mask_Static);
    node->setUserData(new ObjectRef(mId));
    node->addChild(Resource::MeshManager::get().get(mdlidx));
    root->addChild(node);

    // Is this how doors are specified, or is it determined by the model index?
    // What to do if a door has an action?
    if(mActionOffset <= 0 && mModelData[5] == 'D' && mModelData[6] == 'O' && mModelData[7] == 'R')
    {
        Door::get().allocate(mId, 0.0f);
        Activator::get().allocate(mId, mActionFlags|0x02, Door::activateFunc, ~static_cast<size_t>(0), Door::deallocateFunc);
    }
    else if(mModelData[5] == 'E' && mModelData[6] == 'X' && mModelData[7] == 'T')
    {
        ExitDoor::get().allocate(mId, regnum, locnum);
        Activator::get().allocate(mId, mActionFlags|0x02, ExitDoor::activateFunc, ~static_cast<size_t>(0), ExitDoor::deallocateFunc);
    }
    Renderer::get().setNode(mId, node);
    Placeable::get().setPos(mId, osg::Vec3f(mXPos, mYPos, mZPos), osg::Vec3f(mXRot, mYRot, mZRot));
}

void ModelObject::print(std::ostream &stream) const
{
    DF::ObjectBase::print(stream);

    std::array<char,9> id{{ mModelData[0], mModelData[1], mModelData[2], mModelData[3],
                            mModelData[4], mModelData[5], mModelData[6], mModelData[7], 0 }};

    stream<< "Rotation: "<<mXRot<<" "<<mYRot<<" "<<mZRot<<"\n";
    stream<< "ModelIdx: "<<mModelIdx<<" ("<<id.data()<<")\n";
    stream<< "ActionFlags: 0x"<<std::hex<<std::setw(8)<<mActionFlags<<std::dec<<std::setw(0)<<"\n";
    stream<< "SoundId: "<<(int)mSoundId<<"\n";
}


void FlatObject::load(std::istream &stream)
{
    mTexture = VFS::read_le16(stream);
    mGender = VFS::read_le16(stream);
    mFactionId = VFS::read_le16(stream);
    mActionOffset = VFS::read_le32(stream);
    mUnknown = stream.get();
}

void FlatObject::buildNodes(osg::Group *root, size_t /*regnum*/, size_t /*locnum*/)
{
    osg::ref_ptr<osg::MatrixTransform> node(new osg::MatrixTransform());
    node->setNodeMask(Renderer::Mask_Flat);
    node->setUserData(new ObjectRef(mId));
    node->addChild(Resource::MeshManager::get().loadFlat(mTexture, true));
    root->addChild(node);

    Renderer::get().setNode(mId, node);
    Placeable::get().setPoint(mId, osg::Vec3f(mXPos, mYPos, mZPos));
}

void FlatObject::print(std::ostream &stream) const
{
    DF::ObjectBase::print(stream);

    stream<< "Texture: 0x"<<std::hex<<std::setw(4)<<mTexture<<std::dec<<std::setw(0)<<"\n";
    stream<< "Gender: 0x"<<std::hex<<std::setw(4)<<mGender<<std::dec<<std::setw(0)<<"\n";
    stream<< "FactionId: "<<mFactionId<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(2)<<(int)mUnknown<<std::setw(0)<<std::dec<<"\n";
}


DBlockHeader::~DBlockHeader()
{
    detachNode();
    if(!mObjects.empty())
    {
        Renderer::get().remove(&*mObjects.getIdList(), mObjects.size());
        Placeable::get().deallocate(&*mObjects.getIdList(), mObjects.size());
    }
}


void DBlockHeader::load(std::istream &stream, size_t blockid, float x, float z, size_t regnum, size_t locnum, osg::Group *root)
{
    mUnknown1 = VFS::read_le32(stream);
    mWidth = VFS::read_le32(stream);
    mHeight = VFS::read_le32(stream);
    mObjectRootOffset = VFS::read_le32(stream);
    mUnknown2 = VFS::read_le32(stream);
    stream.read(mModelData[0].data(), sizeof(mModelData));
    for(uint32_t &val : mUnknown3)
        val = VFS::read_le32(stream);

    stream.seekg(mObjectRootOffset);

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
                ref_ptr<ModelObject> mdl(new ModelObject(blockid|offset, x, y, z));
                mdl->load(stream, mModelData);

                mObjects.insert(blockid|offset, mdl);
            }
            else if(type == ObjectType_Flat)
            {
                stream.seekg(objoffset);
                ref_ptr<FlatObject> flat(new FlatObject(blockid|offset, x, y, z));
                flat->load(stream);

                mObjects.insert(blockid|offset, flat);
            }

            offset = next;
        }
    }

    for(ref_ptr<ObjectBase> &obj : mObjects)
        obj->loadAction(stream);

    mBaseNode = new osg::MatrixTransform(osg::Matrix::translate(x, 0.0f, z));
    for(ref_ptr<ObjectBase> &obj : mObjects)
        obj->buildNodes(mBaseNode.get(), regnum, locnum);

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
    auto iter = mObjects.find(id);
    if(iter == mObjects.end())
        return nullptr;
    return *iter;
}

size_t DBlockHeader::getObjectByTexture(size_t texid) const
{
    for(const ObjectBase *obj : mObjects)
    {
        if(obj->mType == ObjectType_Flat)
        {
            const FlatObject *flat = static_cast<const FlatObject*>(obj);
            if(flat->mTexture == texid) return flat->mId;
        }
    }
    Log::get().stream(Log::Level_Error)<< "Failed to find Flat with texture 0x"<<std::setfill('0')<<std::setw(4)<<std::hex<<texid;
    return ~static_cast<size_t>(0);
}


void DBlockHeader::print(std::ostream &stream, int objtype) const
{
    stream<< "Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown1<<std::dec<<std::setw(0)<<"\n";
    stream<< "Width: "<<mWidth<<"\n";
    stream<< "Height: "<<mHeight<<"\n";
    stream<< "ObjectRootOffset: 0x"<<std::hex<<std::setw(8)<<mObjectRootOffset<<std::dec<<std::setw(0)<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown2<<std::dec<<std::setw(0)<<"\n";
    stream<< "ModelData:"<<"\n";
    const uint32_t *unknown = mUnknown3.data();
    int idx = 0;
    for(const auto &id : mModelData)
    {
        if(id[0] != -1)
        {
            std::array<char,9> disp{{id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], 0}};
            stream<< " "<<idx<<": "<<disp.data()<<" 0x"<<std::hex<<std::setw(8)<<*unknown<<std::setw(0)<<std::dec<<"\n";
        }
        ++unknown;
        ++idx;
    }

    auto iditer = mObjects.getIdList();
    for(ref_ptr<ObjectBase> obj : mObjects)
    {
        stream<< "**** Object 0x"<<std::hex<<std::setw(8)<<*iditer<<std::setw(0)<<std::dec<<" ****\n";
        obj->print(stream);
        ++iditer;
    }
}

} // namespace DF
