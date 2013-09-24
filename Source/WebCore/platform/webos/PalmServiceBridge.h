#ifndef PalmServiceBridge_h
#define PalmServiceBridge_h

#include "ActiveDOMObject.h"
#include "Event.h"
#include "EventListener.h"
#include "EventTarget.h"
#include "LunaServiceMgr.h"
#include <wtf/OwnPtr.h>

#include <heap/Strong.h>
#include <heap/StrongInlines.h>

#include <glib.h>
#include <list>


namespace WebCore {

class Document;


class PalmServiceBridge : public RefCounted<PalmServiceBridge>,
                          public LunaServiceManagerListener,
                          public ActiveDOMObject {
    public:
        static PassRefPtr<PalmServiceBridge> create(ScriptExecutionContext* context, bool subscribe = false)
        {
            return adoptRef(new PalmServiceBridge(context, subscribe));
        }

        bool init(Document*, bool subscribed = false);
        ~PalmServiceBridge();

        static int numHandlesForUrl(const char* appId);
        static void handlesForUrl(const char* appId, std::list<PalmServiceBridge*>& outHandles);

        virtual PalmServiceBridge* toPalmServiceBridge() { return this; }

        static void detachServices(Document*);
        static void cancelServices(Document*);

        String version();

        int token();

        int call(const String& uri, const String& payload, ExceptionCode&);
        void cancel(ExceptionCode&);

        // callback from LunaServiceManagerListener
        virtual void serviceResponse(const char* body);

        Document* document() const;

        void setOnservicecallback(PassRefPtr<EventListener> eventListener) { m_callbackFunction = eventListener; }
        EventListener* onservicecallback() const { return m_callbackFunction.get(); }

        virtual ScriptExecutionContext* scriptExecutionContext() const;

        // ActiveDOMObject:
        virtual void contextDestroyed();
        virtual bool canSuspend() const;
        virtual void stop();

    private:
        RefPtr<EventListener> m_callbackFunction;
        bool m_canceled;
        bool m_subscribed;
        bool m_inServiceCallback;
        const char *m_identifier;
        bool m_isPrivileged;

        PalmServiceBridge(ScriptExecutionContext*, bool);
        PalmServiceBridge();
};

}

#endif
