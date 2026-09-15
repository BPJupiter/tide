// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

C_LINKAGE_BEGIN
TI_Vocab_Info ti_vocab_info_table[7] =
{
{str8_lit_comp("window"), str8_lit_comp("windows"), str8_lit_comp("Window"), str8_lit_comp("Windows"), TI_IconKind_Window},
{str8_lit_comp("geo3d"), str8_lit_comp(""), str8_lit_comp("Geometry (3D)"), str8_lit_comp(""), TI_IconKind_Cube},
{str8_lit_comp("exit"), str8_lit_comp(""), str8_lit_comp("Exit"), str8_lit_comp(""), TI_IconKind_X},
{str8_lit_comp("run_command"), str8_lit_comp(""), str8_lit_comp("Run Command"), str8_lit_comp(""), TI_IconKind_Null},
{str8_lit_comp("wm_event"), str8_lit_comp(""), str8_lit_comp("OS Event"), str8_lit_comp(""), TI_IconKind_Null},
{str8_lit_comp("open_window"), str8_lit_comp(""), str8_lit_comp("Open New Window"), str8_lit_comp(""), TI_IconKind_Window},
{str8_lit_comp("close_window"), str8_lit_comp(""), str8_lit_comp("Close Window"), str8_lit_comp(""), TI_IconKind_Window},
};

