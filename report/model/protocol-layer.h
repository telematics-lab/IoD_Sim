#pragma once
#include <ns3/object.h>

namespace ns3 {

class ProtocolLayer : public Object
{
public:
    /*
     * Pure virtual declarations
     * or very common methods/properties
     * can be defined here
     */

    /**
     * Write Stack Layer report data to a XML file with a given handler
     *
     * \param handle the XML handler to write data on
     */
    virtual void Write (xmlTextWriterPtr handle) = 0;
protected:
    /**
     * Object internal initialization
     */
    virtual void DoInitialize () { Object::DoInitialize (); };
};

} // namespace ns3
