#include "cachewraith/alert/CooldownManager.h"
#include "cachewraith/security/NginxLogMonitor.h"
#include "cachewraith/security/SshLogMonitor.h"

#include <cassert>
#include <iostream>

using namespace cachewraith;

int main() {
    const auto failed = parseSshLogLine("Apr 25 10:00:00 host sshd[123]: Failed password for invalid user admin from 1.2.3.4 port 22 ssh2");
    assert(failed);
    assert(failed->kind == SshEvent::Kind::FailedPassword);
    assert(failed->user == "admin");
    assert(failed->ip == "1.2.3.4");

    const auto accepted = parseSshLogLine("Apr 25 10:01:00 host sshd[124]: Accepted publickey for root from 5.6.7.8 port 22 ssh2");
    assert(accepted);
    assert(accepted->kind == SshEvent::Kind::AcceptedPublicKey);
    assert(accepted->user == "root");
    assert(accepted->ip == "5.6.7.8");

    const auto nginx = parseNginxAccessLine(R"(1.2.3.4 - - [25/Apr/2026:10:00:00 +0000] "GET /.env HTTP/1.1" 404 12 "-" "curl")");
    assert(nginx);
    assert(nginx->ip == "1.2.3.4");
    assert(nginx->path == "/.env");
    assert(nginx->status == 404);
    assert(isSuspiciousNginxPath(nginx->path, {"/.env", "/wp-admin"}));

    CooldownManager cooldowns(Cooldowns{});
    Alert alert;
    alert.type = "service_down";
    alert.key = "nginx";
    assert(cooldowns.shouldSend(alert));
    assert(!cooldowns.shouldSend(alert));
    alert.bypass_cooldown = true;
    assert(cooldowns.shouldSend(alert));

    std::cout << "All parser tests passed\n";
    return 0;
}
