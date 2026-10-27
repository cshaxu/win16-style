# M0 T18 S5 VCL tab wrapper parity

The existing public tab contract already covered `TPageControl`, but omitted
the exact VCL `TTabControl` registration name. S5 maps that one class to the
same role, with no new drawing path. The tab fixture now exercises native,
`TPageControl`, and `TTabControl` registrations across ordinary, multiline,
button, owner-draw, bottom, vertical, and fixed-width modes; only the
previously permitted ordinary modes attach. It also proves both VCL names
reject DefWindowProc impostors.

No date picker, month calendar, or hot-key class was added: the product has no
renderer for those controls, so name recognition would not be support. The
clean dual-width package identity gate passed and the 87-case package run
completed `passed-with-skips` because private-desktop GUI tests return error
5 in this session. The tested four files were published to `assets/` and
`O:\winnt\system32`; real Delphi integration remains a T18-final item.
