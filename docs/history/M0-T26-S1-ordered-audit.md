# T26 S1 Win16 库顺序审计台账

用户要求按文件顺序逐个审计，不再按零散候选跳转。基线 `f157e02a` 加入本轮已在构建中的列表状态变更；此清单记录当前文件 SHA-256；第 3 项新增 `win16-window/frame_font.h`，总数由 150 增至 151。
按目录名及相对路径升序处理全部文件，包括头文件；目录 `win16-dialog` 当前为空，相关产品对话框入口在库文件之后另行逐项审计。文件移动、删除或新增时更新清单并保留对应关系，不能静默遗漏。

状态：待审、审计中、待修复/待验证、已过关。只有逐项记录职责、实际调用方、风格与公共边界、代码处置、验证和限制后才能标记已过关；历史局部结论可以作为证据，但须按该顺序重新核对。

| 序号 | 文件 | SHA-256 | 状态 |
| ---: | --- | --- | --- |
| 1 | `src/lib-style/win16-common/control_api.h` | `3c26e7bdd7cc1fa995bc9aab43e321c70234a369577ea4e396ea1365429efd84` | 已过关 |
| 2 | `src/lib-style/win16-common/font.c` | `8246cf5cd78b8516e342543d6609a193b8f2e98c3c4d352cd83d3a8ebf21979c` | 已过关 |
| 3 | `src/lib-style/win16-common/font.h` | `63256a509950663b489a187ca6a912510fd5e8e658d8b864d86c7e3f53e04fd3` | 已过关 |
| 4 | `src/lib-style/win16-common/layout.c` | `83cd34e01d35e27fc40c54e82460cab945b45d3a986b3d5ee2ec8ac9ab5c4ede` | 已过关 |
| 5 | `src/lib-style/win16-common/layout.h` | `a55b5d4bad75fda24a46828966f0b8024c85dd52648ac8166e77ae94c541d2d3` | 已过关 |
| 6 | `src/lib-style/win16-common/palette.c` | `c7487eede32ec62a4d803d98bc26f72e890fee6751be31676374e0e0d5b3d9ae` | 已过关 |
| 7 | `src/lib-style/win16-common/palette.h` | `f3a47af9ce56a40f6afa6785fbb67033b46965f7e6fd538f00316d6307e40a36` | 已过关 |
| 8 | `src/lib-style/win16-control/attachment_guard.c` | `77951c6a64d1d4e990cf4e09f76f2c93a9d6d0f96bf2bb03c268c0f09d3b0214` | 已过关 |
| 9 | `src/lib-style/win16-control/attachment_state.h` | `764d7929b4d856293a0483e173aaad03cb7b7b0d9e61f03f2357057cfe59f3d2` | 已过关 |
| 10 | `src/lib-style/win16-control/basic_message.c` | `2bc39526c285621a7d38b19bf9e61d56f05b64f228d3ad4912911d26b9a48b3e` | 已过关 |
| 11 | `src/lib-style/win16-control/basic_message.h` | `22e3e7959725de7eb3972df0238d3a8fdbb1f3b61d8c5fda50390e3a956fa24c` | 已过关 |
| 12 | `src/lib-style/win16-control/basic_paint.c` | `e04652dacf363bb3ad61034d95d2abb16bdb6ee5961a855056e1f2e02cbd73c5` | 已过关 |
| 13 | `src/lib-style/win16-control/basic_paint.h` | `34d6b0857ae92824d63596a060e8191d25e4b95956446f1a57c6b77946186fe6` | 已过关 |
| 14 | `src/lib-style/win16-control/choice_paint.c` | `8859b2a5f78a219711d7feb872f2f8fad7e89a99eacb956c32993851be446900` | 已过关 |
| 15 | `src/lib-style/win16-control/choice_paint.h` | `6efc933f3e2293f106275bcaa13ce66067da445af567a91ef3c35c069677902c` | 已过关 |
| 16 | `src/lib-style/win16-control/choice_view.c` | `c369bc99d500f668a91c8f354503b707e581c4938b709c9469f022e8ed709d42` | 已过关 |
| 17 | `src/lib-style/win16-control/choice_view.h` | `02a9c00c937c411be9e66158a56bbbfdc7dac5f82e07e4591cb24761947bc92d` | 已过关 |
| 18 | `src/lib-style/win16-control/combo_glyph.h` | `75b05a50bf02bbc8fcbd6c6f84948fcdbf82fa7f0fbbfc590a6e4eb94c3ffe34` | 已过关 |
| 19 | `src/lib-style/win16-control/combo_paint.c` | `d7b9ba1aac4a0b3f22086f0e8e323e4f8ed960fa2d04768a29510724cbe9062a` | 已过关 |
| 20 | `src/lib-style/win16-control/combo_paint.h` | `9aff44009ff2d51bb54aa237d8ebf0a4a4b2f295976383147c8aae0c67cbabca` | 已过关 |
| 21 | `src/lib-style/win16-control/combo_text.c` | `a42411b0209b1b008311716bade4d572a1df95d633df6060d0fcdd72819d0451` | 已过关 |
| 22 | `src/lib-style/win16-control/combo_text.h` | `dc8aaafadbdf403f34c37b33829067c7353a72b96c6db233fff8a0b302c118a0` | 已过关 |
| 23 | `src/lib-style/win16-control/control_class.h` | `c9283ac35cd85c856c219c28145cce706b3406d7f99ed74c027599efbca52969` | 已过关 |
| 24 | `src/lib-style/win16-control/control_dispatch.c` | `90df43ca6d849e54deb9986a3b45c778c46b0577bdf10283a931c2adb17d6506` | 待审 |
| 25 | `src/lib-style/win16-control/control_dispatch.h` | `299ead239b40d141908fbce6b776767a45bef6d6f6bad6f2e619fc0b53e830d8` | 待审 |
| 26 | `src/lib-style/win16-control/control_glyphs.h` | `75031b93af2382f8ce1f47d971f32efdfe00cb7000e0db09ef878a1b50adff83` | 待审 |
| 27 | `src/lib-style/win16-control/control_guard.c` | `caf72ecd2a933f48366021d7a62771f69f75f9480c930ea67d4b49f3965f09f1` | 待审 |
| 28 | `src/lib-style/win16-control/control_guard.h` | `3f451ee7f28387d3adeeae88676a819cefa4f99e27d9cd6995cf2b00a7a11e13` | 待审 |
| 29 | `src/lib-style/win16-control/control_install.c` | `7e7618955fc2108280f647562d8e0f7a0c871e4045f3cbc18d57670496efdcf2` | 待审 |
| 30 | `src/lib-style/win16-control/control_install.h` | `dbe7b18324cb3ff529189f43264fb20504d7cb29adfa0f7b22736a65195f2839` | 待审 |
| 31 | `src/lib-style/win16-control/control_lifetime.c` | `f29ae1bb58a0754c8de9e1df6be2d1e743f97eb88020b43d827af6673989e057` | 待审 |
| 32 | `src/lib-style/win16-control/control_lifetime.h` | `1412fcd6165eeeba24d396054cf921e7e3c8500686d401f08b72f17daa336cc2` | 待审 |
| 33 | `src/lib-style/win16-control/control_remove.c` | `27678f0276c7b028cf292bcf53dabdba7289f1a956ccf93838c2b8e28fb78dbb` | 待审 |
| 34 | `src/lib-style/win16-control/control_remove.h` | `dbcd064a957c340b5c296e0c3a7debd67bfc8f46e5e7036f09dd54cda31e9c3f` | 待审 |
| 35 | `src/lib-style/win16-control/control_scope.c` | `f534206388ab958ef31a3addbfced46565dde214b9a20c1935cf28c084ca62af` | 待审 |
| 36 | `src/lib-style/win16-control/control_scope.h` | `d7a2c4f7d8531924c51dd7e493f6e410959315da9598b7b090a4ca99734ffbec` | 待审 |
| 37 | `src/lib-style/win16-control/control_state.c` | `a56c61f2f2603d89cb08b6bf7b97208bc0e2b6a79fd3e855c19e3a4a3343cea8` | 待审 |
| 38 | `src/lib-style/win16-control/control_state.h` | `cccb65a997d5cbabebc7a62577cf79da8678b9754c5373757d64cb74ccff4e78` | 待审 |
| 39 | `src/lib-style/win16-control/cool_grip_identity.c` | `04a9841be43ce2e95553eb3a9cb6a9f094ef03296dec5c86b26981239b2e979c` | 待审 |
| 40 | `src/lib-style/win16-control/cool_grip_identity.h` | `5aa8dce1ae9e718d2a067c7da16fca5f7d07c60a2b5fd2e93e974edfbc4e4e02` | 待审 |
| 41 | `src/lib-style/win16-control/edit_message.c` | `b2004278efe1e65e88bf3e8877cb3fe9704efcea97c69aac7048bd3a3d2e6ce6` | 待审 |
| 42 | `src/lib-style/win16-control/edit_message.h` | `d7c5da65909dc16c9ed2ac58f59dbf34de20af6c10030737d5fae6b316e5ee21` | 待审 |
| 43 | `src/lib-style/win16-control/fileowner_message.c` | `ce1ae8ed053c4b967b52cdfd6eb5cc3f5f9fd1405eb2db4a3b0ffabf16619911` | 待审 |
| 44 | `src/lib-style/win16-control/fileowner_message.h` | `662ee200c53369290027cb9a6e84805235a3b0ead717edcb50ad966063dc76d5` | 待审 |
| 45 | `src/lib-style/win16-control/header_paint.c` | `9bcf190b4606c88c8ba5a9ece6a636682ea8a3a4251b6f820418355c96c0aad2` | 待审 |
| 46 | `src/lib-style/win16-control/header_paint.h` | `0242e8b959c1aa6900e5282708515c55f07bfe0d7bab833935feb06abbf829f3` | 待审 |
| 47 | `src/lib-style/win16-control/list_message.c` | `0ac87da08ab21cf8a95f38a73d40dbee1f26dedea69aec6fd93b7ee5068a57e5` | 待审 |
| 48 | `src/lib-style/win16-control/list_message.h` | `96b05ce3b3a94624aa9327119cb120b25ea7cf631ad736262a4a3754c183a7cb` | 待审 |
| 49 | `src/lib-style/win16-control/list_paint.c` | `9da3e1338e35d9dffdd8ce0facf8ae78053cff5a4ff3d0bd2453f04213aedccf` | 待审 |
| 50 | `src/lib-style/win16-control/list_paint.h` | `108ddfa842255a92e1d92e5e66208970b444fa124b307fc9753ee8107301758c` | 待审 |
| 51 | `src/lib-style/win16-control/month_paint.c` | `9e0735b5b64f94438c9198c781af21770b7edaccf6ff2aa58546fe9ca62b4ccc` | 待审 |
| 52 | `src/lib-style/win16-control/month_paint.h` | `54019ebba59ec1f11e6d2a0d70a2a019c96304d6c440360a61e3b7e30267bf60` | 待审 |
| 53 | `src/lib-style/win16-control/month_text.c` | `cac6b67de87b786a77d44bb0565a3d9b80670af1afd6b615db926a6cd2873ba4` | 待审 |
| 54 | `src/lib-style/win16-control/month_text.h` | `13a533325ee251c26bb2077130bb5e37be6cee9458dbb3c7d3fa53c8f6a1c84a` | 待审 |
| 55 | `src/lib-style/win16-control/native_combo_identity.c` | `8ab3b75c571ed8de36d205e2555578df3232f12b24eb704fb3280f44ce4b75ec` | 待审 |
| 56 | `src/lib-style/win16-control/native_combo_identity.h` | `0e156a50a84184a49ff84d0cc84b8cd0219efe00578f9430e058d62b22b91a92` | 待审 |
| 57 | `src/lib-style/win16-control/native_presentation.c` | `f0feede2f8e65d8fe974d8bb5d0d253b42a503f4f7803449d0583898303d8ad7` | 待审 |
| 58 | `src/lib-style/win16-control/native_presentation.h` | `a944a0f4edaa3748f39c3f6795394f31762bbb5ef1bf91ee0ac81dbe2ba44e30` | 待审 |
| 59 | `src/lib-style/win16-control/notify_message.c` | `9f2332f1b82a71503c22a376498d841323fe55d9656ce895c6961008cabce9ed` | 待审 |
| 60 | `src/lib-style/win16-control/notify_message.h` | `44ac844c460c0a26ce509497a5a8db3315e895e09ff584b44f587421317002df` | 待审 |
| 61 | `src/lib-style/win16-control/other_paint.c` | `d334904841aceed64def25e3159e832a6f139c9070cf64e1e697990bb00d36c7` | 待审 |
| 62 | `src/lib-style/win16-control/other_paint.h` | `94459d8578fcebd60cc401db53ef589b1a7b663ff0ce35b5185167ca79aca278` | 待审 |
| 63 | `src/lib-style/win16-control/progress_paint.c` | `cade3335d0f1efa3065149d3d55b9d7bfabedd3ff63201c6bf50574b8fd73523` | 待审 |
| 64 | `src/lib-style/win16-control/progress_paint.h` | `f43e978a1dfebe98344cb5b3d051186e475777eb5209a73fcdb854f3a6dafa4a` | 待审 |
| 65 | `src/lib-style/win16-control/scroll_assets.c` | `257fa54bcca6b8d8c0f5bdcb091f3002a35d4158c9c3b1cf7c0b87f8ee183a81` | 待审 |
| 66 | `src/lib-style/win16-control/scroll_assets.h` | `a21066150e645b60aa05436dcfa7690fcb413057c5ab8b295cf08b52d7307eb4` | 待审 |
| 67 | `src/lib-style/win16-control/scroll_glyphs.h` | `a748e7d89d52abf6225403244971da442c5b391ab877d1f248ddf2ca96001d40` | 待审 |
| 68 | `src/lib-style/win16-control/scroll_layout.c` | `f675d60e01b5618c61c9e59729ec6dbea6ada802436457ae1cffce8825c856ed` | 待审 |
| 69 | `src/lib-style/win16-control/scroll_layout.h` | `65e112bc5fb72bd770313a2af4dbdde44ef5109f1f75acb56ffd0c20805d97a2` | 待审 |
| 70 | `src/lib-style/win16-control/scroll_message.c` | `a81ec1c71a98c8399028281faf85dd90e174b25d31e811db2d3337d3b897c801` | 待审 |
| 71 | `src/lib-style/win16-control/scroll_message.h` | `ac96754592cbe76b8701fa04915ea18b461991a44b5bb16de44a9c5efa184b69` | 待审 |
| 72 | `src/lib-style/win16-control/scroll_native.c` | `5f3ddf5d2408a8c404acc9cf6874351030274398d5cd6b9ca75d4d1177ec497a` | 待审 |
| 73 | `src/lib-style/win16-control/scroll_native.h` | `722808178add0650d40211ac7f43eecac03d2637a46a9e57e884d968c397345a` | 待审 |
| 74 | `src/lib-style/win16-control/scroll_paint.c` | `7186e6052e5e536e002c956c6505756c7cc924a61c1a95eceb01373b2cb105d5` | 待审 |
| 75 | `src/lib-style/win16-control/scroll_paint.h` | `73430fabd8399933fcde5ed2829e522c4677926efa95b5d83a7e3403a8319fc7` | 待审 |
| 76 | `src/lib-style/win16-control/scroll_track.c` | `72bb6ba21989e4b40be7e1897928003cde4ee23467b3133d8f799b1f30926de9` | 待审 |
| 77 | `src/lib-style/win16-control/scroll_track.h` | `36be02d2e879175c02863c495693d5c79dfa1cbca716419c6e0ec46b59c5934e` | 待审 |
| 78 | `src/lib-style/win16-control/special_install.c` | `ed4869d10cb6bdbec6e0c46d8d121aadc07dd77118b17e1caf07deb17c332103` | 待审 |
| 79 | `src/lib-style/win16-control/special_install.h` | `b4be7bbb66ac51e85b62bfae1c90cb926829b021bd9c726505269572df670f20` | 待审 |
| 80 | `src/lib-style/win16-control/special_paint.c` | `27cfcdf4a57afd036ee3d2bd4f2ae1b542105a98ec7cf3810365638660a399ae` | 待审 |
| 81 | `src/lib-style/win16-control/special_paint.h` | `9eed7cbc5eec9fe4be5a35f3d80e2a90e8f592006404968eb47ebf1df40d9a08` | 待审 |
| 82 | `src/lib-style/win16-control/spin_scope.c` | `1eb516c67ff3c98708004c3220553c37a604096eaf099954fe009cdca0c082b5` | 待审 |
| 83 | `src/lib-style/win16-control/spin_scope.h` | `1e435d595fe40eba8d581186d18d3e0f51ac153d87a5ae79cfb9d7ea282626ce` | 待审 |
| 84 | `src/lib-style/win16-control/status_font.c` | `cce26d4e34b445df76d9623e68717419ae5366e5f016a1367f727e8b802435ce` | 待审 |
| 85 | `src/lib-style/win16-control/status_font.h` | `e987f7856503f0e0164b46d5c5ba3302a3c179defc6153944102a56ea6a54293` | 待审 |
| 86 | `src/lib-style/win16-control/status_frame.c` | `bf2e1b0ce63b68a791288ec5c35011c509c7a89364f70fc8bd40b7b558f84360` | 待审 |
| 87 | `src/lib-style/win16-control/status_frame.h` | `f743c97a782dbb5b9af1e48520823ba99b29ace80c89c3586b7b7554b17a44c3` | 待审 |
| 88 | `src/lib-style/win16-control/status_text.c` | `87595843e4c2cc28436e5375dc9f2bc4541a6b121a71e806bb4015bf1eccfd61` | 待审 |
| 89 | `src/lib-style/win16-control/status_text.h` | `74fb3b53a38e931c7312f6dbfde2e350a13915e2629d9ae853f41da1a495f466` | 待审 |
| 90 | `src/lib-style/win16-control/tab_paint.c` | `851b637c211182d994f27117d4ccde0993e59719916982c2cc95e7d98395603e` | 待审 |
| 91 | `src/lib-style/win16-control/tab_paint.h` | `b23708ff1dbb04542b2174b2e32d4355f4b11391fed9dca7e14f061abb0ad0de` | 待审 |
| 92 | `src/lib-style/win16-control/text_policy.c` | `69096cf8e505c6c8b7c0d4bd25743ba5aa93fdde8d50a922294d8892bd1231af` | 待审 |
| 93 | `src/lib-style/win16-control/text_policy.h` | `50074d3156e5516ac52cb161388629b6a9cc688199b63b2f62bc592475ad04c1` | 待审 |
| 94 | `src/lib-style/win16-control/theme_kind.c` | `5d373e8e069764094be7f09630521fd2b625d57d0c27482e31bfaa50d8ac34d7` | 待审 |
| 95 | `src/lib-style/win16-control/theme_kind.h` | `dd9182da02336ffc68fb36c6c6ded493a57b07719211df12c786dbd2359e87b0` | 待审 |
| 96 | `src/lib-style/win16-control/toolbar_paint.c` | `47bf23e4e8a86b5d7e664e3e57da74d5f4df97503ffeef1583c1bf9610814fa1` | 待审 |
| 97 | `src/lib-style/win16-control/toolbar_paint.h` | `20a149697c1e2336fdba81cc450fe1b01665313ded58b70e1fa25c7f3bb46c62` | 待审 |
| 98 | `src/lib-style/win16-control/trackbar_paint.c` | `68e06057e1b4632e7660dacdc0fdeb578870a5faf263a4b7a3b806debeb5a5c4` | 待审 |
| 99 | `src/lib-style/win16-control/trackbar_paint.h` | `999a58448d0a17ec7afc6283555a7ab10e2cc998c7cdca89b75860cbf0f662a2` | 待审 |
| 100 | `src/lib-style/win16-control/updown_adapter.c` | `01dd0012baeb819e1cf4f843136f579e05ea81d9325430fce2ca5b47a233fd21` | 待审 |
| 101 | `src/lib-style/win16-control/updown_adapter.h` | `2cc302bd4b319c046e70a1fda89c433c40fe8af2feb038a65ee7423d1052a232` | 待审 |
| 102 | `src/lib-style/win16-control/updown_paint.c` | `66925149de796f9f234ade7253a76f4bc5a813731a1aee9022373d75b1882d24` | 待审 |
| 103 | `src/lib-style/win16-control/updown_paint.h` | `228d753f3ea4c5c14f6f6f69af97bbfa778e962160716705ab673b31d6553567` | 待审 |
| 104 | `src/lib-style/win16-control/vb_text.c` | `02e40a52bf3f9b6495826780e43c474a070c598c4403d5fc6a26f6cdba9a2b69` | 待审 |
| 105 | `src/lib-style/win16-control/vb_text.h` | `eae57707ab1fd917877418cc94df4a17dedde1c4968e4d2ad3965f88b6ed5c41` | 待审 |
| 106 | `src/lib-style/win16-control/vcl_extra_frame.c` | `5ab1193a79361c3817d8d4c090bcbfbf69368871142bb49a1cbfae4206af846e` | 待审 |
| 107 | `src/lib-style/win16-control/vcl_extra_frame.h` | `c83a619ef8bb4138c40a588551ec55dd792d3d7e7cbe14ab049f669f3a7f9edc` | 待审 |
| 108 | `src/lib-style/win16-control/vcl_frame_policy.c` | `3178a5f867641cf689f3e5fd549ee4cd00df40cf50b94206d11c6f1490a30e43` | 待审 |
| 109 | `src/lib-style/win16-control/vcl_frame_policy.h` | `f3abc4a76f5e02da02b0ffa9578615ba1b7746a5a4d43390c8035084c37aeb9e` | 待审 |
| 110 | `src/lib-style/win16-control/vcl_identity.c` | `eba6a8b06f9cafad8b0b465c06694541f74d0f287d88c45747af8e04112f9a3c` | 待审 |
| 111 | `src/lib-style/win16-control/vcl_identity.h` | `0d84b3a319f00e86220897002de33db0a7e635392a31e98ba5e94755824335e0` | 待审 |
| 112 | `src/lib-style/win16-control/vcl_native_paint.c` | `92de4090a1b0199510d6428dbe9c03886b09a4a32570155904f0fdc880b85d6d` | 待审 |
| 113 | `src/lib-style/win16-control/vcl_native_paint.h` | `2372cae31d72c1b5d0381905fa077efdcbaf785cb484e8fec6c876731e4aa06d` | 待审 |
| 114 | `src/lib-style/win16-window-uwp/frame.c` | `611847b338b1fddda29fe8997a11c2c3af6436062e2f70963e6394cbdd4eaae6` | 待审 |
| 115 | `src/lib-style/win16-window-uwp/frame.h` | `00da809635b3781756d1760ce368b241259267eeca36e77d99cff117c0c5fbe4` | 待审 |
| 116 | `src/lib-style/win16-window-uwp/frame_layout.c` | `a345511d535bda17c081e1fce4432e2b4d775a532b08b3648fae9533fded3960` | 待审 |
| 117 | `src/lib-style/win16-window-uwp/frame_layout.h` | `50cab3768287cf12f5a60008ce382293a983f26070546ea5033303385f9f41e8` | 待审 |
| 118 | `src/lib-style/win16-window/frame_buttons.h` | `e7a472d76858c3d01fb1501a34d958ccd8337c8d8ccc36893c17ef9222bac0c5` | 待审 |
| 119 | `src/lib-style/win16-window/frame_dispatch.c` | `6b057e074a4444de0f88d9917d90c99b77d7e493877e957b4097085831a274a9` | 待审 |
| 120 | `src/lib-style/win16-window/frame_dispatch.h` | `b94c3a6b62a3904d11eafe719d9d49489a4f9495ee947d2b14310f4de7a932f5` | 待审 |
| 121 | `src/lib-style/win16-window/frame_font.c` | `46964bbb4e795df8818ff7009fe33d9295fec1f39116122cac74af3016308c87` | 待审 |
| 122 | `src/lib-style/win16-window/frame_font.h` | `dcd05c8d415169bc70cd3c6884cf8eec8159e5cf7c0806e4fda546845b30243b` | 待审 |
| 123 | `src/lib-style/win16-window/frame_input.c` | `206a0c8f95f1b972aa4de7b203516c3caca0bc8678b26312b6edbd826bcaf45f` | 待审 |
| 124 | `src/lib-style/win16-window/frame_input.h` | `2725920d1159d39153d0bfac2e7e94d86aa79c8e725fdd1bcb12f0ae8fd45233` | 待审 |
| 125 | `src/lib-style/win16-window/frame_layout.c` | `cf08844671dd5c1b46e43e61abca08255821b7399a90dba11c87bc5b68bf35d3` | 待审 |
| 126 | `src/lib-style/win16-window/frame_layout.h` | `aa4138b097b5a81190eb63fdcde855b0e3659f5d49d6bccd18074755a9147e1b` | 待审 |
| 127 | `src/lib-style/win16-window/frame_lifecycle.c` | `2372e7e3a44488936fac5fd1c66c9a6742191177963338a08c246492e5584db2` | 待审 |
| 128 | `src/lib-style/win16-window/frame_lifecycle.h` | `9471ceb5aab4c90b11becf74dfbe1969bb2faa3526e3eeff956cf25c7cedddb8` | 待审 |
| 129 | `src/lib-style/win16-window/frame_message.c` | `3fb669f9e780edd100c1d0d517cefde41e512db1186766e5441a6c4f5c708445` | 待审 |
| 130 | `src/lib-style/win16-window/frame_message.h` | `dd42ca6758f2e5fc5664848f34081295680653792e39b74399d4e0719314d347` | 待审 |
| 131 | `src/lib-style/win16-window/frame_paint.c` | `0ce5a283cd055ef454254f175b05134c83eec83449ceb438aa97be1226a231f2` | 待审 |
| 132 | `src/lib-style/win16-window/frame_paint.h` | `e9324c74dcd59cfe6c9af7cd1bffc467b4020ddbc1c1da375a57a0546c5eb5fc` | 待审 |
| 133 | `src/lib-style/win16-window/frame_state.c` | `426ea9b1ed8dfc2c524f88da9221d31eb11f30515f7920dd31c4bd882573bde1` | 待审 |
| 134 | `src/lib-style/win16-window/frame_state.h` | `9078c5a86c45fafb6858249c30f2ab0149e5991d9c884e044e78772a12f53576` | 待审 |
| 135 | `src/lib-style/win16-window/frame_view.c` | `55cc044e31a20351a5cf53d5c110c85f7810c82d79dbc0de82de233c22cdaf21` | 待审 |
| 136 | `src/lib-style/win16-window/frame_view.h` | `6748a2c8636037c3dade81b965871c350be5dcc616d61368008746b78efb6ddb` | 待审 |
| 137 | `src/lib-style/win16-window/menu_bar.c` | `47d105dd8bdd47c99e7ff8574c6958b20444b134ad303eb540532017ef6494b6` | 待审 |
| 138 | `src/lib-style/win16-window/menu_bar.h` | `5792fc73118ef17bc7f59700d461a47e4ff6a499d4354b8042e321e8f169abfa` | 待审 |
| 139 | `src/lib-style/win16-window/menu_bar_paint.c` | `392b28274f295dfed1cbf48c464b194784fdb6af055eaff03fc0c530a074bd94` | 待审 |
| 140 | `src/lib-style/win16-window/menu_glyphs.h` | `03922a10b6a7b46e1792852b93f1e1590d45546a57a63cead41fb7b0f0eefa4e` | 待审 |
| 141 | `src/lib-style/win16-window/menu_items.c` | `b71c31aa23818810887e4c5630e80fcb98cb79b1cd03c930290a41cd6b49e162` | 待审 |
| 142 | `src/lib-style/win16-window/menu_items.h` | `8eb73d3ca798cc0561f25c1603c4d94807d1ae154cf17d7bbc07e89148d726da` | 待审 |
| 143 | `src/lib-style/win16-window/menu_iteration.c` | `3f68b0c46f13505d0e141955d2734c40b486e7c476e15ce9213be9bdb119865e` | 待审 |
| 144 | `src/lib-style/win16-window/menu_iteration.h` | `76f030d3ab0efb853d871357a0e4e87b60dc5c3f991f7a93f3b45b5cb14fad5b` | 待审 |
| 145 | `src/lib-style/win16-window/menu_message.c` | `100325c5f1a6030c9a823d6d9edb4a1e53bc5b1435b85208ac5d3c8555727826` | 待审 |
| 146 | `src/lib-style/win16-window/menu_message.h` | `deaedbf7c25bbebf64e892ad7f647cb37a6e19944eef1c73f5dece6673d95b79` | 待审 |
| 147 | `src/lib-style/win16-window/menu_paint.c` | `00df595f7ab3f4471bd5d851930aac1e03d48430c45a8ed8ecb372dbdc108e84` | 待审 |
| 148 | `src/lib-style/win16-window/menu_paint.h` | `590f81afb53614015b31baf3fd06ac3643175ef346c2deee94ea8442a3591e02` | 待审 |
| 149 | `src/lib-style/win16-window/menu_scope.c` | `d10f1420bc829c4ec769cfcc5b627908450f3a76c9e79d615858a8b6a66a58b9` | 待审 |
| 150 | `src/lib-style/win16-window/menu_scope.h` | `e7f130982359c73f3aa790cdbe8915c2acf5aa2a4889dbd5dfda7855a652a699` | 待审 |
| 151 | `src/lib-style/win16-window/win16_style.h` | `5e927143c2ebae5ff80e4b36cb125c5fdd1fbaf3a02d3d76a0a1c9c05765ecd0` | 待审 |

