#include "menu_presenter_template.h"

const wchar_t *win16_xaml_menu_presenter_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='MenuFlyoutPresenter'>"
        L"<Setter Property='Background' Value='White'/><Setter Property='BorderBrush' Value='Black'/><Setter Property='BorderThickness' Value='1'/>"
        L"<Setter Property='Template'><Setter.Value><ControlTemplate TargetType='MenuFlyoutPresenter'>"
        L"<Border Background='{TemplateBinding Background}' BorderBrush='{TemplateBinding BorderBrush}' BorderThickness='{TemplateBinding BorderThickness}'>"
        L"<ScrollViewer x:Name='MenuFlyoutPresenterScrollViewer' HorizontalScrollMode='Disabled' HorizontalScrollBarVisibility='Disabled' VerticalScrollMode='Auto' VerticalScrollBarVisibility='Auto' ZoomMode='Disabled'><ItemsPresenter/></ScrollViewer>"
        L"</Border></ControlTemplate></Setter.Value></Setter></Style>";
}
