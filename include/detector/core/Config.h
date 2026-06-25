#pragma once

#include <map>
#include <string>

namespace detector {

#define S_Segment_Common "[Common]"
#define S_Segment_Logger "[Logger]"

// Simple singleton reader for the INI-style `.cfg` file. Values are stored as
// strings and exposed through typed getters (string / int / float / bool).
// Lines beginning with '#' are comments; `[Common]` / `[Logger]` switch the
// active section.
class CfgLoader {
public:
    enum {
        Segment_COMMON = 0,
        Segment_LOGGER = 1
    };

    static CfgLoader* instance();

    int init(std::string const& cfgPath);
    void fini();
    void reset();
    // For debugging: dump all loaded key/value pairs.
    std::string toString();

    int getCfgByName(std::string& value, std::string const& name, int segment = Segment_COMMON);
    int getCfgByName(int& value, std::string const& name, int segment = Segment_COMMON);
    int getCfgByName(float& value, std::string const& name, int segment = Segment_COMMON);
    int getCfgByName(bool& value, std::string const& name, int segment = Segment_COMMON);

private:
    CfgLoader() = default;

    CfgLoader(CfgLoader const&) = delete;
    CfgLoader& operator=(CfgLoader const&) = delete;

    std::string m_commentChar = "#";
    std::string m_kvSplit = "=";

    std::map<std::string, std::string> m_commonMap;
    std::map<std::string, std::string> m_loggerMap;
};

} // namespace detector