## 001 `win16-common/control_api.h`：已过关

- 职责：为 Win16 DLL 字体与配色声明提供导入标记，不包含跨风格运行时行为。
- 实际调用：`win16-common/font.h` 和 `palette.h`，产品 `ntw16s-dll/api.h`；导出由双位宽 `.def` 文件控制。
- 处置：保留风格侧头文件，产品头改为引用它并删除重复宏；公共库没有反向包含产品私有头。
- 验证：双位宽正式 DLL ABI 检查与 Win16 公共库链接通过；具体资源及未覆盖范围见[第 1 项记录](M0-T26-S1-ordered-01.md)。

## 002 `win16-common/font.c`：已过关

- 内容：两种 Win16 System 位图字体生成、菜单与 VB 字体变体、线程局部租约和资源释放；调用方包括 Win16 窗口、控件、现代外框及产品文本适配。
- 分类：字形选择、缩放、字宽、DPI、菜单和 VB 策略属于 Win16；ANSI FNT v2 字节编码不依赖风格。
- 处置：两处重复序列化抽入 `common/bitmap_font.h`，Win16 原生成路径直接使用；线程局部租约仍绑定 Win16 字体策略。
- 验证：双位宽正式 DLL 的格式、租约及完整字体布局矩阵通过；其他风格字体和真实现代应用未覆盖，见[第 2 项记录](M0-T26-S1-ordered-02.md)。

