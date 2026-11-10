#include "menu_item_template.h"

const wchar_t *win16_xaml_menu_item_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='MenuFlyoutItem'>"
        L"<Setter Property='Foreground' Value='Black'/><Setter Property='Template'><Setter.Value><ControlTemplate TargetType='MenuFlyoutItem'>"
        L"<Grid x:Name='LayoutRoot' Background='White' Padding='4,2'><VisualStateManager.VisualStateGroups>"
        L"<VisualStateGroup x:Name='CommonStates'><VisualState x:Name='Normal'/><VisualState x:Name='PointerOver'><VisualState.Setters>"
        L"<Setter Target='LayoutRoot.Background' Value='#000080'/><Setter Target='IconContent.Foreground' Value='White'/><Setter Target='TextBlock.Foreground' Value='White'/><Setter Target='KeyboardAcceleratorTextBlock.Foreground' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Pressed'><VisualState.Setters><Setter Target='LayoutRoot.Background' Value='#000080'/><Setter Target='IconContent.Foreground' Value='White'/><Setter Target='TextBlock.Foreground' Value='White'/><Setter Target='KeyboardAcceleratorTextBlock.Foreground' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Disabled'><VisualState.Setters><Setter Target='IconContent.Foreground' Value='#808080'/><Setter Target='TextBlock.Foreground' Value='#808080'/><Setter Target='KeyboardAcceleratorTextBlock.Foreground' Value='#808080'/></VisualState.Setters></VisualState></VisualStateGroup>"
        L"<VisualStateGroup x:Name='KeyboardAcceleratorTextVisibility'><VisualState x:Name='KeyboardAcceleratorTextCollapsed'/><VisualState x:Name='KeyboardAcceleratorTextVisible'><VisualState.Setters><Setter Target='KeyboardAcceleratorTextBlock.Visibility' Value='Visible'/></VisualState.Setters></VisualState></VisualStateGroup>"
        L"</VisualStateManager.VisualStateGroups><Grid.ColumnDefinitions><ColumnDefinition Width='Auto'/><ColumnDefinition Width='*'/><ColumnDefinition Width='Auto'/></Grid.ColumnDefinitions>"
        L"<ContentPresenter x:Name='IconContent' Content='{TemplateBinding Icon}' Foreground='{TemplateBinding Foreground}'/><TextBlock x:Name='TextBlock' Grid.Column='1' Text='{TemplateBinding Text}' Foreground='{TemplateBinding Foreground}'/>"
        L"<TextBlock x:Name='KeyboardAcceleratorTextBlock' Grid.Column='2' Margin='16,0,0,0' Text='{TemplateBinding KeyboardAcceleratorTextOverride}' Foreground='{TemplateBinding Foreground}' Visibility='Collapsed'/></Grid>"
        L"</ControlTemplate></Setter.Value></Setter></Style>";
}
