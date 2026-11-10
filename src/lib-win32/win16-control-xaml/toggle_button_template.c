#include "toggle_button_template.h"

const wchar_t *win16_xaml_toggle_button_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='ToggleButton'>"
        L"<Setter Property='Foreground' Value='Black'/><Setter Property='Template'><Setter.Value><ControlTemplate TargetType='ToggleButton'>"
        L"<Border Background='#C0C0C0' BorderBrush='Black' BorderThickness='1'><VisualStateManager.VisualStateGroups>"
        L"<VisualStateGroup x:Name='CommonStates'><VisualState x:Name='Normal'/><VisualState x:Name='PointerOver'/><VisualState x:Name='Indeterminate'/>"
        L"<VisualState x:Name='Pressed'><VisualState.Setters><Setter Target='TopEdge.Background' Value='#808080'/><Setter Target='LeftEdge.Background' Value='#808080'/><Setter Target='BottomEdge.Background' Value='White'/><Setter Target='RightEdge.Background' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Checked'><VisualState.Setters><Setter Target='TopEdge.Background' Value='#808080'/><Setter Target='LeftEdge.Background' Value='#808080'/><Setter Target='BottomEdge.Background' Value='White'/><Setter Target='RightEdge.Background' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='CheckedPointerOver'><VisualState.Setters><Setter Target='TopEdge.Background' Value='#808080'/><Setter Target='LeftEdge.Background' Value='#808080'/><Setter Target='BottomEdge.Background' Value='White'/><Setter Target='RightEdge.Background' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='CheckedPressed'><VisualState.Setters><Setter Target='TopEdge.Background' Value='#808080'/><Setter Target='LeftEdge.Background' Value='#808080'/><Setter Target='BottomEdge.Background' Value='White'/><Setter Target='RightEdge.Background' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Disabled'><VisualState.Setters><Setter Target='Content.Foreground' Value='#808080'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='CheckedDisabled'><VisualState.Setters><Setter Target='Content.Foreground' Value='#808080'/><Setter Target='TopEdge.Background' Value='#808080'/><Setter Target='LeftEdge.Background' Value='#808080'/><Setter Target='BottomEdge.Background' Value='White'/><Setter Target='RightEdge.Background' Value='White'/></VisualState.Setters></VisualState>"
        L"</VisualStateGroup><VisualStateGroup x:Name='FocusStates'><VisualState x:Name='Unfocused'/><VisualState x:Name='Focused'><VisualState.Setters><Setter Target='FocusFrame.Opacity' Value='1'/></VisualState.Setters></VisualState><VisualState x:Name='PointerFocused'/></VisualStateGroup></VisualStateManager.VisualStateGroups>"
        L"<Grid><Border x:Name='TopEdge' Background='White' Height='2' VerticalAlignment='Top' IsHitTestVisible='False'/><Border x:Name='LeftEdge' Background='White' Width='2' HorizontalAlignment='Left' IsHitTestVisible='False'/>"
        L"<Border x:Name='BottomEdge' Background='#808080' Height='2' VerticalAlignment='Bottom' IsHitTestVisible='False'/><Border x:Name='RightEdge' Background='#808080' Width='2' HorizontalAlignment='Right' IsHitTestVisible='False'/>"
        L"<Border x:Name='FocusFrame' BorderBrush='Black' BorderThickness='1' Margin='4' Opacity='0' IsHitTestVisible='False'/><ContentPresenter x:Name='Content' Foreground='{TemplateBinding Foreground}' Content='{TemplateBinding Content}' ContentTemplate='{TemplateBinding ContentTemplate}' HorizontalAlignment='Center' VerticalAlignment='Center' Margin='4'/></Grid>"
        L"</Border></ControlTemplate></Setter.Value></Setter></Style>";
}
