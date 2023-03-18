#ifndef COMPONENTS_MYGUI_OSG_VERTEXBUFFER_H
#define COMPONENTS_MYGUI_OSG_VERTEXBUFFER_H

#include <vector>
#include <cstddef>

#include <MYGUI/MyGUI_IVertexBuffer.h>

#include <osg/ref_ptr>
#include <osg/BufferObject>
#include <osg/Array>


namespace MyGUI_OSG
{

class VertexBuffer : public MyGUI::IVertexBuffer {
    osg::ref_ptr<osg::VertexBufferObject> mBuffer;
    osg::ref_ptr<osg::Vec3Array> mPositionArray;
    osg::ref_ptr<osg::Vec4ubArray> mColorArray;
    osg::ref_ptr<osg::Vec2Array> mTexCoordArray;
    std::vector<MyGUI::Vertex> mLockedData;

    size_t mNeedVertexCount;

public:
    VertexBuffer();
    virtual ~VertexBuffer();

    virtual void setVertexCount(size_t count);
    virtual size_t getVertexCount() const;

    virtual MyGUI::Vertex *lock();
    virtual void unlock();

/*internal:*/
    void destroy();
    void create();

    osg::VertexBufferObject *getBuffer() const { return mBuffer.get(); }
};

} // namespace MyGUI_OSG

#endif /* COMPONENTS_MYGUI_OSG_VERTEXBUFFER_H */
