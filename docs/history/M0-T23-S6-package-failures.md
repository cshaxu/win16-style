# M0 T23 S6: preliminary package failures

The same clean candidate failed the private-desktop release check in two focused package runs. Function markers passed; the cause of the retained desktop is unknown. Each candidate case later passed when run alone. This does not turn the failed runs into passes.

## First run: late-only-x64-x86-w

```text
PROPAGATION ENTRY bits=32 role=grandchild dll=1
PROPAGATION ONLY WINDOW bits=32 role=grandchild attached=0
PROPAGATION ENTRY bits=32 role=child dll=1
PROPAGATION ONLY WINDOW bits=32 role=child attached=0
PROPAGATION CHILD create=1 exit=0
PROPAGATION ONLY ROOT create=1 child_exit=0
PROPAGATION ENTRY bits=32 role=grandchild dll=0
PROPAGATION DESCENDANT stopped=1 create=1 exit=41
PROPAGATION ENTRY bits=32 role=child dll=0
PROPAGATION ENTRY bits=32 role=grandchild dll=1
PROPAGATION ONLY WINDOW bits=32 role=grandchild attached=0
PROPAGATION ENTRY bits=32 role=child dll=1
PROPAGATION ONLY WINDOW bits=32 role=child attached=0
PROPAGATION CHILD create=1 exit=0
LATE PROCESS bits=64 api=W init=1 repeat=1 created=1 exit=0 stopped=1 stopped_exit=41 descendant=1 restarted=1 joined=1
LATE PROPAGATION ONLY bits=64 style_registration=0 style_flag=0
PRIVATE_DESKTOP_LEAK name=Win16S-Dialog-8944-185732437
DIALOG_DRIVER exit=8
```

## Second run: late-only-x86-x64-w

```text
PROPAGATION ENTRY bits=64 role=grandchild dll=1
PROPAGATION ONLY WINDOW bits=64 role=grandchild attached=0
PROPAGATION ENTRY bits=64 role=child dll=1
PROPAGATION ONLY WINDOW bits=64 role=child attached=0
PROPAGATION CHILD create=1 exit=0
PROPAGATION ONLY ROOT create=1 child_exit=0
PROPAGATION ENTRY bits=64 role=grandchild dll=0
PROPAGATION DESCENDANT stopped=1 create=1 exit=41
PROPAGATION ENTRY bits=64 role=child dll=0
PROPAGATION ENTRY bits=64 role=grandchild dll=1
PROPAGATION ONLY WINDOW bits=64 role=grandchild attached=0
PROPAGATION ENTRY bits=64 role=child dll=1
PROPAGATION ONLY WINDOW bits=64 role=child attached=0
PROPAGATION CHILD create=1 exit=0
LATE PROCESS bits=32 api=W init=1 repeat=1 created=1 exit=0 stopped=1 stopped_exit=41 descendant=1 restarted=1 joined=1
LATE PROPAGATION ONLY bits=32 style_registration=0 style_flag=0
PRIVATE_DESKTOP_LEAK name=Win16S-Dialog-40352-185841859
DIALOG_DRIVER exit=8
```
