# M0 T26 S1: UWP global manager lifecycle tests

The shared UWP manager's existing owned-process fixture now covers an apply
failure followed by retry, duplicate observation after success, a removal
failure followed by stop retry, a blocked worker causing bounded idle/stop
timeouts and successful completion after release, and target exit before stop.
Both x86 and x64 native incremental builds passed. With the test's own synthetic
child process on the current desktop, both test executions passed outside the
command sandbox and reported four apply attempts and three removal attempts;
the child process was closed and the worker and handles were cleaned up.

The published eight-file package was already current and its assets/O:\winnt
hashes were verified equal before testing. No product source or binary changed
in this P. Earlier sandboxed policy inspection returned `ERROR_ACCESS_DENIED`;
no system security policy was altered. The test uses a test-owned host filter
and callbacks, not a real ApplicationFrameHost, and does not prove actual UWP
visual restoration, PID reuse handling or end-to-end global style switching.
T26 and the wider cross-style migration remain open.