## 003 `win16-common/font.h`：已过关

- 职责：声明 Win16 位图字体获取、释放、来源映射和风格租约；产品、Win16 窗口及控件依赖这些接口。
- 问题与处置：窗口标题和菜单字体刷新函数实现于 `win16-window/frame_font.c`，其声明原本错误地放在本基础头；已迁至同目录新增的 `frame_font.h`，各实际调用方直接包含该头。字体租约声明保留在 Win16，导出宏仍由顺序审计第 1 项统一。
- 增量验证：双位宽公共基础库、窗口库、控件库链接测试通过；正式同快照构建、发布及复测通过，见[第 3 项记录](M0-T26-S1-ordered-03.md)；真实可见现代窗口不在本次覆盖。

## 004 `win16-common/layout.c`：已过关

- 职责：`win16_style_measure` 和 `win16_style_hit` 实现 Win3.1 特定边框、标题栏按钮尺寸/次序、客户区和命中优先级；Win16 普通外框及现代外框布局调用。
- 处置：保留这些风格策略；通用工作区交集和系统缩放光标已由 `common/frame.h` 提供，不将 Win3.1 参数冒充其他年代的默认值。本次没有变更产品源码或二进制。
- 验证：新增与源文件对应的原生单元测试覆盖非法尺寸、DPI、按钮、外框命中、最大化及无标题窗口；双位宽均通过。双位宽现代外框几何测试覆盖 96–288 DPI，正式 DLL 的自有窗口夹具覆盖六类窗口及最大化/还原。测试副本已清理；真实 UWP/WinUI 外观不在本次覆盖。
- 关联修复：`common_link.c` 对借用的菜单字体改用对应租约释放接口，避免测试直接删除受管理字体。

