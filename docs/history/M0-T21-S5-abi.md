# M0 T21 S5: shared window attachment ABI

Both DLLs add `Ntw16sWindowAttach` at ordinal 36 with the C calling convention
and signature `BOOL __cdecl Ntw16sWindowAttach(HWND)`. The function only accepts
a window owned by the calling process and GUI thread. It invokes the same
classifier and ownership implementation used after ordinary window creation,
returns `ERROR_ALREADY_EXISTS` for a pre-existing owner and
`ERROR_NOT_SUPPORTED` if no attachment was installed. The caller must keep the
DLL loaded while its attachment or callback is live. `Ntw16sWindowRemove` is
the matching release API. The internal creation path still retains the original
CREATESTRUCT style and its startup-only preparation; the late path uses the
current style and does not rerun the process bootstrap or install process hooks.

Existing ordinals 1–35, the C ABI version and the four-product package remain
unchanged. ABI tests and the PE package gate explicitly verify the additive
name and ordinal on x86/x64. The import ABI decisions remain frozen historical
source decisions. This interface makes reuse explicit; it does not promise that
startup-only hooks or every custom control are available to an existing process.
