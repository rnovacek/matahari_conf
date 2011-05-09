
#include <matahari/mh_agent.h>
#include "qmf/org/matahariproject/QmfPackage.h"
#include <iostream>
extern "C" {
#include "conf.h"
#include <string.h>
}

class ConfAgent : public MatahariAgent
{
private:
    qmf::org::matahariproject::PackageDefinition _package;
public:
    int setup(qmf::AgentSession session);
    gboolean invoke(qmf::AgentSession session, qmf::AgentEvent event, gpointer user_data);
};

int
main(int argc, char *argv[])
{
    ConfAgent agent;
    int rc = agent.init(argc, argv, "conf");
    if (rc == 0) {
        agent.run();
    }
    return rc;
}

int
ConfAgent::setup(qmf::AgentSession session)
{
    _package.configure(session);
    _instance = qmf::Data(_package.data_Config);

    _agent_session.addData(_instance);    
    return 0;
}

char *
getError(int handle)
{
    char *message = NULL, *message_minor = NULL, *details = NULL;
    conf_error_verbose(handle, message, message_minor, details);
    if (message == NULL)
        message = strdup("No Augeas context with this handle");
    return message;
}

gboolean
ConfAgent::invoke(qmf::AgentSession session, qmf::AgentEvent event, gpointer user_data)
{
    if(event.getType() == qmf::AGENT_METHOD) {
        const std::string& methodName(event.getMethodName());
        std::cout << "Method " << methodName << " from " << event.getUserId() << std::endl;

        if (methodName == "init") {
            int handle = conf_init(event.getArguments()["lens"].asString().c_str());
            if (handle < 0)
            {
                session.raiseException(event, "Unable to initialize Augeas.");
                return TRUE;
            }
            event.addReturnArgument("handle", handle);
        }
        else if (methodName == "get") {
            int handle = event.getArguments()["handle"].asInt32();
            const char *value;
            int result = conf_get(handle, event.getArguments()["path"].asString().c_str(), &value);
            if (result < 0)
            {
                session.raiseException(event, getError(handle));
                return TRUE;
            }
            event.addReturnArgument("value", value);
            event.addReturnArgument("result", result);
        }
        else if (methodName == "set") {
            int handle = event.getArguments()["handle"].asInt32();
            int result = conf_set(handle,
                                event.getArguments()["path"].asString().c_str(),
                                event.getArguments()["value"].asString().c_str());
            if (result < 0)
            {
                session.raiseException(event, getError(handle));
                return TRUE;
            }
            event.addReturnArgument("result", result);
        }
        else if (methodName == "setm") {
            int handle = event.getArguments()["handle"].asInt32();
            int result = conf_setm(handle,
                                event.getArguments()["base"].asString().c_str(),
                                event.getArguments()["sub"].asString().c_str(),
                                event.getArguments()["value"].asString().c_str());
            if (result < 0)
            {
                session.raiseException(event, getError(handle));
                return TRUE;
            }
            event.addReturnArgument("result", result);
        }
        else if (methodName == "insert") {
            int handle = event.getArguments()["handle"].asInt32();
            int result = conf_insert(handle,
                                    event.getArguments()["path"].asString().c_str(),
                                    event.getArguments()["label"].asString().c_str(),
                                    event.getArguments()["before"].asInt32());
            if (result < 0)
            {
                session.raiseException(event, getError(handle));
                return TRUE;
            }
            event.addReturnArgument("result", result);
        }
        else if (methodName == "mv") {
            int handle = event.getArguments()["handle"].asInt32();
            int result = conf_mv(handle,
                                event.getArguments()["src"].asString().c_str(),
                                event.getArguments()["dest"].asString().c_str());
            if (result < 0)
            {
                session.raiseException(event, getError(handle));
                return TRUE;
            }
            event.addReturnArgument("result", result);
        }
        else if (methodName == "rm") {
            int handle = event.getArguments()["handle"].asInt32();
            int result = conf_rm(handle,
                                event.getArguments()["path"].asString().c_str());
            if (result < 0)
            {
                session.raiseException(event, getError(handle));
                return TRUE;
            }
            event.addReturnArgument("result", result);
        }
        else if (methodName == "match") {
            char **matches = NULL;
            int handle = event.getArguments()["handle"].asInt32();
            int result = conf_match(handle,
                                    event.getArguments()["path"].asString().c_str(),
                                    &matches);
            if (result < 0)
            {
                session.raiseException(event, getError(handle));
                return TRUE;
            }
            event.addReturnArgument("matches", matches);
            event.addReturnArgument("result", result);
        }
        else if (methodName == "save") {
            int handle = event.getArguments()["handle"].asInt32();
            int result = conf_save(handle);
            if (result < 0)
            {
                session.raiseException(event, getError(handle));
                return TRUE;
            }
            event.addReturnArgument("result", result);
        }
        else if (methodName == "error") {
            int handle = event.getArguments()["handle"].asInt32();
            int code = conf_error(handle);
            event.addReturnArgument("code", code);
        }
        else if (methodName == "error_verbose") {
            char *message = NULL, *minor_message = NULL, *details = NULL;
            int handle = event.getArguments()["handle"].asInt32();
            conf_error_verbose(handle, message, minor_message, details);
            event.addReturnArgument("message", message);
            event.addReturnArgument("minor_message", minor_message);
            event.addReturnArgument("details", details);
        }
        else if (methodName == "close") {
            int handle = event.getArguments()["handle"].asInt32();
            conf_close(handle);
        }
        else {
            session.raiseException(event, MH_NOT_IMPLEMENTED);
            goto bail;
        }
    }
    session.methodSuccess(event);
  bail:
    return TRUE;
}