- 差异统计（不含本台账）：6 个文本文件新增 49 行、删除 4 行；未修改产品二进制，assets 与 O:\winnt 的八文件均与 HEAD 一致。

## 005 `win16-common/layout.h`：已过关

- 职责：声明 Win16 窗口相对坐标结构和对应几何、命中接口，仅依赖 Windows 类型。
- 调用：Win16 普通外框和现代外框布局、绘制及产品命中入口；没有具体产品私有头反向依赖。结构包含 Win16 特有的系统菜单、最小化和最大化几何，保留在风格侧，不作为其他年代通用数据结构。
- 处置：无需源码修改。双位宽 `core-win16-layout` 与 `core-uwp-frame_layout` 均通过；前者直接覆盖声明的接口，后者覆盖现代外框 96–288 DPI 几何。该测试不证明可见 UWP/WinUI 或其他风格效果。产品文件未变。

## 006 `win16-common/palette.c`：已过关

- 职责与调用：提供 Win16 默认色、按系统高对比度映射 Win16 颜色角色、设置对话框 DC 的颜色；普通窗口、菜单、控件、现代外框及产品适配器使用相应入口。
- 处置：默认值和颜色角色属于 Win16 风格；高对比度查询目前没有其他相同调用方，单独抽取查询会产生无实际复用的包装，暂留原处。Win2000 的系统色处理用于帧按钮映射，契约不同。本项未更改产品源码。
- 验证：新增对应源文件的原生测试，双位宽检查全部默认值、当前对比度分支及对话框 DC 颜色和画刷，均通过；当前系统高对比度关闭，开启分支未运行，不更改系统设置。

