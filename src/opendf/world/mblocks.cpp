
#include "mblocks.hpp"

#include <iostream>
#include <iomanip>

#include <osg/Group>
#include <osg/MatrixTransform>

#include "components/vfs/manager.hpp"
#include "components/resource/meshmanager.hpp"

#include "render/renderer.hpp"
#include "world.hpp"
#include "log.hpp"


namespace DF
{

struct MSection3 : public MObjectBase {
    uint16_t mUnknown1;
    uint16_t mUnknown2;

    void load(std::istream &stream);
};

struct MDoor : public MObjectBase {
    uint16_t mUnknown1;
    int16_t mRotation;
    uint16_t mUnknown2;
    uint8_t mNullValue;

    void load(std::istream &stream);
};

struct MFlat : public MObjectBase {
    uint16_t mTexture;
    uint16_t mUnknown;
    uint8_t mFlags;

    void load(std::istream &stream);
    void allocate(osg::Group *root);

    virtual void print(std::ostream &stream) const;
};

struct MPerson : public MObjectBase {
    uint16_t mTexture;
    uint16_t mFactionId;

    void load(std::istream &stream);
};

struct MModel : public MObjectBase {
    uint32_t mModelIdx; /* le16*100 + byte */
    uint8_t  mUnknown1;
    uint32_t mUnknown2;
    uint32_t mUnknown3;
    uint32_t mUnknown4;
    uint32_t mNullValue1;
    uint32_t mNullValue2;
    int32_t  mUnknownX, mUnknownY, mUnknownZ;
    //int32_t  mXPos, mYPos, mZPos;
    uint32_t mNullValue3;
    int16_t  mYRotation;
    uint16_t mUnknown5;
    uint32_t mUnknown6;
    uint32_t mUnknown8;
    uint16_t mNullValue4;

    void load(std::istream &stream);
    void allocate(osg::Group *root);

