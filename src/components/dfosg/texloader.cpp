
#include "texloader.hpp"

#include <vector>
#include <sstream>
#include <iomanip>

#include <osg/Image>

#include "components/vfs/manager.hpp"


namespace
{

class TexEntryHeader {
    uint8_t mUnknown1;
    uint8_t mColor;
    uint32_t mOffset;
    uint16_t mUnknown2;
    uint32_t mUnknown3;
    uint16_t mNullValue[2];

public:
    void load(std::istream &stream)
    {
        mUnknown1 = stream.get();
        mColor = stream.get();
        mOffset = VFS::read_le32(stream);
        mUnknown2 = VFS::read_le16(stream);
        mUnknown3 = VFS::read_le32(stream);
        mNullValue[0] = VFS::read_le32(stream);
        mNullValue[1] = VFS::read_le32(stream);
    }

    uint8_t getColor() const { return mColor; }
    uint32_t getOffset() const { return mOffset; }
};

class TexFileHeader {
    uint16_t mImageCount;
    std::array<char,24> mName;

    std::vector<TexEntryHeader> mHeaders;

public:
    void load(std::istream &stream)
    {
        mImageCount = VFS::read_le16(stream);
        stream.read(mName.data(), mName.size());

        mHeaders.resize(mImageCount);
        for(TexEntryHeader &hdr : mHeaders)
            hdr.load(stream);
    }

    uint16_t getImageCount() const { return mImageCount; }
    const std::vector<TexEntryHeader> &getHeaders() const { return mHeaders; }
};

class TexHeader {
    int16_t mOffsetX;
    int16_t mOffsetY;
    uint16_t mWidth;
    uint16_t mHeight;
    uint16_t mCompression;
    uint32_t mRecordSize;
    uint32_t mDataOffset;
    uint16_t mIsNormal;
    uint16_t mFrameCount;
    uint16_t mUnknown;
    int16_t mXScale;
    int16_t mYScale;

public:
    //static const uint16_t sUncompressed  = 0x0000;
    static const uint16_t sRleCompressed = 0x0002;
    static const uint16_t sImageRle  = 0x0108;
    static const uint16_t sRecordRle = 0x1108;
    void load(std::istream &stream)
    {
        mOffsetX = VFS::read_le16(stream);
        mOffsetY = VFS::read_le16(stream);
        mWidth = VFS::read_le16(stream);
        mHeight = VFS::read_le16(stream);
        mCompression = VFS::read_le16(stream);
        mRecordSize = VFS::read_le32(stream);
        mDataOffset = VFS::read_le32(stream);
        mIsNormal = VFS::read_le16(stream);
        mFrameCount = VFS::read_le16(stream);
        mUnknown = VFS::read_le16(stream);
        mXScale = VFS::read_le16(stream);
        mYScale = VFS::read_le16(stream);
    }

    int16_t getXOffset() const { return mOffsetX; }
    int16_t getYOffset() const { return mOffsetY; }
    uint16_t getWidth() const { return mWidth; }
    uint16_t getHeight() const { return mHeight; }
    uint16_t getCompression() const { return mCompression; }
    uint32_t getDataOffset() const { return mDataOffset; }
    uint16_t getIsNormal() const { return mIsNormal; }
    uint16_t getFrameCount() const { return mFrameCount; }
    int16_t getXScale() const { return mXScale; }
    int16_t getYScale() const { return mYScale; }
};

} // namespace