## 007 `win16-common/palette.h`：已过关

- 职责与调用：声明 Win16 的八色角色和配色接口；普通外框、菜单、控件、现代外框及产品适配通过此契约交换颜色。
- 处置：接口描述具体 Win16 风格，不反向包含产品私有头；目前其他风格并无相同色彩契约，保留在风格库，避免提前固定不适用于其他主题的公共接口。
- 验证：第 6 项的双位宽原生配色测试直接包含该头并调用所声明接口，均通过；高对比度开启未覆盖。

## 008 `win16-control/attachment_guard.c`：已过关

- 文件混合控件附着安装及撤销、绘制期分派、运行时类别与原生过程核验；产品入口集中在 `ntw16s-dll/attachment.c`。基线 51 个公开函数已分类，详情和留在风格侧的具体理由见[函数盘点](M0-T26-S1-ordered-08-inventory.md)。
- 已抽出：ANSI 和 Unicode 两条原生窗口过程转发路径共用的过程及调用模块归属核验迁入 `common/control_native_witness.h`，原路径直接调用，控件类型及实际绘制策略仍由 Win16 决定。
- 验证：双位宽共享校验正反例、Win16 DLL 增量构建及 MinGW/Ninja/ccache 双位宽干净打包通过；先发布到 assets 和 O:\winnt，再用正式启动器运行双位宽 ANSI DBEdit 与 Unicode 轨迹条自建夹具，均通过，含伪装、外部调用、重入移除和资源清理。发布明细见[交付记录](M0-T26-S1-ordered-08-witness.md)。真实第三方程序未覆盖。
- 已按 51 个公开函数分组列明生命周期、识别和绘制边界，以及尚需核查的共享事务和剪裁机制，见[函数盘点](M0-T26-S1-ordered-08-inventory.md)。
- 打印 DC 剪裁快照的捕获、应用与资源释放已迁入公共 `clip.h`，Win16 实际接入；双位宽干净构建、发布和正式包回归通过，见[交付记录](M0-T26-S1-ordered-08-clip.md)。第 9 项头文件因接口接线发生必要修改，哈希已更新但尚未审计。
- VCL MDI 的窗口关系、线程、模块和样式位核验迁入公共控件身份组件，原 Win16 函数删除；调用方提供宿主附着判断。双位宽干净构建、发布和正式包回归通过，见[交付记录](M0-T26-S1-ordered-08-mdi.md)。
- VCL CoolBar 的窗口、线程和模块归属验证迁入公共控件身份组件，原 Win16 函数删除；调用方提供当前附着对象判断。双位宽干净构建、发布和正式包回归通过，见[交付记录](M0-T26-S1-ordered-08-cool.md)。