    virtual void print(std::ostream &stream) const;
};


void MObjectBase::load(std::istream &stream)
{
    mXPos = VFS::read_le32(stream);
    mYPos = VFS::read_le32(stream);
    mZPos = VFS::read_le32(stream);
}

void MObjectBase::print(std::ostream &stream) const
{
    stream<< "Pos: "<<mXPos<<" "<<mYPos<<" "<<mZPos<<"\n";
}

void MSection3::load(std::istream &stream)
{
    MObjectBase::load(stream);
    mUnknown1 = VFS::read_le16(stream);
    mUnknown2 = VFS::read_le16(stream);
}

void MDoor::load(std::istream &stream)
{
    MObjectBase::load(stream);
    mUnknown1 = VFS::read_le16(stream);
    mRotation = VFS::read_le16(stream);
    mUnknown2 = VFS::read_le16(stream);
    mNullValue = stream.get();
}


void MFlat::load(std::istream &stream)
{
    MObjectBase::load(stream);
    mTexture = VFS::read_le16(stream);
    mUnknown = VFS::read_le16(stream);
    mFlags = stream.get();
}

void MFlat::allocate(osg::Group *root)
{
    osg::ref_ptr<osg::MatrixTransform> node = new osg::MatrixTransform();
    node->setNodeMask(Renderer::Mask_Flat);
    node->setUserData(new ObjectRef(mId));
    node->addChild(Resource::MeshManager::get().loadFlat(mTexture, false));
    root->addChild(node);

    Renderer::get().setNode(mId, node);
    Placeable::get().setPoint(mId, osg::Vec3f(mXPos, mYPos, mZPos));
}

void MFlat::print(std::ostream &stream) const
{
    DF::MObjectBase::print(stream);
    stream<< "Texture: 0x"<<std::hex<<std::setw(4)<<mTexture<<std::dec<<std::setw(0)<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(4)<<mUnknown<<std::dec<<std::setw(0)<<"\n";
    stream<< "Flags: 0x"<<std::hex<<std::setw(2)<<(int)mFlags<<std::setw(0)<<std::dec<<"\n";
}


void MPerson::load(std::istream &stream)
{
    MObjectBase::load(stream);
    mTexture = VFS::read_le16(stream);
    mFactionId = VFS::read_le16(stream);
}


void MModel::load(std::istream &stream)
{
    mModelIdx  = (int)VFS::read_le16(stream) * 100;
    mModelIdx += stream.get();
    mUnknown1 = stream.get();
    mUnknown2 = VFS::read_le32(stream);
    mUnknown3 = VFS::read_le32(stream);
    mUnknown4 = VFS::read_le32(stream);
    mNullValue1 = VFS::read_le32(stream);
    mNullValue2 = VFS::read_le32(stream);
    mUnknownX = VFS::read_le32(stream);
    mUnknownY = VFS::read_le32(stream);
    mUnknownZ = VFS::read_le32(stream);
    mXPos = VFS::read_le32(stream);
    mYPos = VFS::read_le32(stream);
    mZPos = VFS::read_le32(stream);
    mNullValue3 = VFS::read_le32(stream);
    mYRotation = VFS::read_le16(stream);
    mUnknown5 = VFS::read_le16(stream);
    mUnknown6 = VFS::read_le32(stream);
    mUnknown8 = VFS::read_le32(stream);
    mNullValue4 = VFS::read_le16(stream);
}

void MModel::allocate(osg::Group *root)
{
    osg::ref_ptr<osg::MatrixTransform> node = new osg::MatrixTransform();
    node->setNodeMask(Renderer::Mask_Static);
    node->setUserData(new ObjectRef(mId));
    node->addChild(Resource::MeshManager::get().get(mModelIdx));
    root->addChild(node);

    Renderer::get().setNode(mId, node);
    Placeable::get().setPos(mId, osg::Vec3f(mXPos, mYPos, mZPos), osg::Vec3f(0.0f, mYRotation, 0.0f));
}

void MModel::print(std::ostream &stream) const
{
    DF::MObjectBase::print(stream);
    stream<< "ModelIdx: "<<mModelIdx<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(2)<<(int)mUnknown1<<std::dec<<std::setw(0)<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown2<<std::dec<<std::setw(0)<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown3<<std::dec<<std::setw(0)<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown4<<std::dec<<std::setw(0)<<"\n";
    stream<< "Null: "<<mNullValue1<<"\n";
    stream<< "Null: "<<mNullValue2<<"\n";
    stream<< "UnknownPos: "<<mUnknownX<<" "<<mUnknownY<<" "<<mUnknownZ<<"\n";
    stream<< "Null: "<<mNullValue3<<"\n";
    stream<< "YRotation: "<<mYRotation<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(4)<<mUnknown5<<std::dec<<std::setw(0)<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown6<<std::dec<<std::setw(0)<<"\n";
    stream<< "Unknown: 0x"<<std::hex<<std::setw(8)<<mUnknown8<<std::dec<<std::setw(0)<<"\n";
    stream<< "Null: "<<mNullValue4<<"\n";
}


MBlock::MBlock() { }
MBlock::~MBlock()
{ deallocate(); }

void MBlock::load(std::istream &stream, size_t blockid, int x, int z, int yrot, osg::Group *root)
{
    osg::Matrix mat(osg::Matrix::rotate(
        -yrot*3.14159f/1024.0f, osg::Vec3f(0.0f, 1.0f, 0.0f)
    ));
    mat.postMultTranslate(osg::Vec3(x, 0.0f, -z));
    mBaseNode = new osg::MatrixTransform(mat);

    mModelCount = stream.get();
    mFlatCount = stream.get();
    mSection3Count = stream.get();
    mPersonCount = stream.get();
    mDoorCount = stream.get();
    mUnknown1 = VFS::read_le16(stream);
    mUnknown2 = VFS::read_le16(stream);
    mUnknown3 = VFS::read_le16(stream);
    mUnknown4 = VFS::read_le16(stream);
    mUnknown5 = VFS::read_le16(stream);
    mUnknown6 = VFS::read_le16(stream);

    mModels.reserve(mModelCount);
    for(size_t i = 0;i < mModelCount;++i)
    {
        MModel &model = mModels[blockid | i];
        model.mId = blockid | i;
        model.load(stream);
    }
    mFlats.reserve(mFlatCount);
    for(size_t i = 0;i < mFlatCount;++i)
    {
        MFlat &flat = mFlats[blockid | (mModelCount+i)];
        flat.mId = blockid | (mModelCount+i);
        flat.load(stream);
    }
    mSection3s.resize(mSection3Count);
    for(MSection3 &sec3 : mSection3s)
        sec3.load(stream);
    mNpcs.resize(mPersonCount);
    for(MPerson &npc : mNpcs)
        npc.load(stream);
    mDoors.resize(mDoorCount);
    for(MDoor &door : mDoors)
        door.load(stream);

    root->addChild(mBaseNode);
}

void MBlock::allocate()
{
    for(MModel &model : mModels)
        model.allocate(mBaseNode);
    for(MFlat &flat : mFlats)
        flat.allocate(mBaseNode);
}

void MBlock::deallocate()
{
    if(mBaseNode)
        mBaseNode->removeChildren(0, mBaseNode->getNumChildren());
    if(!mModels.empty())
    {
        Renderer::get().remove(&*mModels.getIdList(), mModels.size());
        Placeable::get().deallocate(&*mModels.getIdList(), mModels.size());
    }
    if(!mFlats.empty())
    {
        Renderer::get().remove(&*mFlats.getIdList(), mFlats.size());
        Placeable::get().deallocate(&*mFlats.getIdList(), mFlats.size());
    }
}


MObjectBase *MBlock::getObject(size_t id)
{
    if(mModels.exists(id))
        return &mModels[id];
    return &mFlats[id];
}


void MBlockPosition::load(std::istream &stream)
{
    mUnknown1 = VFS::read_le32(stream);
    mUnknown2 = VFS::read_le32(stream);
    mX = VFS::read_le32(stream);
    mZ = VFS::read_le32(stream);
    mYRot = VFS::read_le32(stream);
}


MBlockHeader::MBlockHeader() { }
MBlockHeader::~MBlockHeader()
{
    detachNode();
    deallocate();
}

void MBlockHeader::deallocate()
{
    if(!mModels.empty())
    {
        Renderer::get().remove(&*mModels.getIdList(), mModels.size());
        Placeable::get().deallocate(&*mModels.getIdList(), mModels.size());
    }
    if(!mFlats.empty())
    {
        Renderer::get().remove(&*mFlats.getIdList(), mFlats.size());
        Placeable::get().deallocate(&*mFlats.getIdList(), mFlats.size());
    }
    if(!mScenery.empty())
    {
        Renderer::get().remove(&*mScenery.getIdList(), mScenery.size());
        Placeable::get().deallocate(&*mScenery.getIdList(), mScenery.size());
    }
}


void MBlockHeader::load(std::istream &stream, uint8_t climate, size_t blockid, float x, float z, osg::Group *root)
{
    mBaseNode = new osg::MatrixTransform(osg::Matrix::translate(x, 0.0f, z));

    size_t texfile = 0;
    if(climate == 223) texfile = 502<<7;
    else if(climate == 224) texfile = 503<<7;
    else if(climate == 225) texfile = 503<<7;
    else if(climate == 226) texfile = 510<<7;
    else if(climate == 227) texfile = 500<<7;
    else if(climate == 228) texfile = 502<<7;
    else if(climate == 229) texfile = 503<<7;
    else if(climate == 230) texfile = 504<<7;
    else if(climate == 231) texfile = 508<<7;
    else if(climate == 232) texfile = 508<<7;

    mBlockCount = stream.get();
    mModelCount = stream.get();
    mFlatCount = stream.get();

    for(MBlockPosition &blockpos : mBlockPositions)
        blockpos.load(stream);
    for(ExteriorBuilding &building : mBuildings)
    {
        building.mNameSeed = VFS::read_le16(stream);
        building.mNullValue1 = VFS::read_le32(stream);
        building.mNullValue2 = VFS::read_le32(stream);
        building.mNullValue3 = VFS::read_le32(stream);
        building.mNullValue4 = VFS::read_le32(stream);
        building.mFactionId = VFS::read_le16(stream);
        building.mSector = VFS::read_le16(stream);
        building.mLocationId = VFS::read_le16(stream);
        building.mBuildingType = stream.get();
        building.mQuality = stream.get();
    }
    for(uint32_t &unknown : mUnknown1)
        unknown = VFS::read_le32(stream);
    for(uint32_t &size : mBlockSizes)
        size = VFS::read_le32(stream);

    stream.read(reinterpret_cast<char*>(mUnknown2.data()), mUnknown2.size());
    stream.read(reinterpret_cast<char*>(mGroundTexture.data()), mGroundTexture.size());
    stream.read(reinterpret_cast<char*>(mGroundScenery.data()), mGroundScenery.size());
    stream.read(reinterpret_cast<char*>(mAutomap.data()), mAutomap.size());

    // Unused list? An array of 33 8.3 filenames are here...
    stream.ignore(429);

    mExteriorBlocks.resize(mBlockCount);
    mInteriorBlocks.resize(mBlockCount);
    for(size_t i = 0;i < mBlockCount;++i)
    {
        size_t pos = stream.tellg();
        mExteriorBlocks[i].load(stream, blockid | (i<<17) | 0x00000,
                                mBlockPositions[i].mX, mBlockPositions[i].mZ,
                                mBlockPositions[i].mYRot, mBaseNode);

        mInteriorBlocks[i].load(stream, blockid | (i<<17) | 0x10000,
                                mBlockPositions[i].mX, mBlockPositions[i].mZ,
                                mBlockPositions[i].mYRot, mBaseNode);
        stream.seekg(pos + mBlockSizes[i]);
    }

    mModels.reserve(mModelCount);
    for(size_t i = 0;i < mModelCount;++i)
    {
        MModel &model = mModels[blockid | 0x00ff0000 | i];
        model.mId = blockid | 0x00ff0000 | i;
        model.load(stream);
    }
    mFlats.reserve(mFlatCount);
    for(size_t i = 0;i < mFlatCount;++i)
    {
        MFlat &flat = mFlats[blockid | 0x00ff0000 | (mModelCount+i)];
        flat.mId = blockid | 0x00ff0000 | (mModelCount+i);
        flat.load(stream);
    }
    for(size_t i = 0;i < mGroundScenery.size();++i)
    {
        if(mGroundScenery[i] == 0xff)
            continue;

        MFlat &flat = mScenery[blockid | 0x00ff0000 | (mModelCount+mFlatCount+i)];
        flat.mId = blockid | 0x00ff0000 | (mModelCount+mFlatCount+i);
        flat.mXPos = (i%16) * 256.0f;
        flat.mYPos = 0.0f;
        flat.mZPos = (i/16) * -256.0f;
        flat.mTexture = texfile | (mGroundScenery[i]>>2);
        flat.mUnknown = 0;
        flat.mFlags = 0;
    }

    root->addChild(mBaseNode);

    for(size_t i = 0;i < mBlockCount;++i)
        mExteriorBlocks[i].allocate();
    for(MModel &model : mModels)
        model.allocate(mBaseNode);
    for(MFlat &flat : mFlats)
        flat.allocate(mBaseNode);
    for(MFlat &flat : mScenery)
        flat.allocate(mBaseNode);
}

void MBlockHeader::detachNode()
{
    if(!mBaseNode) return;
    while(mBaseNode->getNumParents() > 0)
    {
        osg::Group *parent = mBaseNode->getParent(0);
        parent->removeChild(mBaseNode);
    }
}


MObjectBase *MBlockHeader::getObject(size_t id)
{
    if(((id>>16)&0xff) == 0xff)
    {
        if(mModels.exists(id))
            return &mModels[id];
        return &mFlats[id];
    }
    if(!(id&0x10000))
        return mExteriorBlocks.at((id>>17)&0x7f).getObject(id);
    else
        return mInteriorBlocks.at((id>>17)&0x7f).getObject(id);
}

size_t MBlockHeader::getObjectByTexture(size_t texid) const
{
    for(const MFlat &flat : mFlats)
    {
        if(flat.mTexture == texid)
            return flat.mId;
    }
    Log::get().stream(Log::Level_Error)<< "Failed to find Flat with texture 0x"<<std::setfill('0')<<std::setw(4)<<std::hex<<texid;
    return ~static_cast<size_t>(0);
}

} // namespace DF
