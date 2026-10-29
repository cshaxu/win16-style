# M0 T18 S64: control census, evidence-based interim reconciliation

Scope: repository frozen VB edition/component rows and VCL family seeds, not a universal product catalogue. Count one VB5 or VB6 version row separately; count each frozen VCL family once. Hosts and menus are included because the frozen inventory includes them. No family has evidence certifying every ordinary variant and runtime; a limited path is partial, not complete. Native preservation and missing evidence are not proof that no code exists.

This is an interim reconciliation, not a final supported-versus-unimplemented audit: unresolved rows include ordinary visible controls, special hosted surfaces and scope decisions. A row in the partial group may still need work. Normal framework painting is in scope even if the control is windowless; application-authored content stays outside. Existing OCX support is listed where evidenced. Only the previously frozen deferred dispositions are marked deferred here, not every OCX. CommonDialog delegates to the separate dialog project and is not counted as nonvisual work.

## VB5: 59 frozen rows

- limited-evidence: **21** — `CommandButton`, `CheckBox`, `OptionButton`, `TextBox`, `ComboBox`, `ListBox`, `HScrollBar`, `VScrollBar`, `DirListBox`, `FileListBox`, `Frame`, `PictureBox`, `Label`, `Form`, `MDIForm`, `Menu`, `comctl32.ocx:Toolbar`, `comctl32.ocx:TreeView`, `comctl32.ocx:Slider`, `msmask32.ocx:MaskEdBox`, `dblist32.ocx:DBCombo`.

- native-or-unverified: **17** — `Data`, `OLE`, `UserControl`, `UserDocument`, `PropertyPage`, `comctl32.ocx:StatusBar`, `comctl32.ocx:TabStrip`, `comctl32.ocx:ListView`, `comctl32.ocx:ProgressBar`, `comct232.ocx:Animation`, `comct232.ocx:UpDown`, `richtx32.ocx:RichTextBox`, `tabctl32.ocx:SSTab`, `msflxgrd.ocx:MSFlexGrid`, `dblist32.ocx:DBList`, `msrdc20.ocx:MSRDC`, `mci32.ocx:MMControl`.

- nonvisual-or-content-only: **12** — `Image`, `Shape`, `Line`, `Timer`, `comctl32.ocx:ImageList`, `picclp32.ocx:PictureClip`, `mscomm32.ocx:MSComm`, `msinet.ocx:Inet`, `msmapi32.ocx:MAPISession`, `msmapi32.ocx:MAPIMessages`, `mswinsck.ocx:Winsock`, `sysinfo.ocx:SysInfo`.

- separate-dialog-task: **1** — `comdlg32.ocx:CommonDialog`.

- deferred: **8** — `DriveListBox`, `dbgrid32.ocx:DBGrid`, `grid32.ocx:Grid`, `msoutl32.ocx:Outline`, `mschart.ocx:MSChart`, `mciwndx.ocx:MCIWnd`, `mv141kn.ocx:MedView`, `mdtsg.ocx:SchGrid`.

## VB6: 89 frozen rows

- limited-evidence: **26** — `CommandButton`, `CheckBox`, `OptionButton`, `TextBox`, `ComboBox`, `ListBox`, `HScrollBar`, `VScrollBar`, `DirListBox`, `FileListBox`, `Frame`, `PictureBox`, `Label`, `Data`, `Form`, `MDIForm`, `Menu`, `comctl32.ocx:Toolbar`, `comctl32.ocx:TreeView`, `comctl32.ocx:Slider`, `mscomctl.ocx:TreeView`, `msmask32.ocx:MaskEdBox`, `dblist32.ocx:DBCombo`, `msdatgrd.ocx:DataGrid`, `msdatlst.ocx:DataList`, `msdatlst.ocx:DataCombo`.

