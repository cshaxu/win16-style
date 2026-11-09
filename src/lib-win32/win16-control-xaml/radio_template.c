#include "radio_template.h"

const wchar_t *win16_xaml_radio_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' "
        L"xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='RadioButton'>"
        L"<Setter Property='Foreground' Value='Black'/>"
        L"<Setter Property='Template'><Setter.Value><ControlTemplate TargetType='RadioButton'>"
        L"<Grid Background='Transparent'><VisualStateManager.VisualStateGroups>"
        L"<VisualStateGroup x:Name='CommonStates'><VisualState x:Name='Normal'/>"
        L"<VisualState x:Name='PointerOver'/><VisualState x:Name='Pressed'/>"
        L"<VisualState x:Name='Disabled'><VisualState.Setters>"
        L"<Setter Target='Label.Foreground' Value='#808080'/><Setter Target='Mark.Fill' Value='#808080'/>"
        L"</VisualState.Setters></VisualState></VisualStateGroup>"
        L"<VisualStateGroup x:Name='CheckStates'><VisualState x:Name='Unchecked'/>"
        L"<VisualState x:Name='Checked'><VisualState.Setters><Setter Target='Mark.Opacity' Value='1'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Indeterminate'/></VisualStateGroup>"
        L"<VisualStateGroup x:Name='FocusStates'><VisualState x:Name='Unfocused'/>"
        L"<VisualState x:Name='Focused'><VisualState.Setters><Setter Target='FocusFrame.Opacity' Value='1'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='PointerFocused'/></VisualStateGroup></VisualStateManager.VisualStateGroups>"
        L"<Grid.ColumnDefinitions><ColumnDefinition Width='16'/><ColumnDefinition Width='*'/></Grid.ColumnDefinitions>"
        L"<Grid Width='13' Height='13' HorizontalAlignment='Left' VerticalAlignment='Center'>"
        L"<Ellipse Fill='White' Stroke='Black' StrokeThickness='1'/>"
        L"<Ellipse x:Name='Mark' Width='7' Height='7' Fill='Black' Opacity='0'/></Grid>"
        L"<Border x:Name='FocusFrame' Grid.Column='1' BorderBrush='Black' BorderThickness='1' Opacity='0' IsHitTestVisible='False'/>"
        L"<ContentPresenter x:Name='Label' Grid.Column='1' Foreground='{TemplateBinding Foreground}' "
        L"Content='{TemplateBinding Content}' ContentTemplate='{TemplateBinding ContentTemplate}' VerticalAlignment='Center'/>"
        L"</Grid></ControlTemplate></Setter.Value></Setter></Style>";
}
