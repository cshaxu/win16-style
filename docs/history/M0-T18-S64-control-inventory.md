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

- limited-evidence: **35** — `TForm`, `TPanel`, `TGroupBox`, `TScrollBox`, `TButton`, `TBitBtn`, `TCheckBox`, `TRadioButton`, `TRadioGroup`, `TEdit`, `TMemo`, `TRichEdit`, `TComboBox`, `TListBox`, `TCheckListBox`, `TColorBox`, `TStaticText`, `TPopupMenu`, `TToolBar`, `TStatusBar`, `TControlBar`, `TTreeView`, `TListView`, `THeaderControl`, `TTabControl`, `TPageControl`, `TProgressBar`, `TTrackBar`, `TUpDown`, `TDateTimePicker`, `TMonthCalendar`, `THotKey`, `TStringGrid`, `TDrawGrid`, `TValueListEditor`.

- native-or-unverified: **31** — `TMDIForm`, `TFrame`, `TSplitter`, `TSpeedButton`, `TMaskEdit`, `TLabel`, `TShape`, `TBevel`, `TChart`, `TMainMenu`, `TCoolBar`, `TAnimate`, `TDirectoryListBox`, `TDriveComboBox`, `TFileListBox`, `TDBGrid`, `TDBEdit`, `TDBComboBox`, `TDBListBox`, `TDBNavigator`, `TDBLookupComboBox`, `TDBLookupListBox`, `TWebBrowser`, `TMediaPlayer`, `TOleContainer`, `TButtonGroup`, `TCategoryButtons`, `TDockTabSet`, `TTabSet`, `TOutline`, `TTabbedNotebook`.

- nonvisual-or-content-only: **6** — `TImage`, `TPaintBox`, `TImageList`, `TActionList`, `TTimer`, `TTrayIcon`.

- separate-dialog-task: **2** — `TOpenDialog`, `TSaveDialog`.

The nine families promoted since the S63 ledger are TPanel, TGroupBox, TScrollBox, TRichEdit, TPopupMenu, TControlBar, TMonthCalendar, THotKey and TValueListEditor; their S64 P71/P73/P75/P121–P135 histories provide limited evidence only. This table does not promote other post-S63 controls absent from the original 74 seeds. `TShape` and `TBevel` remain under review rather than being dismissed as application-drawn merely because they lack a normal child HWND.

VB additions after the frozen ledger: ordinary MDI child frame is limited for both editions (P30/P31); VB6 Data has a limited frame (P47). UserControl child controls can be styled without proving its outer shell; it remains unverified. The 23 frozen deferred rows include nonvisual VB6 Vbsql, counted once in nonvisual instead of deferred.

Evidence: [VB frozen inventory](../etc/evidence/vb-controls-audit.json), [VB final ledger](../etc/evidence/vb-final-audit-ledger.json), [Delphi S63 count](../etc/evidence/delphi-s63-count.json), [current task](../states/CURRENT.md), [scope decision](M0-T18-S64-P68-ordinary-scope-group-caption.md). Counts are an auditable evidence accounting, not an exact count of remaining code changes or proof of complete implementation.
