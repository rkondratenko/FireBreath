/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Oct 16, 2009
License:    Eclipse Public License - Version 1.0
            http://www.eclipse.org/legal/epl-v10.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#include "NPJavascriptObject.h"

using namespace FB::Npapi;

NPJavascriptObject *NPJavascriptObject::NewObject(NpapiBrowserHost *host, FB::JSAPI *api)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(host->CreateObject(&NPJavascriptObjectClass));

    obj->setAPI(api, host);
    return obj;
}

NPJavascriptObject::NPJavascriptObject(NPP npp)
    : m_api(NULL), m_browser(NULL), m_valid(true)
{
}

NPJavascriptObject::~NPJavascriptObject(void)
{
    m_api->detachEventSink(m_browser.ptr());
}

void NPJavascriptObject::setAPI(FB::JSAPI *api, NpapiBrowserHost *host)
{
    m_api = api;
    m_browser = host;
    
    m_api->attachEventSink(m_browser.ptr());
}

void NPJavascriptObject::Invalidate()
{
    m_valid = false;
    m_api->invalidate();
}

bool NPJavascriptObject::HasMethod(NPIdentifier name)
{
    return m_api->HasMethod(m_browser->StringFromIdentifier(name));
}

bool NPJavascriptObject::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    try {
        std::vector<FB::variant> vArgs;
        for (unsigned int i = 0; i < argCount; i++) {
            vArgs.push_back(m_browser->getVariant(&args[i]));
        }

        FB::variant ret = m_api->Invoke(m_browser->StringFromIdentifier(name), vArgs);
        m_browser->getNPVariant(result, ret);
        return true;
    } catch (script_error e) {
        m_browser->SetException(this, e.what());
        return false;
    }
}

bool NPJavascriptObject::InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    return Invoke(m_browser->GetStringIdentifier(""), args, argCount, result);
}

bool NPJavascriptObject::HasProperty(NPIdentifier name)
{
    std::string sName(m_browser->StringFromIdentifier(name));
    // We check for events of that name as well in order to allow setting of an event handler in the
    // old javascript style, i.e. plugin.onload = function() .....;
    return m_api->HasEvent(sName) || m_api->HasProperty(sName);
}

bool NPJavascriptObject::GetProperty(NPIdentifier name, NPVariant *result)
{
    try {
        std::string sName(m_browser->StringFromIdentifier(name));
        FB::variant res = m_api->GetProperty(sName);

        m_browser->getNPVariant(result, res);
        return true;
    } catch (script_error e) {
        return false;
    }
}

bool NPJavascriptObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
    try {
        FB::variant arg = m_browser->getVariant(value);

        m_api->SetProperty(m_browser->StringFromIdentifier(name), arg);
        return true;
    } catch (script_error e) {
        m_browser->SetException(this, e.what());
        return false;
    }}

bool NPJavascriptObject::RemoveProperty(NPIdentifier name)
{
    try {
        // TODO: add support for removing properties
        return false;
    } catch (script_error e) {
        m_browser->SetException(this, e.what());
        return false;
    }
}

bool NPJavascriptObject::Enumeration(NPIdentifier **value, uint32_t *count)
{
    try {
        // TODO: add support for enumerating members
        return false;
    } catch (script_error e) {
        m_browser->SetException(this, e.what());
        return false;
    }
}

bool NPJavascriptObject::Construct(const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    try {
        // TODO: add support for constructing
        return false;
    } catch (script_error e) {
        m_browser->SetException(this, e.what());
        return false;
    }
}


/****************************************************************************\
  These are the static functions given to the browser in the NPClass struct.
  You might look at these as the "entry points" for the NPJavascriptObject
\****************************************************************************/

NPObject *NPJavascriptObject::Allocate(NPP npp, NPClass *aClass)
{
    return (NPObject *)new NPJavascriptObject(npp);
}

void NPJavascriptObject::_Deallocate(NPObject *npobj)
{
    delete npobj;
}

void NPJavascriptObject::_Invalidate(NPObject *npobj)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    obj->Invalidate();
}

bool NPJavascriptObject::_HasMethod(NPObject *npobj, NPIdentifier name)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->HasMethod(name);
}

bool NPJavascriptObject::_Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->Invoke(name, args, argCount, result);
}

bool NPJavascriptObject::_InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->InvokeDefault(args, argCount, result);
}

bool NPJavascriptObject::_HasProperty(NPObject *npobj, NPIdentifier name)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->HasProperty(name);
}

bool NPJavascriptObject::_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->GetProperty(name, result);
}

bool NPJavascriptObject::_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->SetProperty(name, value);
}

bool NPJavascriptObject::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->RemoveProperty(name);
}

bool NPJavascriptObject::_Enumeration(NPObject *npobj, NPIdentifier **value, uint32_t *count)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->Enumeration(value, count);
}

bool NPJavascriptObject::_Construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    NPJavascriptObject *obj = static_cast<NPJavascriptObject *>(npobj);
    return obj->Construct(args, argCount, result);
}

// This defines the "entry points"; it's how the browser knows how to create the object
// when you call NPN_CreateObject, and how it knows how to call functions on it
NPClass NPJavascriptObject::NPJavascriptObjectClass = {
    NP_CLASS_STRUCT_VERSION_CTOR,
    NPJavascriptObject::Allocate,
    NPJavascriptObject::_Deallocate,
    NPJavascriptObject::_Invalidate,
    NPJavascriptObject::_HasMethod,
    NPJavascriptObject::_Invoke,
    NPJavascriptObject::_InvokeDefault,
    NPJavascriptObject::_HasProperty,
    NPJavascriptObject::_GetProperty,
    NPJavascriptObject::_SetProperty,
    NPJavascriptObject::_RemoveProperty,
    NPJavascriptObject::_Enumeration,
    NPJavascriptObject::_Construct
};