namespace DFOSG
{

TexLoader TexLoader::sLoader;


TexLoader::TexLoader()
{
}

TexLoader::~TexLoader()
{
}


osg::Image *TexLoader::createDummyImage()
{
    osg::Image *image = new osg::Image();

    // Yellow/black diagonal stripes
    image->allocateImage(2, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    unsigned char *dst = image->data(0, 0);
    *(dst++) = 255;
    *(dst++) = 255;
    *(dst++) = 0;
    *(dst++) = 255;
    *(dst++) = 0;
    *(dst++) = 0;
    *(dst++) = 0;
    *(dst++) = 255;
    dst = image->data(0, 1);
    *(dst++) = 0;
    *(dst++) = 0;
    *(dst++) = 0;
    *(dst++) = 255;
    *(dst++) = 255;
    *(dst++) = 255;
    *(dst++) = 0;
    *(dst++) = 255;

    return image;
}


osg::Image *TexLoader::loadUncompressedSingle(size_t width, size_t height, const Resource::Palette &palette, std::istream &stream)
{
    osg::Image *image = new osg::Image();
    image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);

    for(size_t y = 0;y < height;++y)
    {
        std::array<uint8_t,256> line;
        stream.read(reinterpret_cast<char*>(line.data()), line.size());

        unsigned char *dst = image->data(0, y);
        for(size_t x = 0;x < width;++x)
        {
            const Resource::PaletteEntry &color = palette[line[x]];
            *(dst++) = color.r;
            *(dst++) = color.g;
            *(dst++) = color.b;
            *(dst++) = (line[x]==0) ? 0 : 255;
        }
    }

    return image;
}

void TexLoader::loadUncompressedMulti(osg::Image *image, const Resource::Palette &palette, std::istream &stream)
{
    size_t width = VFS::read_le16(stream);
    size_t height = VFS::read_le16(stream);

    for(uint32_t y = 0;y < height && stream;++y)
    {
        bool isZero = true;
        uint8_t c = stream.get();

        uint32_t x = 0;
        do {
            if(isZero)
            {
                for(uint32_t i = 0;i < c;++i)
                {
                    unsigned char *dst = image->data(x++, y);
                    *(dst++) = palette[0].r;
                    *(dst++) = palette[0].g;
                    *(dst++) = palette[0].b;
                    *(dst++) = 0;
                }
            }
            else for(uint32_t i = 0;i < c;++i)
            {
                unsigned char *dst = image->data(x++, y);
                uint8_t idx = stream.get();
                *(dst++) = palette[idx].r;
                *(dst++) = palette[idx].g;
                *(dst++) = palette[idx].b;
                *(dst++) = (idx==0) ? 0 : 255;
            }
            if(x < width || (x >= width && isZero))
                c = stream.get();
            isZero = !isZero;
        } while(x < width && stream);
    }
}


ImagePtrArray TexLoader::load(size_t idx, int16_t *xoffset, int16_t *yoffset, int16_t *xscale, int16_t *yscale,
                              const Resource::Palette& palette)
{
    std::stringstream sstr; sstr.fill('0');
    sstr<<"TEXTURE."<<std::setw(3)<<(idx>>7);

    VFS::IStreamPtr stream = VFS::Manager::get().open(sstr.str());

    TexFileHeader hdr;
    hdr.load(*stream);

    const TexEntryHeader &entryhdr = hdr.getHeaders().at(idx&0x7f);
    if(entryhdr.getOffset() == 0)
    {
        osg::ref_ptr<osg::Image> image(new osg::Image());

        // Solid color "texture".
        image->allocateImage(1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        uint8_t idx = entryhdr.getColor();
        unsigned char *dst = image->data(0, 0);
        *(dst++) = palette[idx].r;
        *(dst++) = palette[idx].g;
        *(dst++) = palette[idx].b;
        *(dst++) = (idx==0) ? 0 : 255;

        std::vector<osg::ref_ptr<osg::Image>> images(1, image);
        return images;
    }

    if(!stream->seekg(entryhdr.getOffset()))
        throw std::runtime_error("Failed to seek to texture offset");

    TexHeader texhdr;
    texhdr.load(*stream);

    *xoffset = texhdr.getXOffset();
    *yoffset = texhdr.getYOffset();
    *xscale = texhdr.getXScale();
    *yscale = texhdr.getYScale();

    // Would be nice to load a multiframe texture as a 3D Image, but such an
    // image can't be properly loaded into a Texture2DArray (it wants to load
    // a 2D Image for each individual layer).
    ImagePtrArray images;
    if(texhdr.getFrameCount() == 0)
    {
        // Allocate a dummy image
        images.push_back(createDummyImage());
    }
    else if(texhdr.getFrameCount() == 1)
    {
        osg::ref_ptr<osg::Image> image;

        if(texhdr.getCompression() == texhdr.sRleCompressed)
            std::cerr<< "Unhandled RleCompressed compression type"<< std::endl;
        else if(texhdr.getCompression() == texhdr.sImageRle)
            std::cerr<< "Unhandled ImageRle compression type"<< std::endl;
        else if(texhdr.getCompression() == texhdr.sRecordRle)
            std::cerr<< "Unhandled RecordRle compression type"<< std::endl;
        else //if(texhdr.getCompression() == texhdr.sUncompressed)
        {
            if(!stream->seekg(entryhdr.getOffset() + texhdr.getDataOffset()))
                throw std::runtime_error("Failed to seek to image offset");

            image = loadUncompressedSingle(texhdr.getWidth(), texhdr.getHeight(), palette, *stream);
        }

        if(!image)
            image = createDummyImage();

        images.push_back(image);
    }
    else
    {
        if(texhdr.getCompression() == texhdr.sRleCompressed)
            std::cerr<< "Unhandled RleCompressed compression type"<< std::endl;
        else if(texhdr.getCompression() == texhdr.sImageRle)
            std::cerr<< "Unhandled ImageRle compression type"<< std::endl;
        else if(texhdr.getCompression() == texhdr.sRecordRle)
            std::cerr<< "Unhandled RecordRle compression type"<< std::endl;
        else //if(texhdr.getCompression() == texhdr.sUncompressed)
        {
            if(!stream->seekg(entryhdr.getOffset() + texhdr.getDataOffset()))
                throw std::runtime_error("Failed to seek to image offset");
            std::vector<uint32_t> offsets(texhdr.getFrameCount());
            for(uint32_t &offset : offsets)
                offset = VFS::read_le32(*stream);

            for(uint32_t offset : offsets)
            {
                if(!stream->seekg(entryhdr.getOffset() + texhdr.getDataOffset() + offset))
                    throw std::runtime_error("Failed to seek to frame offset");
                images.push_back(new osg::Image());

                osg::Image *image = images.back();
                image->allocateImage(texhdr.getWidth(), texhdr.getHeight(), 1,
                                     GL_RGBA, GL_UNSIGNED_BYTE);

                loadUncompressedMulti(image, palette, *stream);
            }
        }

        if(images.empty())
            images.push_back(createDummyImage());
    }

    return images;
}

} // namespace DFOSG