## 009 `win16-control/attachment_state.h`：已过关

- 职责与调用：定义 Win16 的控件附着状态、候选角色、字体、重入引用，以及第 8 项各分派和生命周期回调契约；直接由 `attachment_guard.c`、产品 `ntw16s-dll/attachment.c` 和控件独立链接测试包含。
- 处置：状态组合及安装回调依赖 Win16 调色、字体和候选策略，保留风格侧；独立通用协议、剪裁快照和身份检查已由公共头提供。该头不反向包含产品私有头；将整个状态结构上移会把 Win16 角色和资源策略暴露给其他风格。本次无产品源码修改。
- 验证：双位宽重新编译并运行 `core-win16-control-link`，独立链接、附着资格和分派优先级通过；第 8 项的双位宽产品路径及失败/重入回归见[函数盘点](M0-T26-S1-ordered-08-inventory.md)。不表示所有状态组合、真实第三方程序或其他风格已验证。

## 010 `win16-control/basic_message.c`：已过关

- 职责与调用：`win16_control_basic_message` 由 `control_dispatch.c` 调用，为 Win16 基本控件协调原生消息、位图 STATIC、VB Frame、绘制和重绘时机；画笔及呈现由调用方提供。
- 处置：原生重绘抑制及可见性保护已使用 `common/control_redraw.h`，类名检查在公共 `control_class.h`；消息路由、何时覆盖原生绘制及 Win16 状态属于风格策略。位图打印的局部 DC 保存和恢复与跨消息剪裁快照契约不同，无需为了单次 GDI 调用增加包装。本项未修改产品源码。
- 验证：当前正式包双位宽 `core-control-visibility` 各 16 种父子可见性组合通过，`core-vcl-static` 的原生嵌套、伪装拒绝和字体保留通过；第 9 项独立链接测试覆盖分隔线识别。位图 STATIC 的打印分支未单独覆盖，真实 VB 程序和其他风格未验证。