TI_Name_Schema_Info ti_name_schema_info_table[27] =
{
{str8_lit_comp("user"), 0, str8_lit_comp("@expand_commands(edit_user_theme) x:\n{\n  //- rjf: animations\n  @display_name('Animations') @description(\"Enables animations.\")\n  @default(1) 'animations': bool,\n  @display_name('Scrolling Animations') @description(\"Enables scrolling animations.\")\n  @expand_if(\"$.animations\") @default(1) 'scrolling_animations': bool,\n  @display_name('Tooltip Animations') @description(\"Enables tooltip animations.\")\n  @expand_if(\"$.animations\") @default(1) 'tooltip_animations': bool,\n  @display_name('Menu Animations') @description(\"Enables menu animations.\")\n  @expand_if(\"$.animations\") @default(1) 'menu_animations': bool,\n\n  //- rjf: fonts\n  @display_name('Use Alternative Font For UI') @description(\"Uses an alternative font for non-code UI elements.\")\n  @default(1) 'use_alternative_font_for_ui': bool,\n  @expand_if(\"$.use_alternative_font_for_ui\") @display_name('UI Font') @description(\"The name of, or path to, the font used when displaying non-code UI elements.\")\n  @default('') 'main_font': string,\n  @expand_if(\"$.use_alternative_font_for_ui\") @display_name('Code Font') @description(\"The name of, or path to, the font used when displaying code.\")\n  @default('') 'code_font': string,\n  @expand_if(\"!$.use_alternative_font_for_ui\") @display_name('Font') @description(\"The name of, or path to, the font used in all UI.\")\n  @default('') 'font': string,\n\n  //- rjf: theme\n  @default(\"Default (Dark)\") @display_name('User Theme')\n  @description(\"The user's theme, which describes all colors used throughout the UI.\")\n  'theme': string,\n  @no_expand @display_name('User Theme')\n  'theme_colors': set,\n\n\n  //- rjf: cursor decorations\n  @default(1) @display_name('Cursor Trail') @expand_if(\"$.animations\") @description(\"Controls whether or not a movement trail animation of the cursor is drawn.\")\n  'cursor_trail': bool,\n\n  //- rjf: occluding background settings\n  @default(0) @display_name('Opaque Backgrounds') @description(\"Controls whether or not all floating background colors are forced to be fully opaque.\")\n  'opaque_backgrounds': bool,\n  @default(1) @display_name('Background Blur') @description(\"Controls whether or not occluded regions behind floating elements are blurred.\")\n  'background_blur': bool,\n\n  //- rjf: appearance settings\n  @default(1) @display_name('Drop Shadows') @description(\"Controls whether or not drop shadows are drawn.\")\n  'drop_shadows': bool,\n  @default(1.f) @display_name('Rounded Corner Amount') @description(\"Controls the degree to which UI corners are rounded.\")\n  'rounded_corner_amount': @range[0, 1] f32,\n\n  //- rjf: code formatting settings\n  @default(2) @display_name('User Tab Width') 'tab_width': @range[1, 32] u64,\n\n  //- rjf: windows style menu bar\n  @default(1) @display_name('Focus Menu Bar With Alt') @description(\"Mimics standard Windows behavior of focusing the menu bar using the Alt key.\")\n  'focus_menu_bar_with_alt': bool,\n\n  //- rjf: native filesystem dialogues\n  @default(0) @display_name('Use Native File System Dialog') @description(\"Uses the operating system's file system dialog box, rather than the debugger's built-in UI.\")\n  'use_native_file_system_dialog': bool,\n\n  //- rjf: transient tabs\n  @default(1) @display_name('Transient Tabs') @description(\"When snapping to source code locations, opens new files in a 'transient' tab if they are not already open. Transient tabs are replaced on subsequent snaps automatically.\")\n  'transient_tabs': bool,\n\n  //- rjf: auto-load last project\n  @default(0) @display_name('Auto Load Last Project') @description(\"Enables loading the most recently opened project, if one is not specified from the command line.\")\n  'auto_load_last_project': bool,\n\n  //- rjf: update check\n  @default(1) @display_name('Check For Updates') @description(\"Enables a small network request to check if there is a newer release version available.\")\n  'check_for_updates': bool,\n\n  //- rjf: system installation\n  @display_name('Install To System') @description(\"Controls whether or not associated installation files are created on the system, for tighter integration with the operating system shell.\") 'install_to_system': bool,\n}\n")},
{str8_lit_comp("project"), 0, str8_lit_comp("@expand_commands(edit_project_theme) x:\n{\n  @display_name('Project Name') 'name': string,\n  @default(2) @display_name('Project Tab Width') 'tab_width': @range[1, 32] u64,\n\n\n  //- rjf: theme\n  @default(\"None\") @display_name('Project Theme') @description(\"The project's theme, which describes all colors used throughout the UI, and can override the user's theme.\")\n  'theme': string,\n  @no_expand @display_name('Project Theme') @description(\"The project's theme, which describes all colors used throughout the UI, and can override the user's theme.\")\n  'theme_colors': set,\n\n}\n")},
{str8_lit_comp("theme_color"), 0, str8_lit_comp("@collection_commands(add_theme_color, fork_theme, save_theme, save_and_set_theme)\n@row_commands(duplicate_cfg, remove_cfg)\nx:\n{\n  @display_name('Tags') tags: string,\n  @display_name('Value') value: @color @hex u32,\n}\n")},
{str8_lit_comp("window"), 0, str8_lit_comp("x:\n{\n  //- rjf: text rasterization settings\n  @default(1) @display_name('Smooth UI Text') @description(\"Controls whether or not UI text is fully anti-aliased, for a smoother appearance.\")\n  'smooth_ui_text': bool,\n  @default(1) @display_name('Hint UI Text') @description(\"Controls whether or not UI text is hinted, for better text readability at small sizes.\")\n  'hint_ui_text': bool,\n  @default(0) @display_name('Smooth Code Text') @description(\"Controls whether or not code text is fully anti-aliased, for a smoother appearance.\")\n  'smooth_code_text': bool,\n  @default(1) @display_name('Hint Code Text') @description(\"Controls whether or not code text is hinted, for better text readability at small sizes.\")\n  'hint_code_text': bool,\n  @default(11) @display_name('Window Font Size') @description(\"Controls the window's default font size. Does not apply to tabs with their own font size set.\")\n  'font_size': @range[6, 72] u64,\n\n  //- rjf: size settings\n  @default(3.f) @display_name('Window Row Height') @description(\"Controls the window's default row height, in multiples of the font size. Does not apply to tabs with their own row height set.\")\n  'row_height': @range[1.75f, 5.f] f32,\n  @default(3.f) @description(\"Controls the height of tabs, in multiples of the font size.\")\n  'tab_height': @range[1.75f, 5.f] f32,\n\n  //- rjf: theme settings\n  @default(1) @display_name('Use Project Theme') @description(\"Prefer using the project theme for this window, if any. If off, only the user's theme settings will be used.\")\n  'use_project_theme': bool,\n}\n")},
{str8_lit_comp("tab"), 0, str8_lit_comp("@row_commands(@file copy_tab_full_path, @file show_file_in_explorer, duplicate_tab, close_tab)\nx:\n{\n  @override @display_name('Tab Font Size') @description(\"Controls the tab's font size.\") @no_callee_helper\n  'font_size': @range[6, 72] u64,\n}\n")},
{str8_lit_comp("bin"), 1, str8_lit_comp("x:{ @description('An expression to which the 2-radix display should apply.') 'expression': expr_string }")},
{str8_lit_comp("oct"), 1, str8_lit_comp("x:{ @description('An expression to which the 8-radix display should apply.') 'expression': expr_string }")},
{str8_lit_comp("hex"), 1, str8_lit_comp("x:{ @description('An expression to which the 16-radix display should apply.') 'expression': expr_string }")},
{str8_lit_comp("digits"), 1, str8_lit_comp("x:{ @description('An expression to which the digit display should apply.') 'expression': expr_string, @description('The minimum number of digits to display.') 'digit_count': u64 }")},
{str8_lit_comp("no_string"), 1, str8_lit_comp("x:{ @description('The expression for which string visualization should be disabled.') 'expression': expr_string}")},
{str8_lit_comp("no_char"), 1, str8_lit_comp("x:{ @description('The expression for which character visualization should be disabled.') 'expression': expr_string}")},
{str8_lit_comp("no_addr"), 1, str8_lit_comp("x:{ @description('The expression for which address visualization should be disabled.') 'expression': expr_string}")},
{str8_lit_comp("sequence"), 1, str8_lit_comp("x:{ @description('The number of expansions to generate.') 'count': expr_string}")},
{str8_lit_comp("rows"), 1, str8_lit_comp("x:{ @description('The expression for which rows should be generated.') 'expression': expr_string, @description('A list of expressions which will be used to generate rows.') '...'}")},
{str8_lit_comp("columns"), 1, str8_lit_comp("x:{ @description('The expression for whose expansions columns should be generated.') 'expression': expr_string, @description('A list of expressions which will be used to generate columns for each expansion from `expression`.') '...'}")},
{str8_lit_comp("omit"), 1, str8_lit_comp("x:{ @description('The expression for which certain members should be omitted.') 'expression': expr_string, @description('A list of member names to omit from the expansion.') '...'}")},
{str8_lit_comp("range1"), 1, str8_lit_comp("x:{ @description('The expression which should be bounded by `min` and `max`.') 'expression': expr_string, @description('The lower bound.') min, @description('The upper bound.') max}")},
{str8_lit_comp("array"), 1, str8_lit_comp("x:{ @description('An expression of the base address of the array.') 'base_address': expr_string, @description('The number of elements in the array.') count}")},
{str8_lit_comp("slice"), 1, str8_lit_comp("x:{ @description('An expression of a structure that is to be interpreted as a slice.') 'expression': expr_string}")},
{str8_lit_comp("list"), 1, str8_lit_comp("x:\n{\n  @description(\"An expression describing the first node in the list.\")\n  'expression': expr_string,\n  @order(0) @description(\"The name of the member which encodes the link to the next node.\")\n  'member_name': code_string,\n}\n")},
{str8_lit_comp("watch"), 0, str8_lit_comp("@inherit(tab) x:\n{\n  @override @display_name('Tab Row Height') @description(\"Controls the tab's row height, in multiples of the font size.\")\n  'row_height': @range[1.75f, 5.f] f32,\n  'label': code_string,\n  @description(\"The root expression which is evaluated to produce the watch window.\")\n  'expression': expr_string,\n  @no_expand 'watch_expressions': set,\n}\n")},
{str8_lit_comp("text"), 1, str8_lit_comp("@inherit(tab) @expand_commands(@output clear_output) x:\n{\n  @description(\"An expression to describe data which should be viewed as text or code.\")\n  'expression': expr_string,\n  @optional @description(\"The language that the text should be interpreted as being within. Used for syntax highlighting and other parsing features.\")\n  'lang': code_string,\n  @no_callee_helper @default(1) @description(\"Controls whether or not line numbers are shown.\")\n  'show_line_numbers':bool,\n  @no_callee_helper @default(1) @display_name('Line Wrapping') @description(\"Splits textual lines into multiple visual lines, so that all text is within the visible area.\")\n  'line_wrapping': bool,\n  @no_callee_helper @default(0) @display_name('Scroll To Bottom On Change') @description(\"Scrolls to the bottom if the text is changed.\")\n  'scroll_to_bottom_on_change': bool,\n  @no_callee_helper @no_revert @default(0) @display_name('Transient') @description(\"Controls whether or not this tab will be automatically replaced by the debugger when it snaps to new source code locations.\")\n  'auto': bool,\n}\n")},
{str8_lit_comp("color"), 1, str8_lit_comp("@inherit(tab) x:\n{\n  @display_name(\"Value\") @description(\"An expression to describe the value or location of the color.\")\n  'expression': expr_string,\n}\n")},
{str8_lit_comp("geo3d"), 1, str8_lit_comp("@inherit(tab) x:\n{\n  @display_name(\"Expression\") @description(\"An expression to describe the base address of the index buffer.\")\n  'expression': expr_string,\n  'count': expr_string,\n  'vtx': expr_string,\n  'vtx_size': expr_string,\n  'yaw': @range[0, 1] f32,\n  'pitch': @range[-0.5, 0] f32,\n  'zoom': @range[0, 100] f32,\n}\n")},
{str8_lit_comp("getting_started"), 0, str8_lit_comp("@inherit(tab) x:\n{\n}\n")},
{str8_lit_comp("recent_project"), 0, str8_lit_comp("x:{'path':path, 'name':string}")},
{str8_lit_comp("machine"), 0, str8_lit_comp("x:{'label':code_string, @no_expand 'active':bool, 'unattached_processes':set, 'processes':set}")},
};

