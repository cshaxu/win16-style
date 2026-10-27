# M0 T18 S3 basic VCL probe transfer

S3 established that the current execution session cannot create a self-owned
desktop: all five relevant x86/x64 protocol and DoubleKiller probes returned
`PRIVATE_DESKTOP_CREATE_FAIL error=5` before `CreateProcess`. No target
application ran, and this is neither an injection failure nor a rendering
result.

The user then explicitly changed T18 sequencing: implement the bounded,
public-protocol paths first and defer real Delphi integration to the final T18
closure. S3 therefore transfers rather than treats the environment limitation
as a product blocker. Historical real VCL evidence remains separate from a
current-package result; details and exact commands are in
[delphi-basic-vcl-probe.json](../etc/evidence/delphi-basic-vcl-probe.json).

S4 is admitted for the first code group. It may use only public messages,
registered-class/module ownership and self-contained positive/negative tests.
It cannot use a component name alone, private VCL layout, a current-desktop
fallback, or a real-app pass claim. Every resulting integration gap remains a
T18 final-audit requirement.