- native-or-unverified: **33** — `OLE`, `UserControl`, `UserDocument`, `PropertyPage`, `comctl32.ocx:StatusBar`, `comctl32.ocx:TabStrip`, `comctl32.ocx:ListView`, `comctl32.ocx:ProgressBar`, `mscomctl.ocx:Toolbar`, `mscomctl.ocx:StatusBar`, `mscomctl.ocx:TabStrip`, `mscomctl.ocx:ListView`, `mscomctl.ocx:ProgressBar`, `mscomctl.ocx:Slider`, `mscomctl.ocx:ImageCombo`, `comct232.ocx:Animation`, `comct232.ocx:UpDown`, `mscomct2.ocx:Animation`, `mscomct2.ocx:UpDown`, `mscomct2.ocx:MonthView`, `mscomct2.ocx:DTPicker`, `mscomct2.ocx:FlatScrollBar`, `comct332.ocx:CoolBar`, `richtx32.ocx:RichTextBox`, `tabctl32.ocx:SSTab`, `msflxgrd.ocx:MSFlexGrid`, `mshflxgd.ocx:MSHFlexGrid`, `dblist32.ocx:DBList`, `msadodc.ocx:Adodc`, `msrdc20.ocx:MSRDC`, `msdatrep.ocx:DataRepeater`, `mschrt20.ocx:MSChart`, `mci32.ocx:MMControl`.

- nonvisual-or-content-only: **15** — `Image`, `Shape`, `Line`, `Timer`, `comctl32.ocx:ImageList`, `mscomctl.ocx:ImageList`, `picclp32.ocx:PictureClip`, `mscomm32.ocx:MSComm`, `msinet.ocx:Inet`, `msmapi32.ocx:MAPISession`, `msmapi32.ocx:MAPIMessages`, `mswinsck.ocx:Winsock`, `sysinfo.ocx:SysInfo`, `msscript.ocx:ScriptControl`, `vbsql.ocx:Vbsql`.

- separate-dialog-task: **1** — `comdlg32.ocx:CommonDialog`.

- deferred: **14** — `DriveListBox`, `dbgrid32.ocx:DBGrid`, `MSWLess.ocx:WLCheck`, `MSWLess.ocx:WLCombo`, `MSWLess.ocx:WLCommand`, `MSWLess.ocx:WLFrame`, `MSWLess.ocx:WLHScroll`, `MSWLess.ocx:WLList`, `MSWLess.ocx:WLOption`, `MSWLess.ocx:WLText`, `MSWLess.ocx:WLVScroll`, `grid32.ocx:Grid`, `msoutl32.ocx:Outline`, `mschart.ocx:MSChart`.

## Delphi VCL: 74 frozen families

- limited-evidence: **45** — `TDBListBox`, `TMDIForm`, `TMediaPlayer`, `TDBNavigator`, `TDBLookupComboBox`, `TDBComboBox`, `TDBEdit`, `TDBGrid`, `TMainMenu`, `TMaskEdit`, `TForm`, `TPanel`, `TGroupBox`, `TScrollBox`, `TButton`, `TBitBtn`, `TCheckBox`, `TRadioButton`, `TRadioGroup`, `TEdit`, `TMemo`, `TRichEdit`, `TComboBox`, `TListBox`, `TCheckListBox`, `TColorBox`, `TStaticText`, `TPopupMenu`, `TToolBar`, `TStatusBar`, `TControlBar`, `TTreeView`, `TListView`, `THeaderControl`, `TTabControl`, `TPageControl`, `TProgressBar`, `TTrackBar`, `TUpDown`, `TDateTimePicker`, `TMonthCalendar`, `THotKey`, `TStringGrid`, `TDrawGrid`, `TValueListEditor`.

- native-or-unverified: **21** — `TFrame`, `TSplitter`, `TSpeedButton`, `TLabel`, `TShape`, `TBevel`, `TChart`, `TCoolBar`, `TAnimate`, `TDirectoryListBox`, `TDriveComboBox`, `TFileListBox`, `TDBLookupListBox`, `TWebBrowser`, `TOleContainer`, `TButtonGroup`, `TCategoryButtons`, `TDockTabSet`, `TTabSet`, `TOutline`, `TTabbedNotebook`.

- nonvisual-or-content-only: **6** — `TImage`, `TPaintBox`, `TImageList`, `TActionList`, `TTimer`, `TTrayIcon`.

- separate-dialog-task: **2** — `TOpenDialog`, `TSaveDialog`.

The nine families promoted since the S63 ledger are TPanel, TGroupBox, TScrollBox, TRichEdit, TPopupMenu, TControlBar, TMonthCalendar, THotKey and TValueListEditor; their S64 P71/P73/P75/P121–P135 histories provide limited evidence only. This table does not promote other post-S63 controls absent from the original 74 seeds. `TShape` and `TBevel` remain under review rather than being dismissed as application-drawn merely because they lack a normal child HWND.