String8 ti_reg_slot_code_name_table[4] =
{
{0},
str8_lit_comp("window"),
str8_lit_comp("cmd_name"),
str8_lit_comp("wm_event"),
};

Rng1u64 ti_reg_slot_range_table[4] =
{
{0},
{OffsetOf(TI_Regs, window), OffsetOf(TI_Regs, window) + sizeof(CFG_ID)},
{OffsetOf(TI_Regs, cmd_name), OffsetOf(TI_Regs, cmd_name) + sizeof(String8)},
{OffsetOf(TI_Regs, wm_event), OffsetOf(TI_Regs, wm_event) + sizeof(WM_Event *)},
};

TI_Cmd_Kind_Info ti_cmd_kind_info_table[6] =
{
{0},
{ str8_lit_comp("exit"), str8_lit_comp("Exits the TIDE program."), TI_CmdKindFlag_ListInUI*1},
{ str8_lit_comp("run_command"), str8_lit_comp("Runs a command from the command palette."), TI_CmdKindFlag_ListInUI*0},
{ str8_lit_comp("wm_event"), str8_lit_comp(""), TI_CmdKindFlag_ListInUI*0},
{ str8_lit_comp("open_window"), str8_lit_comp("Opens a new window."), TI_CmdKindFlag_ListInUI*1},
{ str8_lit_comp("close_window"), str8_lit_comp("Closes an opened window."), TI_CmdKindFlag_ListInUI*1},
};

