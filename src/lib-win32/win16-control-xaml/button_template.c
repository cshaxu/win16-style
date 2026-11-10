#include "button_template.h"

#define BUTTON_STYLE(TYPE_NAME) \
    L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' " \
L"xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' xmlns:p='using:Microsoft.UI.Xaml.Controls.Primitives' TargetType='" TYPE_NAME L"'>" \
L"<Setter Property='Foreground' Value='Black'/>" \
L"<Setter Property='Template'><Setter.Value><ControlTemplate TargetType='" TYPE_NAME L"'>" \
L"<Border x:Name='Frame' Background='#C0C0C0' BorderBrush='Black' BorderThickness='1'>" \
L"<VisualStateManager.VisualStateGroups><VisualStateGroup x:Name='CommonStates'>" \
L"<VisualState x:Name='Normal'/><VisualState x:Name='PointerOver'/>" \
L"<VisualState x:Name='Pressed'><VisualState.Setters>" \
L"<Setter Target='TopEdge.Background' Value='#808080'/><Setter Target='LeftEdge.Background' Value='#808080'/>" \
L"<Setter Target='BottomEdge.Background' Value='#FFFFFF'/><Setter Target='RightEdge.Background' Value='#FFFFFF'/>" \
L"</VisualState.Setters></VisualState>" \
L"<VisualState x:Name='Disabled'><VisualState.Setters><Setter Target='Content.Foreground' Value='#808080'/></VisualState.Setters></VisualState>" \
L"</VisualStateGroup><VisualStateGroup x:Name='FocusStates'>" \
L"<VisualState x:Name='Unfocused'/><VisualState x:Name='Focused'><VisualState.Setters>" \
L"<Setter Target='FocusFrame.Opacity' Value='1'/></VisualState.Setters></VisualState>" \
L"<VisualState x:Name='PointerFocused'/></VisualStateGroup></VisualStateManager.VisualStateGroups>" \
L"<Grid>" \
L"<Border x:Name='TopEdge' Background='#FFFFFF' Height='2' VerticalAlignment='Top' IsHitTestVisible='False'/>" \
L"<Border x:Name='LeftEdge' Background='#FFFFFF' Width='2' HorizontalAlignment='Left' IsHitTestVisible='False'/>" \
L"<Border x:Name='BottomEdge' Background='#808080' Height='2' VerticalAlignment='Bottom' IsHitTestVisible='False'/>" \
L"<Border x:Name='RightEdge' Background='#808080' Width='2' HorizontalAlignment='Right' IsHitTestVisible='False'/>" \
L"<Border x:Name='FocusFrame' BorderBrush='Black' BorderThickness='1' Margin='4' Opacity='0' IsHitTestVisible='False'/>" \
L"<ContentPresenter x:Name='Content' Foreground='{TemplateBinding Foreground}' Content='{TemplateBinding Content}' ContentTemplate='{TemplateBinding ContentTemplate}' " \
L"HorizontalAlignment='Center' VerticalAlignment='Center' Margin='4'/>" \
L"</Grid>" \
L"</Border></ControlTemplate></Setter.Value></Setter></Style>"

const wchar_t *win16_xaml_button_template(void)
{
    return BUTTON_STYLE(L"Button");
}

const wchar_t *win16_xaml_repeat_button_template(void)
{
    return BUTTON_STYLE(L"p:RepeatButton");
}
