#include "menu_separator_template.h"

const wchar_t *win16_xaml_menu_separator_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='MenuFlyoutSeparator'>"
        L"<Setter Property='Template'><Setter.Value><ControlTemplate TargetType='MenuFlyoutSeparator'><Grid Background='White' Height='5'>"
        L"<Rectangle x:Name='SeparatorLine' Fill='Black' Height='1' VerticalAlignment='Center'/></Grid>"
        L"</ControlTemplate></Setter.Value></Setter></Style>";
}
