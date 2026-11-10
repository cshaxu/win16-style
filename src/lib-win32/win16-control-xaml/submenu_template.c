#include "submenu_template.h"

const wchar_t *win16_xaml_submenu_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='MenuFlyoutSubItem'>"
        L"<Setter Property='Foreground' Value='Black'/><Setter Property='Template'><Setter.Value><ControlTemplate TargetType='MenuFlyoutSubItem'>"
        L"<Grid x:Name='LayoutRoot' Background='White' Padding='4,2'><VisualStateManager.VisualStateGroups><VisualStateGroup x:Name='CommonStates'>"
        L"<VisualState x:Name='Normal'/><VisualState x:Name='PointerOver'><VisualState.Setters><Setter Target='LayoutRoot.Background' Value='#000080'/><Setter Target='IconContent.Foreground' Value='White'/><Setter Target='TextBlock.Foreground' Value='White'/><Setter Target='SubItemChevron.Stroke' Value='White'/><Setter Target='SubItemChevron.Fill' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Pressed'><VisualState.Setters><Setter Target='LayoutRoot.Background' Value='#000080'/><Setter Target='IconContent.Foreground' Value='White'/><Setter Target='TextBlock.Foreground' Value='White'/><Setter Target='SubItemChevron.Stroke' Value='White'/><Setter Target='SubItemChevron.Fill' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='SubMenuOpened'><VisualState.Setters><Setter Target='LayoutRoot.Background' Value='#000080'/><Setter Target='IconContent.Foreground' Value='White'/><Setter Target='TextBlock.Foreground' Value='White'/><Setter Target='SubItemChevron.Stroke' Value='White'/><Setter Target='SubItemChevron.Fill' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Disabled'><VisualState.Setters><Setter Target='IconContent.Foreground' Value='#808080'/><Setter Target='TextBlock.Foreground' Value='#808080'/><Setter Target='SubItemChevron.Stroke' Value='#808080'/><Setter Target='SubItemChevron.Fill' Value='#808080'/></VisualState.Setters></VisualState></VisualStateGroup></VisualStateManager.VisualStateGroups>"
        L"<Grid.ColumnDefinitions><ColumnDefinition Width='Auto'/><ColumnDefinition Width='*'/><ColumnDefinition Width='Auto'/></Grid.ColumnDefinitions><ContentPresenter x:Name='IconContent' Content='{TemplateBinding Icon}' Foreground='{TemplateBinding Foreground}'/>"
        L"<TextBlock x:Name='TextBlock' Grid.Column='1' Text='{TemplateBinding Text}' Foreground='{TemplateBinding Foreground}'/><Path x:Name='SubItemChevron' Grid.Column='2' Margin='12,0,0,0' Data='M 0,0 L 4,4 L 0,8 Z' Fill='Black' Stroke='Black' Width='5' Height='9'/></Grid>"
        L"</ControlTemplate></Setter.Value></Setter></Style>";
}
