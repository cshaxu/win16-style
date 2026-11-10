#include "list_template.h"

const wchar_t *win16_xaml_list_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' "
    L"xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='ListBox'>"
    L"<Setter Property='Template'><Setter.Value><ControlTemplate TargetType='ListBox'>"
    L"<Border Background='White' BorderBrush='Black' BorderThickness='1'>"
    L"<ScrollViewer x:Name='ScrollViewer' HorizontalScrollMode='{TemplateBinding ScrollViewer.HorizontalScrollMode}' "
    L"HorizontalScrollBarVisibility='{TemplateBinding ScrollViewer.HorizontalScrollBarVisibility}' "
    L"VerticalScrollMode='{TemplateBinding ScrollViewer.VerticalScrollMode}' "
    L"VerticalScrollBarVisibility='{TemplateBinding ScrollViewer.VerticalScrollBarVisibility}'>"
    L"<ItemsPresenter/></ScrollViewer></Border></ControlTemplate></Setter.Value></Setter></Style>";
}
