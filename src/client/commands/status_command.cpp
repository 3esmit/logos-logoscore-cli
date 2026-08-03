#include "status_command.h"
#include "../client_state.h"
#include "../../daemon/daemon_state.h"

#include <signal.h>
#include <errno.h>

int StatusCommand::execute(const std::vector<std::string>& args)
{
    (void)args;

    if (!ClientStateFile::read().fileOk) {
        LogosMap result{{"daemon", LogosMap{{"status","not_configured"}}}};
        output().printStatus(result);
        return 1;
    }

    {
        const DaemonRuntimeState rs = DaemonRuntimeStateFile::read();
        if (rs.fileOk && rs.pid > 0 && ::kill(static_cast<pid_t>(rs.pid), 0) != 0
                                    && errno == ESRCH) {
            LogosMap result{{"daemon", LogosMap{
                {"status", "not_running"},
                {"reason", "stale state file (daemon crashed; pid no longer alive)"},
                {"pid",    rs.pid},
            }}};
            output().printStatus(result);
            return 1;
        }
    }

    // Status owns its "not running" result, so connect directly instead of
    // ensureConnected(): the generic helper would emit NO_DAEMON first and
    // produce a second JSON document before this status result.
    if (!client().isConnected() && !client().connect()) {
        LogosMap result{{"daemon", LogosMap{
            {"status", "not_running"},
            {"reason", client().lastError()},
        }}};
        output().printStatus(result);
        return 1;
    }

    LogosMap status = client().getStatus();

    if (!status.is_object()) {
        output().printError("RPC_FAILED", "getStatus RPC call failed.");
        return 1;
    }

    if (status.value("status", std::string{}) == "error") {
        output().printError(
            status.value("code", std::string{"RPC_FAILED"}),
            status.value("message", std::string{"getStatus RPC call failed."}),
            status);
        return 1;
    }

    if (!status.contains("daemon")) {
        output().printError("RPC_FAILED", "getStatus RPC call failed.");
        return 1;
    }

    output().printStatus(status);
    return 0;
}
