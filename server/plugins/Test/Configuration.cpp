#include "LightBird.h"
#include "Configuration.h"

Configuration::Configuration(LightBird::IApi &api)
    : ITest(api)
{
}

Configuration::~Configuration()
{
}

unsigned int    Configuration::run()
{
    LightBird::IConfiguration   &c = _api.configuration();

    _log.debug("Running the unit tests of the configuration...", "Configuration", "run");
    // Builds the test tree
    c.remove("unitTests");
    c.set("unitTests/node1", "node1A1");
    c.set("unitTests/node1[1]", "node1B1");
    c.set("unitTests/node2[0]", "node2A1");
    c.set("unitTests/node1[2]", "node1C1");
    c.set("unitTests/node2[1]", "node2B1");
    c.set("unitTests/node1[6]", "node1D1");
    c.set("unitTests/node1[4]", "node1E1");
    c.set("unitTests/node1[2]/node3", "node3A1");
    c.set("unitTests/node1[2]/node3[1]", "node3B1");
    c.set("unitTests/node1[2]/node3[2]", "node3C1");
    c.set("unitTests/node1[2]/node3.attr1", "node3D1");
    c.set("unitTests/node1[2]/node3.attr2", "node3E1");
    c.set("unitTests/node1[2]/node3.attr1", "node3D2");
    c.set("unitTests/node1[2]/node3[2].attr3", "node3F1");
    c.set("unitTests/node1[2]/node3[1].attr4", "node3G1");
    c.set("unitTests/node1[2].attr5", "node1F1");
    c.set("unitTests/node1[3].attr5", "node1F2");
    c.set("unitTests/node1[2]/node3", "node3A2");
    c.set("unitTests/node1[2]/node3[1]", "node3B2");
    c.set("unitTests/node1[2]/node3[2]", "node3C2");
    c.set("unitTests/node1", "node1A2");
    c.set("unitTests/node1[2]", "node1C2");
    c.set("unitTests/node1[4]", "node1E2");
    try
    {
        // Tests if the tree has been correctly built
        ASSERT(c.get("unitTests/node1[1]") == "node1B1");
        ASSERT(c.get("unitTests/node2[0]") == "node2A1");
        ASSERT(c.get("unitTests/node2[1]") == "node2B1");
        ASSERT(c.get("unitTests/node1[3]") == "node1D1");
        ASSERT(c.get("unitTests/node1[2]/node3.attr2") == "node3E1");
        ASSERT(c.get("unitTests/node1[2]/node3.attr1") == "node3D2");
        ASSERT(c.get("unitTests/node1[2]/node3[2].attr3") == "node3F1");
        ASSERT(c.get("unitTests/node1[2]/node3[1].attr4") == "node3G1");
        ASSERT(c.get("unitTests/node1[2].attr5") == "node1F1");
        ASSERT(c.get("unitTests/node1[3].attr5") == "node1F2");
        ASSERT(c.get("unitTests/node1[2]/node3") == "node3A2");
        ASSERT(c.get("unitTests/node1[2]/node3[1]") == "node3B2");
        ASSERT(c.get("unitTests/node1[2]/node3[2]") == "node3C2");
        ASSERT(c.get("unitTests/node1") == "node1A2");
        ASSERT(c.get("unitTests/node1[2]") == "node1C2");
        ASSERT(c.get("unitTests/node1[4]") == "node1E2");
        ASSERT(c.get("unitTests/node1[78]") == "");
        ASSERT(c.get("unitTests/node1[-1]") == "node1A2");
        ASSERT(c.get("unitTests/node1[egshuge]") == "node1A2");
        ASSERT(c.get("unitTests/node1[]") == "node1A2");
        ASSERT(c.get("unitTests/node4") == "");
        ASSERT(c.get("unitTests/node[3].attr6") == "");
        // Tests the count function
        ASSERT(c.count("unitTests") == 1);
        ASSERT(c.count("unitTests/node1") == 5);
        ASSERT(c.count("unitTests/node2") == 2);
        ASSERT(c.count("unitTests/node1[2]/node3") == 3);
        ASSERT(c.count("") == 0);
        ASSERT(c.count("gesgsho") == 0);
        ASSERT(c.count("unitTests/gsegesi") == 0);
        c.remove("unitTests");
    }
    catch (unsigned int line)
    {
        _log.debug("Unit tests of the configuration failed!", Properties("line", line).toMap(), "Configuration", "run");
        return (line);
    }
    _log.debug("Unit tests of the configuration successful!", "Configuration", "run");
    return (0);
}
