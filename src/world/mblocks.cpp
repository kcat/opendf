
#include "mblocks.hpp"

#include <osg/Group>
#include <osg/MatrixTransform>

#include "components/vfs/manager.hpp"
#include "components/resource/meshmanager.hpp"

#include "iface.hpp"


namespace DF
{

void MObjectBase::load(std::istream &stream)
{
    mXPos = VFS::read_le32(stream);
    mYPos = VFS::read_le32(stream);
    mZPos = VFS::read_le32(stream);
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

void MFlat::buildNodes(osg::Group *root)
{
    osg::Matrix mat(osg::Matrix::translate(
        osg::Vec3(mXPos, mYPos, mZPos)
    ));

    mBaseNode = new osg::MatrixTransform(mat);
    mBaseNode->setNodeMask(WorldIface::Mask_Flat);
    //mBaseNode->setUserData(new ObjectRef(objid));
    mBaseNode->addChild(Resource::MeshManager::get().loadFlat(mTexture));
    root->addChild(mBaseNode);
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

void MModel::buildNodes(osg::Group *root)
{
    osg::Matrix mat(osg::Matrix::translate(
        osg::Vec3(mXPos, mYPos, mZPos)
    ));

    mBaseNode = new osg::MatrixTransform(mat);
    mBaseNode->setNodeMask(WorldIface::Mask_Static);
    //mBaseNode->setUserData(new ObjectRef(objid));
    mBaseNode->addChild(Resource::MeshManager::get().get(mModelIdx));
    root->addChild(mBaseNode);
}


void MBlock::load(std::istream &stream)
{
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

    mModels.resize(mModelCount);
    for(MModel &model : mModels)
        model.load(stream);
    mFlats.resize(mFlatCount);
    for(MFlat &flat : mFlats)
        flat.load(stream);
    mSection3s.resize(mSection3Count);
    for(MSection3 &sec3 : mSection3s)
        sec3.load(stream);
    mNpcs.resize(mPersonCount);
    for(MPerson &npc : mNpcs)
        npc.load(stream);
    mDoors.resize(mDoorCount);
    for(MDoor &door : mDoors)
        door.load(stream);
}

void MBlock::buildNodes(osg::Group *root, int x, int z, int yrot)
{
    if(!mBaseNode)
    {
        osg::Matrix mat(osg::Matrix::rotate(
            -yrot*3.14159f/1024.0f, osg::Vec3f(0.0f, 1.0f, 0.0f)
        ));
        mat.postMultTranslate(osg::Vec3(x, 0.0f, -z));

        mBaseNode = new osg::MatrixTransform(mat);
        for(size_t i = 0;i < mModelCount;++i)
            mModels[i].buildNodes(mBaseNode);
        for(size_t i = 0;i < mFlatCount;++i)
            mFlats[i].buildNodes(mBaseNode);
    }

    root->addChild(mBaseNode);
}


void MBlockPosition::load(std::istream &stream)
{
    mUnknown1 = VFS::read_le32(stream);
    mUnknown2 = VFS::read_le32(stream);
    mX = VFS::read_le32(stream);
    mZ = VFS::read_le32(stream);
    mYRot = VFS::read_le32(stream);
}


MBlockHeader::~MBlockHeader()
{
    detachNode();
}

void MBlockHeader::load(std::istream &stream)
{
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
    stream.read(reinterpret_cast<char*>(mUnknown3.data()), mUnknown3.size());
    stream.read(reinterpret_cast<char*>(mAutomap.data()), mAutomap.size());

    // Unused list? An array of 33 8.3 filenames are here...
    stream.ignore(429);

    mExteriorBlocks.resize(mBlockCount);
    mInteriorBlocks.resize(mBlockCount);
    for(size_t i = 0;i < mBlockCount;++i)
    {
        size_t pos = stream.tellg();
        mExteriorBlocks[i].load(stream);
        mInteriorBlocks[i].load(stream);
        stream.seekg(pos + mBlockSizes[i]);
    }

    mModels.resize(mModelCount);
    for(MModel &model : mModels)
        model.load(stream);
    mFlats.resize(mFlatCount);
    for(MFlat &flat : mFlats)
        flat.load(stream);
}

void MBlockHeader::buildNodes(osg::Group *root, int x, int z)
{
    if(!mBaseNode)
    {
        osg::Matrix mat(osg::Matrix::translate(
            osg::Vec3(x*4096.0f, 0.0f, z*4096.0f)
        ));

        mBaseNode = new osg::MatrixTransform(mat);
        for(size_t i = 0;i < mBlockCount;++i)
        {
            MBlock &block = mExteriorBlocks[i];
            block.buildNodes(mBaseNode, mBlockPositions[i].mX, mBlockPositions[i].mZ,
                             mBlockPositions[i].mYRot);
        }

        for(size_t i = 0;i < mModelCount;++i)
            mModels[i].buildNodes(mBaseNode);
        for(size_t i = 0;i < mFlatCount;++i)
            mFlats[i].buildNodes(mBaseNode);
    }

    root->addChild(mBaseNode);
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

} // namespace DF