VB additions after the frozen ledger: ordinary MDI child frame is limited for both editions (P30/P31); VB6 Data has a limited frame (P47). UserControl child controls can be styled without proving its outer shell; it remains unverified. The 23 frozen deferred rows include nonvisual VB6 Vbsql, counted once in nonvisual instead of deferred.

Evidence: [VB frozen inventory](../etc/evidence/vb-controls-audit.json), [VB final ledger](../etc/evidence/vb-final-audit-ledger.json), [Delphi S63 count](../etc/evidence/delphi-s63-count.json), [current task](../states/CURRENT.md), [scope decision](M0-T18-S64-P68-ordinary-scope-group-caption.md). Counts are an auditable evidence accounting, not an exact count of remaining code changes or proof of complete implementation.

Update after P138: TMaskEdit is supported only under P138’s witnessed single-line EDIT path. TMainMenu is limited-evidence because P73 already established real main/nested menu use and cancellation, which the stale S63 ledger had not incorporated. With P138 assets the real DelphiVCL4Python 1.0.7 x86/x64 samples at observed 96 DPI entered the owner-draw menu path, opened the nested submenu, captured both and restored the original item type and ID on Escape; the target and launcher exited normally. This does not prove all menu states, callbacks or DPI. The two promotions make 37 limited, 29 native/unverified, 6 nonvisual/content and 2 separately queued dialogs; the counts still include non-ordinary entries awaiting scope review and are not a remaining-work count.

Update after P148: `TDBGrid` has a limited application-owned Unicode scrollbar and frame path, with real x86 sample and dual-width model evidence in [P148](M0-T18-S64-P148-dbgrid.md). The current Delphi counts are 38 limited and 28 native/unverified; no-scroll variants, data writeback and other versions remain unverified.

Update after P152: `TDBEdit` has a limited same-owner ANSI native-edit forwarding path, with a real x86 data-binding sample and dual-width model evidence in [P152](M0-T18-S64-P152-dbedit.md). Current Delphi counts are 39 limited and 27 native/unverified; other runtimes, languages and physical input remain unverified.

Update after P153: `TDBComboBox` has a limited editable ANSI native-combo path, with real x86 data-binding and dual-width native model evidence in [P153](M0-T18-S64-P153-dbcombo.md). Current Delphi counts are 40 limited and 26 native/unverified; Unicode forwarding, other styles and real multilingual input remain unverified.

P154 correction: earlier P148/P152/P153 update notes stated the intended promotions, but the table headings and limited list were not updated while the three names were removed from the unverified list. The table now includes each of `TDBGrid`, `TDBEdit` and `TDBComboBox` exactly once. Counts are 40 limited, 26 native/unverified, 6 nonvisual/content and 2 separate dialog entries, totaling 74. Limited means bounded evidence, not complete support.

Update after P156: `TDBLookupComboBox` has a limited same-owner COMBOBOX-theme dropdown-button path; [P156](M0-T18-S64-P156-dblookup.md) covers one real x86 sample and dual-width models. Delphi is now 41 limited, 25 native/unverified, 6 nonvisual/content and 2 separate dialogs, totaling 74. Other drawing states and interactions remain unverified.

Update after P160: `TDBNavigator` has a limited same-owner BUTTON-theme background path; [P160](M0-T18-S64-P160-dbnavigator.md) covers one real x86 sample and dual-width models. Delphi is now 42 limited, 24 native/unverified, 6 nonvisual/content and 2 separate dialogs, totaling 74. Other drawing modes and versions remain unverified.

Update after P161: `TMediaPlayer` has limited same-owner normal/pressed BUTTON-theme background coverage; [P161](M0-T18-S64-P161-mediaplayer.md) covers real x86/x64 VCL without a media device and dual-width models. Delphi is now 43 limited, 23 native/unverified, 6 nonvisual/content and 2 separate dialogs, totaling 74. Media playback, other states and versions remain unverified.

Update after P163: `TMDIForm` has a limited same-owner external VCL MDI child frame path with a real x86/x64 VCL sample and dual-width native identity and lifecycle models. Delphi now has 44 limited, 22 native/unverified, 6 nonvisual/content and 2 separate dialogs. This is bounded evidence, not complete support across all MDI variants.

Update after P164: `TDBListBox` is counted as limited only for the self-authored native LISTBOX superclass model, which the existing P163 renderer already supports. No real data-bound TDBListBox has been run; actual VCL integration and binding remain unverified. Delphi counts: 45 limited (including this model-only path), 21 native/unverified, 6 nonvisual/content, 2 separate dialogs.
