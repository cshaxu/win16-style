# M0 T18 S64: TBevel shared-canvas investigation

The existing T18/S64 packet covers ordinary VCL controls including TBevel. This investigation does not treat a framework-drawn windowless control as application-authored content or declare it out of scope.

## Evidence

A self-owned DelphiVCL4Python 1.0.7 sample created two default TBevel instances (`bsBox`, `bsLowered`), each 120 by 50 pixels, on the same form. Native x86 and x64 observations at reported 96 DPI loaded the respective released assets DLL and confirmed the parent form was attached. Neither control exposed a child HWND in the sample. During one synchronous parent redraw, both instances used the same parent-window DC and each produced two Polyline calls: gray top/left and white bottom/right edges. Their local clip rectangles were both 0,0–120,50; the DC window origins distinguished their positions (-10,-10 and -155,-10). The observer restored its temporary IAT substitutions after the redraw. Both isolated driver runs reported capture and close success and the self-owned target exited.

A separate x86 run queried the form's `OBJID_CLIENT` using `AccessibleObjectFromWindow`. The call and `get_accChildCount` succeeded, but the returned child count was zero; it did not expose either Bevel as an accessible child. That result concerns this sample and provider only, not all Delphi versions or all accessibility APIs. Its self-owned driver reported capture and close success, target exit, and temporary observer restoration.

## Consequence and next step

Geometry, clipping, colors and GDI calls alone do not identify a TBevel: application painting can use the same primitives on the same parent DC. No product interception has been added on that basis. Continue looking for a reliable framework ownership or object identity witness, then verify default and other normal shapes and states before altering rendering. Other VCL versions, shapes, DPI values, real input, and broader lifecycle behavior remain untested. The work remains open under M0/T18/S64. Scratch is registered in CURRENT under `build/t18-s64-vb-mask-survey`; screenshots and logs are local diagnostic artifacts, not product files.

## Public VCL object lookup follow-up

Embarcadero documents [`FindControl(HWND)`](https://docwiki.embarcadero.com/Libraries/Sydney/en/Vcl.Controls.FindControl) and [`TWinControl.ControlAtPos`](https://docwiki.embarcadero.com/Libraries/Alexandria/en/Vcl.Controls.TWinControl.ControlAtPos). The first identifies the window-owning VCL object and the second returns a child object at a position, but these are VCL object APIs, not documented Win32 HWND messages or a version-independent C ABI. They are plausible research leads, not evidence that a separately injected MinGW C DLL can call them safely in arbitrary statically linked VCL programs.

A self-owned x86 DelphiVCL4Python sample tried its binding with a real `Point(20,20)` and two-, three-, and four-argument `ControlAtPos` calls; each returned the binding's `TypeError` for incompatible arguments. The sample still completed its capture and close normally. The binding failure does not prove the underlying Delphi method fails, or that it cannot work via another interface. No Bevel identification or rendering support was demonstrated by this experiment. The initial attempted fixture edit failed and briefly truncated only the ignored scratch script; it was reconstructed from the existing gallery before the reported run.