String8 ti_icon_kind_text_table[75] =
{
str8_lit_comp(""),
str8_lit_comp("b"),
str8_lit_comp("c"),
str8_lit_comp("B"),
str8_lit_comp("C"),
str8_lit_comp("f"),
str8_lit_comp("F"),
str8_lit_comp("g"),
str8_lit_comp("h"),
str8_lit_comp("r"),
str8_lit_comp("s"),
str8_lit_comp("i"),
str8_lit_comp("w"),
str8_lit_comp("W"),
str8_lit_comp("k"),
str8_lit_comp("K"),
str8_lit_comp("L"),
str8_lit_comp("R"),
str8_lit_comp("U"),
str8_lit_comp("D"),
str8_lit_comp("G"),
str8_lit_comp("P"),
str8_lit_comp("3"),
str8_lit_comp("p"),
str8_lit_comp("O"),
str8_lit_comp("o"),
str8_lit_comp("!"),
str8_lit_comp("1"),
str8_lit_comp("V"),
str8_lit_comp("<"),
str8_lit_comp(">"),
str8_lit_comp("^"),
str8_lit_comp("v"),
str8_lit_comp("9"),
str8_lit_comp("0"),
str8_lit_comp("7"),
str8_lit_comp("8"),
str8_lit_comp("+"),
str8_lit_comp("-"),
str8_lit_comp("'"),
str8_lit_comp("\""),
str8_lit_comp("M"),
str8_lit_comp("."),
str8_lit_comp("x"),
str8_lit_comp("q"),
str8_lit_comp("j"),
str8_lit_comp("u"),
str8_lit_comp("m"),
str8_lit_comp("n"),
str8_lit_comp("l"),
str8_lit_comp("a"),
str8_lit_comp("z"),
str8_lit_comp("y"),
str8_lit_comp("X"),
str8_lit_comp("Y"),
str8_lit_comp("S"),
str8_lit_comp("T"),
str8_lit_comp("Z"),
str8_lit_comp("d"),
str8_lit_comp("N"),
str8_lit_comp("E"),
str8_lit_comp("H"),
str8_lit_comp("e"),
str8_lit_comp("I"),
str8_lit_comp("J"),
str8_lit_comp("A"),
str8_lit_comp("?"),
str8_lit_comp("4"),
str8_lit_comp("5"),
str8_lit_comp("6"),
str8_lit_comp("&"),
str8_lit_comp("*"),
str8_lit_comp("("),
str8_lit_comp(")"),
str8_lit_comp("#"),
};

