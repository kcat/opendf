
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


namespace DF
{

void ActionBase::print(std::ostream &stream) const
{
    stream<< "Type: 0x"<<std::hex<<std::setw(2)<<(int)mType<<std::dec<<std::setw(0)<<"\n";
    stream<< "TimeAccum: "<<mTimeAccum<<"\n";
}


void ActionMovable::load(const std::array<uint8_t,5> &data)
{
    mAxis = data[0];
    mDuration = (data[1] | (data[2]<<8)) / 16.0f;
    mMagnitude = data[3] | (data[4]<<8);
}

void ActionMovable::print(std::ostream& stream) const
{
    DF::ActionBase::print(stream);

    stream<< "Axis: 0x"<<std::hex<<std::setw(2)<<(int)mAxis<<std::dec<<std::setw(0)<<"\n";
    stream<< "Duration: "<<mDuration<<"\n";
    stream<< "Magnitude: "<<mMagnitude<<"\n";
}


bool ActionTranslate::update(ObjectBase *target, float timediff)
{
    mTimeAccum = std::min<float>(mTimeAccum+timediff, mDuration);

    float delta = mTimeAccum / mDuration;
    if(target->mReverse) delta = 1.0f - delta;

    osg::Vec3f pos(target->mXPos, target->mYPos, target->mZPos);
    if(mAxis == Axis_X)
        pos.x() += (mMagnitude*delta);
    else if(mAxis == Axis_NegX)
        pos.x() -= (mMagnitude*delta);
    else if(mAxis == Axis_Y)
        pos.y() += (mMagnitude*delta);
    else if(mAxis == Axis_NegY)
        pos.y() -= (mMagnitude*delta);
    else if(mAxis == Axis_Z)
        pos.z() += (mMagnitude*delta);
    else if(mAxis == Axis_NegZ)
        pos.z() -= (mMagnitude*delta);
    Placeable::get().setPoint(target->mId, pos);

    if(mTimeAccum >= mDuration)
    {
        target->mReverse = !target->mReverse;
        mTimeAccum = 0.0f;
        return false;
    }
    return true;
}

bool ActionRotate::update(ObjectBase *target, float timediff)
{
    mTimeAccum = std::min<float>(mTimeAccum+timediff, mDuration);

    float delta = mTimeAccum / mDuration;
    if(target->mReverse) delta = 1.0f - delta;

    osg::Vec3f rot(target->mXRot, target->mYRot, target->mZRot);
    if(mAxis == Axis_X)
        rot.x() += (mMagnitude*delta);
    else if(mAxis == Axis_NegX)
        rot.x() -= (mMagnitude*delta);
    else if(mAxis == Axis_Y)
        rot.y() += (mMagnitude*delta);
    else if(mAxis == Axis_NegY)
        rot.y() -= (mMagnitude*delta);
    else if(mAxis == Axis_Z)
        rot.z() += (mMagnitude*delta);
    else if(mAxis == Axis_NegZ)
        rot.z() -= (mMagnitude*delta);
    Placeable::get().setRotate(target->mId, rot);

    if(mTimeAccum >= mDuration)
    {
        target->mReverse = !target->mReverse;
        mTimeAccum = 0.0f;
        return false;
    }
    return true;
}

void ActionLinker::load(const std::array<uint8_t,5>& /*data*/)
{
}

bool ActionLinker::update(ObjectBase *target, float timediff)
{
    return false;
}

void ActionUnknown::load(const std::array<uint8_t,5> &data)
{
    mData = data;

    LogStream stream(Log::get().stream(Log::Level_Error));
    stream<<std::setfill('0');
    stream<< "Unhandled action "<<this<<" data:"
          << " 0x"<<std::hex<<std::setw(2)<<(int)data[0]<<" 0x"<<std::hex<<std::setw(2)<<(int)data[1]
          << " 0x"<<std::hex<<std::setw(2)<<(int)data[2]<<" 0x"<<std::hex<<std::setw(2)<<(int)data[3]
          << " 0x"<<std::hex<<std::setw(2)<<(int)data[4];
}

bool ActionUnknown::update(ObjectBase *target, float timediff)
{
    Log::get().stream(Log::Level_Error)<< "Unhandled action on "<<this;
    return false;
}

void ActionUnknown::print(std::ostream& stream) const
{
    DF::ActionBase::print(stream);
    stream<< "Data:"
          << " 0x"<<std::hex<<std::setw(2)<<(int)mData[0]<<" 0x"<<std::hex<<std::setw(2)<<(int)mData[1]
          << " 0x"<<std::hex<<std::setw(2)<<(int)mData[2]<<" 0x"<<std::hex<<std::setw(2)<<(int)mData[3]
          << " 0x"<<std::hex<<std::setw(2)<<(int)mData[4]<<"\n";
}


ObjectBase::ObjectBase(size_t id, uint8_t type, int x, int y, int z)
  : mId(id), mType(type), mActive(false), mReverse(false)
  , mXPos(x), mYPos(y), mZPos(z)
  , mXRot(0), mYRot(0), mZRot(0)
  , mActionFlags(0)
  , mActionOffset(0)
{
}
ObjectBase::~ObjectBase()
{
}

void ObjectBase::loadAction(std::istream &stream, DBlockHeader &block)
{
    if(mActionOffset <= 0)
        return;

    std::array<uint8_t,5> adata;
    stream.seekg(mActionOffset);
    stream.read(reinterpret_cast<char*>(adata.data()), adata.size());
    int32_t target = VFS::read_le32(stream);
    uint8_t type = stream.get();

    ObjectBase *link = nullptr;
    if(target > 0)
    {
        target |= mId&0xff000000;
        link = block.getObject(target);
    }

    if(type == Action_Translate)
        mAction = new ActionTranslate(link);
    else if(type == Action_Rotate)
        mAction = new ActionRotate(link);
    else if(type == Action_Linker)
        mAction = new ActionLinker(link);
    else
    {
        Log::get().stream(Log::Level_Error)<< "Unhandled action type: 0x"<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)type;
        mAction = new ActionUnknown(type, link);
    }
    mAction->load(adata);
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

void ModelObject::buildNodes(osg::Group *root)
{
    if(mModelData[0] == -1)
        return;

    std::array<char,6> id{{ mModelData[0], mModelData[1], mModelData[2],
                            mModelData[3], mModelData[4], 0 }};
    size_t mdlidx = strtol(id.data(), nullptr, 10);

    osg::ref_ptr<osg::MatrixTransform> node(new osg::MatrixTransform());
    node->setNodeMask(WorldIface::Mask_Static);
    node->setUserData(new ObjectRef(mId));
    node->addChild(Resource::MeshManager::get().get(mdlidx));
    root->addChild(node);

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
    if(mAction)
    {
        stream<< "** Action **\n";
        mAction->print(stream);
    }
}


void FlatObject::load(std::istream &stream)
{
    mTexture = VFS::read_le16(stream);
    mGender = VFS::read_le16(stream);
    mFactionId = VFS::read_le16(stream);
    mActionOffset = VFS::read_le32(stream);
    mUnknown = stream.get();
}

void FlatObject::buildNodes(osg::Group *root)
{
    osg::ref_ptr<osg::MatrixTransform> node(new osg::MatrixTransform());
    node->setNodeMask(WorldIface::Mask_Flat);
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
    if(mAction)
    {
        stream<< "** Action **\n";
        mAction->print(stream);
    }
}


DBlockHeader::~DBlockHeader()
{
    detachNode();
    if(!mObjects.empty())
    {
        Renderer::get().remove(&*mObjects.getIdList(),
                               mObjects.size());
        Placeable::get().deallocate(&*mObjects.getIdList(),
                                    mObjects.size());
    }
}


void DBlockHeader::load(std::istream &stream, size_t blockid)
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
        obj->loadAction(stream, *this);
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


ObjectBase *DBlockHeader::getObject(size_t id)
{
    auto iter = mObjects.find(id);
    if(iter == mObjects.end())
        return nullptr;
    return *iter;
}


size_t DBlockHeader::getObjectByTexture(size_t texid) const
{
    for(const ref_ptr<ObjectBase> &obj : mObjects)
    {
        if(obj->mType == ObjectType_Flat)
        {
            FlatObject *flat = static_cast<FlatObject*>(obj.get());
            if(flat->mTexture == texid) return flat->mId;
        }
    }
    Log::get().stream(Log::Log::Level_Error)<< "Failed to find Flat with texture 0x"<<std::setfill('0')<<std::setw(4)<<std::hex<<texid;
    return ~static_cast<size_t>(0);
}


void DBlockHeader::activate(size_t id)
{
    ObjectBase *base = getObject(id);
    if(!base || !(base->mActionFlags&ActionFlag_Activatable))
    {
        if(!base->mActive && base->mType == ObjectType_Model)
        {
            ModelObject *model = static_cast<ModelObject*>(base);
            // TODO: There's likely a flag on the object that specifies a door
            // that can't be directly activated, e.g. the protected door in
            // Shedungent.
            if(model->mModelData[5] == 'D' && model->mModelData[6] == 'O' && model->mModelData[7] == 'R')
            {
                ref_ptr<ActionRotate> action(new ActionRotate(nullptr));
                action->load({{ActionRotate::Axis_Y, 24, 0, 0, 2}});

                mActiveObjects.push_back(std::make_pair(action, base));
                base->mActive = true;
            }
        }
        return;
    }

    // Make sure no object in this chain is still active
    ObjectBase *check = base;
    while(check != nullptr)
    {
        if(check->mActive)
            break;
        if(check->mAction)
            check = check->mAction->mLink;
        else
            check = nullptr;
    }
    if(check != nullptr)
        return;

    while(base != nullptr && base->mAction)
    {
        mActiveObjects.push_back(std::make_pair(base->mAction, base));
        base->mActive = true;
        base = base->mAction->mLink;
    }
}


void DBlockHeader::update(float timediff)
{
    auto iter = mActiveObjects.begin();
    while(iter != mActiveObjects.end())
    {
        if(!iter->first->update(iter->second, timediff))
        {
            iter->second->mActive = false;
            iter = mActiveObjects.erase(iter);
        }
        else
            ++iter;
    }
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
