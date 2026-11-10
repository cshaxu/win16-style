#include "toggle_menu_template.h"

const wchar_t *win16_xaml_toggle_menu_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='ToggleMenuFlyoutItem'>"
        L"<Setter Property='Foreground' Value='Black'/><Setter Property='Template'><Setter.Value><ControlTemplate TargetType='ToggleMenuFlyoutItem'>"
        L"<Grid x:Name='LayoutRoot' Background='White' Padding='4,2'><VisualStateManager.VisualStateGroups>"
        L"<VisualStateGroup x:Name='CommonStates'><VisualState x:Name='Normal'/><VisualState x:Name='PointerOver'><VisualState.Setters>"
        L"<Setter Target='LayoutRoot.Background' Value='#000080'/><Setter Target='CheckGlyph.Stroke' Value='White'/><Setter Target='TextBlock.Foreground' Value='White'/><Setter Target='KeyboardAcceleratorTextBlock.Foreground' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Pressed'><VisualState.Setters><Setter Target='LayoutRoot.Background' Value='#000080'/><Setter Target='CheckGlyph.Stroke' Value='White'/><Setter Target='TextBlock.Foreground' Value='White'/><Setter Target='KeyboardAcceleratorTextBlock.Foreground' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Disabled'><VisualState.Setters><Setter Target='CheckGlyph.Stroke' Value='#808080'/><Setter Target='TextBlock.Foreground' Value='#808080'/><Setter Target='KeyboardAcceleratorTextBlock.Foreground' Value='#808080'/></VisualState.Setters></VisualState></VisualStateGroup>"
        L"<VisualStateGroup x:Name='KeyboardAcceleratorTextVisibility'><VisualState x:Name='KeyboardAcceleratorTextCollapsed'/><VisualState x:Name='KeyboardAcceleratorTextVisible'><VisualState.Setters><Setter Target='KeyboardAcceleratorTextBlock.Visibility' Value='Visible'/></VisualState.Setters></VisualState></VisualStateGroup>"
        L"<VisualStateGroup x:Name='CheckStates'><VisualState x:Name='Unchecked'/><VisualState x:Name='UncheckedWithIcon'/><VisualState x:Name='Checked'><VisualState.Setters><Setter Target='CheckGlyph.Opacity' Value='1'/></VisualState.Setters></VisualState><VisualState x:Name='CheckedWithIcon'><VisualState.Setters><Setter Target='CheckGlyph.Opacity' Value='1'/></VisualState.Setters></VisualState></VisualStateGroup>"
        L"</VisualStateManager.VisualStateGroups><Grid.ColumnDefinitions><ColumnDefinition Width='Auto'/><ColumnDefinition Width='Auto'/><ColumnDefinition Width='*'/><ColumnDefinition Width='Auto'/></Grid.ColumnDefinitions>"
        L"<Path x:Name='CheckGlyph' Data='M 1,5 L 4,8 L 10,1' Stroke='Black' StrokeThickness='2' Width='12' Height='12' Margin='0,0,4,0' Opacity='0'/>"
        L"<ContentPresenter x:Name='IconContent' Grid.Column='1' Content='{TemplateBinding Icon}' Foreground='{TemplateBinding Foreground}'/><TextBlock x:Name='TextBlock' Grid.Column='2' Text='{TemplateBinding Text}' Foreground='{TemplateBinding Foreground}'/>"
        L"<TextBlock x:Name='KeyboardAcceleratorTextBlock' Grid.Column='3' Margin='16,0,0,0' Text='{TemplateBinding KeyboardAcceleratorTextOverride}' Foreground='{TemplateBinding Foreground}' Visibility='Collapsed'/></Grid>"
        L"</ControlTemplate></Setter.Value></Setter></Style>";
}
