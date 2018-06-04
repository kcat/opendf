
#include "vertexbuffer.h"

#include <MYGUI/MyGUI_Diagnostic.h>
#include <MYGUI/MyGUI_VertexData.h>

#include <osg/BufferObject>
#include <osg/Array>

#include "diagnostic.h"


namespace MyGUI_OSG
{

VertexBuffer::VertexBuffer()
  : mNeedVertexCount(0)
{
}

VertexBuffer::~VertexBuffer()
{
    destroy();
}

void VertexBuffer::setVertexCount(size_t count)
{
    if(count == mNeedVertexCount)
        return;

    mNeedVertexCount = count;
    destroy();
    create();
}

size_t VertexBuffer::getVertexCount()
{
    return mNeedVertexCount;
}

MyGUI::Vertex *VertexBuffer::lock()
{
    MYGUI_PLATFORM_ASSERT(mBuffer.valid(), "Vertex buffer is not created");

    // NOTE: Unfortunately, MyGUI wants the VBO data to be interleaved as a
    // MyGUI::Vertex structure. However, OSG uses non-interleaved elements, so
    // we have to give back a "temporary" structure array then copy it to the
    // actual VBO arrays on unlock. This is extra unfortunate since the VBO may
    // be backed by VRAM, meaning we could have up to 3 copies of the data
    // (which we'll need to keep for VBOs that are continually updated).
    mLockedData.resize(mNeedVertexCount, MyGUI::Vertex());
    return mLockedData.data();
}

void VertexBuffer::unlock()
{
    osg::Vec3 *vec = &mPositionArray->front();
    for(const MyGUI::Vertex &elem : mLockedData)
    {
        vec->set(elem.x, elem.y, elem.z);
        ++vec;
    }
    osg::Vec4ub *clr = &mColorArray->front();
    for(const MyGUI::Vertex &elem : mLockedData)
    {
        union {
            MyGUI::uint32 ui;
            unsigned char ub4[4];
        } val = { elem.colour };
        clr->set(val.ub4[0], val.ub4[1], val.ub4[2], val.ub4[3]);
        ++clr;
    }
    osg::Vec2 *crds = &mTexCoordArray->front();
    for(const MyGUI::Vertex &elem : mLockedData)
    {
        crds->set(elem.u, elem.v);
        ++crds;
    }

    mBuffer->dirty();
}

void VertexBuffer::destroy()
{
    mBuffer = nullptr;
    mPositionArray = nullptr;
    mColorArray = nullptr;
    mTexCoordArray = nullptr;
    std::vector<MyGUI::Vertex>().swap(mLockedData);
}

void VertexBuffer::create()
{
    MYGUI_PLATFORM_ASSERT(!mBuffer.valid(), "Vertex buffer already exist");

    mPositionArray = new osg::Vec3Array(mNeedVertexCount);
    mColorArray = new osg::Vec4ubArray(mNeedVertexCount);
    mTexCoordArray = new osg::Vec2Array(mNeedVertexCount);
    mColorArray->setNormalize(true);

    mBuffer = new osg::VertexBufferObject;
    mBuffer->setDataVariance(osg::Object::DYNAMIC);
    mBuffer->setUsage(GL_STREAM_DRAW);
    mBuffer->setArray(0, mPositionArray.get());
    mBuffer->setArray(1, mColorArray.get());
    mBuffer->setArray(2, mTexCoordArray.get());
}

} // namespace MyGUI_OSG