## 011 `win16-control/basic_message.h`：已过关

- 职责与调用：声明使用 Win16 `control_style` 及绘制回调的基本消息处理器，实现与实际分派在 `basic_message.c` 和 `control_dispatch.c`；接口没有产品私有头反向依赖，保留风格侧。
- 处置：产品 `ntw16s-dll/controls.c` 不直接使用该接口，已删除多余包含；公共重绘机制仍由第 10 项所述公共组件提供。双位宽增量编译通过，修改前后产品 `controls.c.obj` 的 SHA-256 分别为 x86 `775c1bba9b4300c5f7329ad4bb0c50278f204f2e2f496a67700017ae2b24ac71` 和 x64 `d4e02b9731f27d56a7686b12fc16fef95990e5ceb38812ecd4ee77172021c8ef`，逐字节一致；沿用既有发布包。第 10 项双位宽正式包回归适用，未单独覆盖的位图打印仍保留。

## 012 `win16-control/basic_paint.c`：已过关

- 职责：绘制 Win16 组合框箭头、按钮边框、滚动条箭头及实色矩形边框。箭头图案、按钮配色与立体边属于 Win16 策略；实色矩形的内缩绘制不依赖风格。
- 已抽出：实色边框迁入 `common/paint_frame.h`，组合框、列表、其他基本控件、滚动条和特殊控件直接使用新入口；对应未审文件仅更换调用并更新摘要，尚未计入已审。双位宽干净构建、发布及正式包回归通过，见[交付记录](M0-T26-S1-ordered-12-frame.md)。
- 箭头和选择框图案使用相同的按比例像素网格，通用绘制迁入 `common/pixel_grid.h`，Win16 保留原始图案、颜色和透明规则；`choice_paint.c` 仅接入并更新摘要，尚待按顺序审计。双位宽干净构建、发布及正式包回归通过，见[交付记录](M0-T26-S1-ordered-12-grid.md)。全部 DPI、真实第三方程序和其他风格未覆盖。

## 013 `win16-control/basic_paint.h`：已过关

- 职责与调用：声明 Win16 按钮立体边、滚动条和组合框箭头绘制接口，由相应控件绘制文件调用；依赖 Win16 调色板，保留风格侧。
- 处置：公共实色边框头此前通过此头间接暴露，现改为六个实际绘制调用文件直接包含，避免风格接口携带无关的公共实现；其他未审文件只调整包含并更新摘要，尚未计入审计完成。
- 验证：双位宽增量编译和 `core-win16-control-link` 通过；六个直接调用文件修改前后各位宽目标文件的 SHA-256 全部一致。沿用第 12 项已发布包及其双位宽像素、控件回归；未验证所有 DPI、真实第三方程序或其他风格。

## 014 `win16-control/choice_paint.c`：已过关

- 职责：Win16 选择图案、清单绘制目标核验和 VB 清单位图处理；产品拦截入口、控件绘制及附着通知均有调用。图案到 DC 的普通像素网格已由公共 `pixel_grid.h` 提供；DC 坐标和 VB 类身份分别使用公共 `dc_geometry.h` 与 `control_class.h`。
- 验证：当前发布包的双位宽清单模型与 VB 清单夹具通过，覆盖 96/144 DPI、像素、错误来源、禁用、重入和父窗口变化；测试使用直接工作入口，无隔离桌面。真实 VB 程序尚未覆盖。
- DIB 路径：索引像素缩放、临时缓冲和 `SetDIBitsToDevice` 已迁入公共 `pixel_dib.h`，保留独立于 `FillRect` 的 DC 契约；Win16 保留原图案及尺寸限制。双位宽干净构建、发布和正式包回归通过，见[交付记录](M0-T26-S1-ordered-14-dib.md)。清单目标资格、VB 运行时与尺寸策略继续保留风格侧；真实 VB 程序和其他风格未验证。

## 015 `win16-control/choice_paint.h`：已过关

- 职责与调用：声明 Win16 清单资格、VB 清单和选择图案接口；`choice_view.c`、`attachment_guard.c`、`special_paint.c` 和产品文本注入入口直接使用。接口依赖 Win16 调色与清单策略，保留风格侧；通用像素网格及 DIB 接口位于公共组件。
- 处置与验证：移除产品 `controls.c` 与 `attachment.c` 中未使用的此头包含。双位宽增量构建通过，两个产品源文件在两种位宽的四个目标文件修改前后 SHA-256 全部一致；沿用第 14 项已发布包和双位宽、96/144 DPI 清单回归。真实 VB 程序和其他风格仍未覆盖。

## 016 `win16-control/choice_view.c`：已过关

- 职责：产品 `controls.c` 构造视图后调用，用 Win16 调色、图案、字体和布局绘制静态、按钮及选择控件。
- 已抽出：静态图标和按钮图标重复的尺寸读取及 `GetIconInfo` 位图释放迁入公共 `icon_size.h`，两个实际调用方直接使用；`other_paint.c` 仅调整接线和摘要，仍待按顺序审计。双位宽干净构建、发布及正式包回归通过，见[交付记录](M0-T26-S1-ordered-16-icon.md)。
- 选择控件与组合框重复的窗口文本分配和读取迁入公共 `window_text.h`，调用方仍负责释放；组合框文件仅改接线并更新摘要，尚待按顺序审计。双位宽干净构建、发布及正式包回归通过，见[交付记录](M0-T26-S1-ordered-16-text.md)。剩余图案选择、颜色、字体、位置和焦点绘制为 Win16 风格策略；静态图标产品路径、真实第三方程序和其他风格未单独验证。

## 017 `win16-control/choice_view.h`：已过关

- 职责与调用：定义选择控件绘制所需的 Win16 调色、字体、脱离状态和 VB/按钮角色回调；产品 `controls.c` 构造视图，`choice_view.c` 消费。回调承载产品识别政策，避免公共组件依赖产品私有头。
- 处置与验证：此视图绑定 Win16 调色与选择控件绘制，保留风格侧；没有需要复制或上移的通用绘制机制。本项无源码修改。双位宽重新链接并运行 `core-win16-control-link` 通过；第 16 项双位宽正式包选择控件回归通过，静态图标产品路径、真实第三方程序和其他风格未单独验证。

