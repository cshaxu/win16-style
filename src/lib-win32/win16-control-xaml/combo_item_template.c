#include "combo_item_template.h"

const wchar_t *win16_xaml_combo_item_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' "
        L"xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='ComboBoxItem'>"
        L"<Setter Property='Foreground' Value='Black'/><Setter Property='Template'><Setter.Value>"
        L"<ControlTemplate TargetType='ComboBoxItem'><Grid x:Name='ItemRoot' Background='White'>"
        L"<VisualStateManager.VisualStateGroups><VisualStateGroup x:Name='CommonStates'>"
        L"<VisualState x:Name='Normal'/><VisualState x:Name='PointerOver'/><VisualState x:Name='Pressed'/>"
        L"<VisualState x:Name='Disabled'><VisualState.Setters><Setter Target='ItemContent.Foreground' Value='#808080'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='Selected'><VisualState.Setters><Setter Target='ItemRoot.Background' Value='#000080'/><Setter Target='ItemContent.Foreground' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='SelectedUnfocused'><VisualState.Setters><Setter Target='ItemRoot.Background' Value='#000080'/><Setter Target='ItemContent.Foreground' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='SelectedPointerOver'><VisualState.Setters><Setter Target='ItemRoot.Background' Value='#000080'/><Setter Target='ItemContent.Foreground' Value='White'/></VisualState.Setters></VisualState>"
        L"<VisualState x:Name='SelectedPressed'><VisualState.Setters><Setter Target='ItemRoot.Background' Value='#000080'/><Setter Target='ItemContent.Foreground' Value='White'/></VisualState.Setters></VisualState>"
        L"</VisualStateGroup></VisualStateManager.VisualStateGroups>"
        L"<ContentPresenter x:Name='ItemContent' Foreground='{TemplateBinding Foreground}' "
        L"Content='{TemplateBinding Content}' ContentTemplate='{TemplateBinding ContentTemplate}' "
        L"Padding='{TemplateBinding Padding}'/></Grid></ControlTemplate></Setter.Value></Setter></Style>";
}
