package monitor

import "testing"

func TestParseSSHLogLine(t *testing.T) {
	event, ok := ParseSSHLogLine("Apr 25 10:00:00 vps sshd[1]: Accepted publickey for root from 1.2.3.4 port 22 ssh2")
	if !ok {
		t.Fatal("ParseSSHLogLine() did not parse accepted login")
	}
	if event.Kind != SSHAcceptedPublicKey || event.User != "root" || event.IP != "1.2.3.4" || event.DeviceName != "vps" {
		t.Fatalf("unexpected event: %#v", event)
	}
}

func TestParseNginxAccessLine(t *testing.T) {
	event, ok := ParseNginxAccessLine(`1.2.3.4 - - [25/Apr/2026:10:00:00 +0000] "GET /.env HTTP/1.1" 404 12 "-" "curl"`)
	if !ok {
		t.Fatal("ParseNginxAccessLine() did not parse access log")
	}
	if event.IP != "1.2.3.4" || event.Method != "GET" || event.Path != "/.env" || event.Status != 404 {
		t.Fatalf("unexpected event: %#v", event)
	}
}