## 018 `win16-control/combo_glyph.h`：已过关

- 职责与调用：Windows 3.1 VGA.DRV 的 7×9 组合框箭头资源；`basic_paint.c` 提供 Win16 箭头，组合框与 VCL 按钮等路径使用，集成测试也直接引用原图案。
- 处置与验证：该数据是特定年代的受保留原图形资源，不能当作所有主题默认图案；按来源策略原位逐字节保留。可跨风格复用的缩放网格已位于 `common/pixel_grid.h`。资源身份核验通过，双位宽像素网格及正式包组合框回归见[第 12 项记录](M0-T26-S1-ordered-12-grid.md)；其他风格与全部 DPI 未验证。本项无源码修改。

## 019 `win16-control/combo_paint.c`：已过关

- 职责：组合框的 Win16 颜色、箭头、字体、焦点及原生编辑框裁剪策略保留风格侧；文本快照、实色边框及箭头像素绘制此前已共用。
- 本次把组合框和滑块重复的非镜像像素坐标及两个零原点检查迁入 `common/dc_geometry.h`；保持原有检查顺序和非 RTL 布局语义。滑块仅改接线，仍按原序待完整审计。
- 双位宽干净构建，先发布再运行 DC 边界、组合框及滑块自建夹具均通过；资源和公共依赖检查通过。真实第三方应用、其他风格和全部 DPI 未验证。详见[交付记录](M0-T26-S1-ordered-19-dc.md)。

## 020 `win16-control/combo_paint.h`：已过关

- 声明 Win16 组合框绘制及原生按钮绘制接口；分别由产品 `controls.c` 和风格模块 `native_combo_identity.c` 调用。接口绑定 Win16 调色板和绘制策略，保留风格侧；坐标、文本、实色边框和像素机制使用公共组件。
- 清除产品 `attachment.c` 未使用的该头包含；双位宽增量构建通过，修改前后该文件的两个目标文件 SHA-256 均逐字节一致。沿用第 19 项正式包测试；真实第三方应用和其他风格未验证。

## 021 `win16-control/combo_text.c`：已过关

- 组合框子编辑器的字体消息分派、绘制期间借用、解绑及重入恢复依赖 `control_style` 和 Win16 字体租约；当前保留风格侧。通用子类所有权检查已经使用 `common/control_scope.h`。
- 与第 31 项 `control_lifetime.c` 都在原生 `WM_SETFONT` 返回后恢复字体；两者对后继子类的识别不同，应在按序审计第 31 项时一起核实是否能抽象，不能把待验证的重复当作已经共享。
- 删除已由自有头声明的重复前置声明，双位宽增量构建前后两个目标文件摘要逐字节一致。用正式发布包运行双位宽自建 VB 组合框夹具，覆盖 40 个字体、原字体、解绑、设置字体期间重入及重设父窗口，均通过；只覆盖当前 96 DPI、合成运行时，真实 VB 和其他风格未验证。

## 022 `win16-control/combo_text.h`：已过关

- 职责与调用：声明组合框子编辑器的消息处理和解绑接口，产品 `controls.c` 传入自身子类过程，`combo_text.c` 实现；参数使用 Win16 `control_style`，当前不属于无风格公共接口。第 21 项所记与第 31 项的字体恢复机制仍须按序核查。
- 本文件没有改动；双位宽 Win16 控件独立链接测试通过，字体解绑及重入覆盖见第 21 项；不代表其他风格或真实 VB 已验证。

## 023 `win16-control/control_class.h`：已过关

- 公共 `control_class.h` 已实现 VB 类及运行时身份识别；本文件另含 Win16 绘制资格（控件样式、RTL、清单 96/144 DPI 限制）以及 VB Frame 按钮类型映射。须逐函数判断语义能否与绘制器资格拆开，不能整体移动。
- 直接使用正式发布包和自建窗口运行 96 DPI VB5/VB6 清单夹具：VB5 x86 首次在重入重设父窗口绘制断言失败、重试通过，VB5 x64 通过；VB6 x86 在同一断言失败，VB6 x64 在重入样式变化后的像素断言失败。错误码 126 不能据此认定根因；失败和不稳定项不计通过，仍需复核。未创建隔离桌面。
- 后续对照：同一测试、自建窗口及 VB5/VB6 夹具在无样式时双位宽均通过；因此未能把失败归因于夹具的无样式基线，尚不能确定具体根因。
- 在审计过程中，VB Frame 的按钮分组类型映射已移入 `common/control_class.h`，两个 Win16 生产调用方接入；其他尚未审计的调用文件仅调整接线。双位宽干净构建、先发布和正式包自建 VB Frame/普通按钮夹具通过，见[交付记录](M0-T26-S1-ordered-23-class.md)；清单重入失败仍未解决，作为独立未通过项保留。
- 新增失败诊断并复测：可见清单仍可能得到 `GetClipBox` 的 `NULLREGION`，此时 `RedrawWindow` 成功但无绘制回调；重入换父窗口失败采样为 redraw=1、observations=0、父窗口未变、裁剪区为空，故该次触发根本未运行。另一次像素不匹配后裁剪区亦为空，但事后状态不足以证明所有像素失败原因。临时置顶实验仍有失败，已撤销；没有放宽测试断言。双位宽各有完整通过和失败，仍视为不稳定。
- 新增无需屏幕绘制的原生身份测试，使用自建 VB6 运行时和隐藏窗口：双位宽覆盖普通及图形复选/单选、owner-draw、错误运行时、Frame 类型及 96 DPI 清单资格和 RTL 拒绝，均通过；VB5、目录/驱动器、图片框、144 DPI 和真实 VB 未由此测试覆盖。清单实际绘制和重入的不稳定结果依然开放。
- 逐函数归类：清单与文件控件的固定 owner-draw 模式和 DPI、普通/图形选择控件的渲染资格、图片框及图形按钮的外框和 RTL 限制属于当前 Win16 绘制能力；Frame 的语义类型已共享。公共类名与运行时身份已由 `common/control_class.h` 提供，不重复复制。
- 补充自建 VB5/VB6 隐藏窗口测试，双位宽覆盖目录、驱动器和图片框的资格、粗边框和 RTL 拒绝；连同基础身份测试，10 项全部通过。144 DPI、真实 VB 和其他风格仍未覆盖。清单实际绘制和重入不稳定作为独立未解决项继续开放，不能由身份测试代替。
