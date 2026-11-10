#include "combo_template.h"

const wchar_t *win16_xaml_combo_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' "
    L"xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='ComboBox'>"
    L"<Setter Property='Foreground' Value='Black'/><Setter Property='Template'><Setter.Value><ControlTemplate TargetType='ComboBox'>"
    L"<Grid x:Name='LayoutRoot' Background='White' BorderBrush='Black' BorderThickness='1'>"
    L"<VisualStateManager.VisualStateGroups><VisualStateGroup x:Name='CommonStates'>"
    L"<VisualState x:Name='Normal'/><VisualState x:Name='PointerOver'/><VisualState x:Name='Pressed'/>"
    L"<VisualState x:Name='Disabled'><VisualState.Setters><Setter Target='ContentPresenter.Foreground' Value='#808080'/>"
    L"<Setter Target='DropDownGlyph.Foreground' Value='#808080'/></VisualState.Setters></VisualState>"
    L"</VisualStateGroup></VisualStateManager.VisualStateGroups>"
    L"<Grid.RowDefinitions><RowDefinition Height='Auto'/><RowDefinition Height='*'/><RowDefinition Height='Auto'/></Grid.RowDefinitions>"
    L"<Grid.ColumnDefinitions><ColumnDefinition Width='*'/><ColumnDefinition Width='20'/></Grid.ColumnDefinitions>"
    L"<ContentPresenter Grid.Row='0' Grid.ColumnSpan='2' Content='{TemplateBinding Header}' ContentTemplate='{TemplateBinding HeaderTemplate}'/>"
    L"<ContentPresenter Grid.Row='1' x:Name='ContentPresenter' Foreground='{TemplateBinding Foreground}' Content='{TemplateBinding SelectionBoxItem}' ContentTemplate='{TemplateBinding SelectionBoxItemTemplate}'/>"
    L"<TextBox Grid.Row='1' x:Name='EditableText' Visibility='Collapsed' BorderThickness='0' "
    L"Text='{Binding RelativeSource={RelativeSource TemplatedParent}, Path=Text, Mode=TwoWay, UpdateSourceTrigger=PropertyChanged}'/>"
    L"<Border Grid.Row='1' Grid.Column='1' BorderBrush='Black' BorderThickness='1' Background='#C0C0C0'>"
    L"<TextBlock x:Name='DropDownGlyph' Text='▼' Foreground='Black' HorizontalAlignment='Center' VerticalAlignment='Center'/></Border>"
    L"<Popup x:Name='Popup'><Border x:Name='PopupBorder' Background='White' BorderBrush='Black' BorderThickness='1'>"
    L"<ScrollViewer x:Name='ScrollViewer' HorizontalScrollMode='{TemplateBinding ScrollViewer.HorizontalScrollMode}' "
    L"HorizontalScrollBarVisibility='{TemplateBinding ScrollViewer.HorizontalScrollBarVisibility}' "
    L"VerticalScrollMode='{TemplateBinding ScrollViewer.VerticalScrollMode}' "
    L"VerticalScrollBarVisibility='{TemplateBinding ScrollViewer.VerticalScrollBarVisibility}'><ItemsPresenter/></ScrollViewer></Border></Popup>"
    L"<ContentPresenter Grid.Row='2' Grid.ColumnSpan='2' Content='{TemplateBinding Description}'/>"
    L"</Grid></ControlTemplate></Setter.Value></Setter></Style>";
}