String8 ti_theme_preset_display_string_table[1] =
{
str8_lit_comp("Default (Dark)"),
};

String8 ti_theme_preset_code_string_table[1] =
{
str8_lit_comp("default_dark"),
};

String8 ti_theme_preset_cfg_string_table[1] =
{
str8_lit_comp("theme:\n{\n  theme_color:{tags: background, value: 0x1f1f1fff}\n  theme_color:{tags: \"alt background\", value: 0x222222ff}\n  theme_color:{tags: \"pop background\", value: 0x675331ff}\n  theme_color:{tags: \"fresh background\", value: 0x3d3631ff}\n  theme_color:{tags: \"match background\", value: 0x31393dff}\n  theme_color:{tags: border, value: 0x404040ff}\n  theme_color:{tags: text, value: 0xe5e5e5ff}\n  theme_color:{tags: \"weak text\", value: 0xa4a4a4ff}\n  theme_color:{tags: \"good text\", value: 0x32a852ff}\n  theme_color:{tags: \"neutral text\", value: 0x3a90bbff}\n  theme_color:{tags: \"bad text\", value: 0xcf5242ff}\n  theme_color:{tags: hover, value: 0xffffffff}\n  theme_color:{tags: \"focus overlay\", value: 0x2292eb14}\n  theme_color:{tags: \"focus border\", value: 0x2392ebff}\n  theme_color:{tags: cursor, value: 0x8aff00ff}\n  theme_color:{tags: selection, value: 0x99ccff0f}\n  theme_color:{tags: \"inactive background\", value: 0x16}\n  theme_color:{tags: drop_shadow, value: 0x0000007f}\n  theme_color:{tags: \"good_pop background\", value: 0x2c5b36ff}\n  theme_color:{tags: \"good_pop border\", value: 0x568761ff}\n  theme_color:{tags: \"good_pop hover\", value: 0xe3f5d3ff}\n  theme_color:{tags: \"good_pop weak text\", value: 0xe3f5d3ff}\n  theme_color:{tags: \"bad_pop background\", value: 0x803425ff}\n  theme_color:{tags: \"bad_pop hover\", value: 0xff825cff}\n  theme_color:{tags: code_default, value: 0xecececff}\n  theme_color:{tags: code_symbol, value: 0xceaf64ff}\n  theme_color:{tags: code_type, value: 0xceaf64ff}\n  theme_color:{tags: code_local, value: 0xa7dae8ff}\n  theme_color:{tags: code_register, value: 0xb7afd5ff}\n  theme_color:{tags: code_keyword, value: 0x838b8fff}\n  theme_color:{tags: code_delimiter_or_operator, value: 0x838b8fff}\n  theme_color:{tags: code_numeric, value: 0x9cb198ff}\n  theme_color:{tags: code_numeric_alt_digit_group, value: 0x6d8e67ff}\n  theme_color:{tags: code_string, value: 0x9cb198ff}\n  theme_color:{tags: code_meta, value: 0xb9819aff}\n  theme_color:{tags: code_comment, value: 0x627b5eff}\n  theme_color:{tags: line_info_0, value: 0x3c2d25ff}\n  theme_color:{tags: line_info_1, value: 0x3c3925ff}\n  theme_color:{tags: line_info_2, value: 0x2e4837ff}\n  theme_color:{tags: line_info_3, value: 0x2e3d48ff}\n  theme_color:{tags: line_info_4, value: 0x3c2d25ff}\n  theme_color:{tags: line_info_5, value: 0x3c3925ff}\n  theme_color:{tags: line_info_6, value: 0x2e4837ff}\n  theme_color:{tags: line_info_7, value: 0x2e3d48ff}\n  theme_color:{tags: thread_0, value: 0xebb624ff}\n  theme_color:{tags: thread_1, value: 0x26d0d2ff}\n  theme_color:{tags: thread_unwound, value: 0xb2ccd8ff}\n  theme_color:{tags: thread_error, value: 0xb23219ff}\n  theme_color:{tags: breakpoint, value: 0xa72911ff}\n  theme_color:{tags: \"floating background\", value: 0x1b1b1baf}\n  theme_color:{tags: \"floating background alt\", value: 0x0000005f}\n  theme_color:{tags: \"floating background fresh\", value: 0x31393d5f}\n  theme_color:{tags: \"floating border\", value: 0xbfbfbf1f}\n  theme_color:{tags: \"floating scroll_bar background\", value: 0x3b3b3b5f}\n  theme_color:{tags: \"floating scroll_bar border\", value: 0x5f5f5f5f}\n  theme_color:{tags: \"scroll_bar background\", value: 0x2b2b2bff}\n  theme_color:{tags: \"scroll_bar border\", value: 0x3f3f3fff}\n  theme_color:{tags: \"implicit background\", value: 0x00000000}\n  theme_color:{tags: \"implicit border\", value: 0x00000000}\n  theme_color:{tags: \"hollow background\", value: 0x00000000}\n  theme_color:{tags: \"hollow border\", value: 0xffffff1f}\n  theme_color:{tags: \"tab background\", value: 0x333333ff}\n  theme_color:{tags: \"tab border\", value: 0x2392ebff}\n  theme_color:{tags: \"tab inactive background\", value: 0}\n  theme_color:{tags: \"tab inactive border\", value: 0x42494eff}\n  theme_color:{tags: \"tab auto background\", value: 0x3e4d6eff}\n  theme_color:{tags: \"tab auto inactive background\", value: 0x3e4d6e39}\n  theme_color:{tags: \"drop_site background\", value: 0xffffff05}\n  theme_color:{tags: \"drop_site border\", value: 0xffffff0f}\n}\n"),
};

C_LINKAGE_END

