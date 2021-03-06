#include "config.h"
#include "PalmServiceBridge.h"
#include "Logging.h"

#include "Document.h"
#include "Event.h"
#include "EventException.h"
#include "EventListener.h"
#include "EventNames.h"
#include "ExceptionCode.h"
#include "Frame.h"
#include "Logging.h"
#include "Page.h"
#include "Settings.h"
#include <wtf/text/WTFString.h>
#include "ScriptController.h"
#include <wtf/RefCountedLeakCounter.h>

#include "JSDOMWindow.h"
#include "JSEventListener.h"
#include "JSFunction.h"
#include "ScriptSourceCode.h"
#include "ScriptValue.h"
#include "runtime_root.h"
#include <runtime/JSLock.h>
using namespace JSC;

#include <map>
#include <set>

bool LoggingInitialized = false;
PmLogContext LogContext;

namespace WebCore {

typedef std::set<PalmServiceBridge*> ServicesSet;
typedef std::map<Document*, ServicesSet*> ServicesSetMap;

#ifndef NDEBUG
static WTF::RefCountedLeakCounter serviceBridgeCounter("PalmServiceBridge");
#endif

static ServicesSetMap* servicesByDocument()
{
    static ServicesSetMap map;
    return &map;
}

int PalmServiceBridge::numHandlesForUrl(const char* appId)
{
    for (ServicesSetMap::iterator setIt = servicesByDocument()->begin(); setIt != servicesByDocument()->end(); ++setIt) {
        if (!strcmp(appId, setIt->first->url().string().utf8().data()))
            return setIt->second->size();
    }

    return 0;
}

void PalmServiceBridge::handlesForUrl(const char* appId, std::list<PalmServiceBridge*>& outHandles)
{
    outHandles.clear();
    for (ServicesSetMap::iterator setIt = servicesByDocument()->begin(); setIt != servicesByDocument()->end(); ++setIt) {
        if (!strcmp(appId, setIt->first->url().string().utf8().data())) {
            ServicesSet* set = setIt->second;

            for (ServicesSet::iterator s = set->begin(); s != set->end(); ++s)
                outHandles.push_back(*s);

            return;
        }
    }
}

static void addToServicesByDocument(Document* doc, PalmServiceBridge* svc)
{
    if (!doc || !svc)
        return;

    ServicesSet* set = 0;
    ServicesSetMap::iterator it = servicesByDocument()->find(doc);
    if (it == servicesByDocument()->end()) {
        set = new ServicesSet();
        (*servicesByDocument())[doc] = set;
    } else
        set = it->second;

    set->insert(svc);
}

static void removeFromServicesByDocument(Document* doc, PalmServiceBridge* svc)
{
    if (!doc || !svc)
        return;

    ServicesSetMap::iterator it = servicesByDocument()->find(doc);
    if (it == servicesByDocument()->end())
        return;

    ServicesSet* set = it->second;
    if (!set)
        return;

    set->erase(svc);
    if (!set->size()) {
        // remove from the hash map
        delete set;
        servicesByDocument()->erase(it);
    }
}

PalmServiceBridge::PalmServiceBridge(ScriptExecutionContext* context, bool subscribe)
    : ActiveDOMObject(context),
      m_canceled(false),
      m_subscribed(subscribe),
      m_inServiceCallback(false),
      m_identifier(0),
      m_isPrivileged(false)
{
    if (!LoggingInitialized) {
        PmLogGetContext("QtWebProcess", &LogContext);
        LoggingInitialized = true;
    }

    addToServicesByDocument(document(), this);

#ifndef NDEBUG
    serviceBridgeCounter.increment();
#endif
    Frame *frame = document()->frame();
    if (document()->page()->mainFrame() == frame) {
        m_identifier = strdup(document()->page()->settings()->identifier().utf8().data());
    }
    else {
        JSValue identifier;
        ExecState* exec = 0;
        Frame *frame = document()->frame();
        if (frame) {
            identifier = frame->script()->executeScript(ScriptSourceCode("PalmSystem && PalmSystem.getIdentifierForFrame(window.frameElement.id, window.frameElement.src)")).jsValue();
            JSGlobalObject* globalObject = frame->script()->bindingRootObject()->globalObject();
            exec = globalObject->globalExec();
            m_identifier = strdup(identifier.toString(exec)->value(exec).utf8().data());
        }
    }

    if (document()->page() != 0)
        m_isPrivileged = document()->page()->settings()->privileged();

    DEBUG("PalmServiceBridge[%p]: created (subscribe %d identifier %s privileged %d)",
          this, subscribe, m_identifier, m_isPrivileged);
}

bool PalmServiceBridge::init(Document* d, bool subscribe)
{
    m_subscribed = subscribe;

    DEBUG("PalmServiceBridge[%p]: initialized (subscribe %d)", this, subscribe);

    return true;
}

PalmServiceBridge::~PalmServiceBridge()
{
    DEBUG("PalmServiceBridge[%p]: destroying (identifier %s privileged %d subscribed %d)",
          this, m_identifier, m_isPrivileged, m_subscribed);

    ExceptionCode ec;
    cancel(ec);

    if (scriptExecutionContext() && document())
        removeFromServicesByDocument(document(), this);

    if (m_identifier)
        free(m_identifier);

#ifndef NDEBUG
    serviceBridgeCounter.decrement();
#endif
}

void PalmServiceBridge::detachServices(Document* doc)
{
    ServicesSetMap::iterator it = servicesByDocument()->find(doc);
    if (it == servicesByDocument()->end())
        return;

    ServicesSet* services = it->second;
    servicesByDocument()->erase(it);

    if (services) {
        while (services->size()) {
            ServicesSet::iterator sit = services->begin();
            ExceptionCode ec;
            (*sit)->cancel(ec);
            services->erase(sit);
        }
        delete services;
    }

}

void PalmServiceBridge::cancelServices(Document* doc)
{
    ServicesSetMap::iterator it = servicesByDocument()->find(doc);
    if (it == servicesByDocument()->end())
        return;

    ServicesSet* services = it->second;

    if (services) {
        for (ServicesSet::iterator sit = services->begin(); sit != services->end(); ++sit) {
            PalmServiceBridge* br = *sit;
            ExceptionCode ec;
            br->cancel(ec);
        }
    }
}

String PalmServiceBridge::version()
{
    return String("1.1");
}

int PalmServiceBridge::token()
{
    return (int)listenerToken;
}

int PalmServiceBridge::call(const String& uri, const String& payload, ExceptionCode& ec)
{
    DEBUG("PalmServiceBridge[%p]: calling on uri %s payload %s (identifier %s privileged %d subscribed %d)",
          this, uri.utf8().data(), payload.utf8().data(), m_identifier, m_isPrivileged, m_subscribed);

    LunaServiceManager::instance()->call(uri.utf8().data(), payload.utf8().data(), this, m_identifier, m_isPrivileged);
    if (LSMESSAGE_TOKEN_INVALID == listenerToken) {
        ExceptionCode ec;
        cancel(ec);
    }

    return (int)listenerToken;
}

void PalmServiceBridge::serviceResponse(const char* body)
{
    if (m_canceled || !document())
        return;

    if (!body)
        body = "";

    DEBUG("PalmServiceBridge[%p]: got service response %s (identifier %s privileged %d subscribed %d)",
          this, body, m_identifier, m_isPrivileged, m_subscribed);

    Frame* frame = document()->frame();

    JSGlobalObject* globalObject = frame->script()->bindingRootObject()->globalObject();
    ExecState* exec = globalObject->globalExec();

    JSLockHolder lock(exec);

    const JSEventListener* listener = JSEventListener::cast(m_callbackFunction.get());
    JSObject* function = 0;
    if (listener)
        function = listener->jsFunction(scriptExecutionContext());
    if (!function)
        return;

    CallData callData;
    CallType callType = JSFunction::getCallData(function, callData);

    if (callType == CallTypeNone)
        return;

    MarkedArgumentBuffer args;
    args.append(jsString(exec, String::fromUTF8(body)));

    JSC::call(exec, function, callType, callData, function, args);

    if (exec->hadException())
        reportCurrentException(exec);

    document()->updateStyleIfNeeded();
}

void PalmServiceBridge::cancel(ExceptionCode& ec)
{
    if (m_canceled)
        return;

    m_canceled = true;
    if (listenerToken) {
        DEBUG("PalmServiceBridge[%p]: canceling current call (identifier %s privileged %d subscribed %d)",
            this, m_identifier, m_isPrivileged, m_subscribed);

        LunaServiceManager::instance()->cancel(this);
    }
}

void PalmServiceBridge::stop()
{
    ExceptionCode ec;

    DEBUG("PalmServiceBridge[%p]: stopping ... (identifier %s privileged %d subscribed %d)",
        this, m_identifier, m_isPrivileged, m_subscribed);

    cancel(ec);
}

bool PalmServiceBridge::canSuspend() const
{
    return false;
}

void PalmServiceBridge::contextDestroyed()
{
    ActiveDOMObject::contextDestroyed();
}

ScriptExecutionContext* PalmServiceBridge::scriptExecutionContext() const
{
    return ActiveDOMObject::scriptExecutionContext();
}

Document* PalmServiceBridge::document() const
{
    ASSERT(scriptExecutionContext()->isDocument());
    return static_cast<Document*>(scriptExecutionContext());
}

} // namespace WebCore
