#ifndef COMPONENTS_MYGUI_OSG_DATAMANAGER_H
#define COMPONENTS_MYGUI_OSG_DATAMANAGER_H

#include <MYGUI/MyGUI_DataManager.h>


namespace MyGUI_OSG
{

// Doesn't really use OSG, since OSG doesn't have a proper VFS.
class DataManager : public MyGUI::DataManager {
public:

    /** Get data stream from specified resource name.
        @param _name Resource name (usually file name).
    */
    virtual MyGUI::IDataStream* getData(const std::string &_name) final;

    /** Free data stream.
        @param _data Data stream.
    */
    virtual void freeData(MyGUI::IDataStream *_data) final;

    /** Is data with specified name exist.
        @param _name Resource name.
    */
    virtual bool isDataExist(const std::string &_name) final;

    /** Get all data names with names that matches pattern.
        @param _pattern Pattern to match (for example "*.layout").
    */
    virtual const MyGUI::VectorString &getDataListNames(const std::string &_pattern) final;

    /** Get full path to data.
        @param _name Resource name.
        @return Return full path to specified data.
        For example getDataPath("My.layout") might return "C:\path\to\project\data\My.layout"
    */
    virtual const std::string &getDataPath(const std::string &_name) final;
};

} // namespace MyGUI_OSG

#endif /* COMPONENTS_MYGUI_OSG_DATAMANAGER_H */
