#include "progress_template.h"

const wchar_t *win16_xaml_progress_template(void)
{
    return L"<Style xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' TargetType='ProgressBar'>"
    L"<Setter Property='Template'><Setter.Value><ControlTemplate TargetType='ProgressBar'>"
    L"<Grid x:Name='LayoutRoot'><VisualStateManager.VisualStateGroups><VisualStateGroup x:Name='CommonStates'>"
    L"<VisualState x:Name='Normal'/><VisualState x:Name='Determinate'/><VisualState x:Name='Updating'/>"
    L"<VisualState x:Name='Paused'><VisualState.Setters><Setter Target='DeterminateProgressBarIndicator.Fill' Value='#808080'/></VisualState.Setters></VisualState>"
    L"<VisualState x:Name='Error'><VisualState.Setters><Setter Target='DeterminateProgressBarIndicator.Fill' Value='#800000'/></VisualState.Setters></VisualState>"
    L"<VisualState x:Name='UpdatingError'><VisualState.Setters><Setter Target='DeterminateProgressBarIndicator.Fill' Value='#800000'/></VisualState.Setters></VisualState><VisualState x:Name='Indeterminate'>"
    L"<VisualState.Setters><Setter Target='IndeterminateProgressBarIndicator.Opacity' Value='1'/></VisualState.Setters>"
    L"<Storyboard RepeatBehavior='Forever'><DoubleAnimation Storyboard.TargetName='IndeterminateProgressBarIndicator' "
    L"Storyboard.TargetProperty='(UIElement.RenderTransform).(TranslateTransform.X)' "
    L"From='{Binding RelativeSource={RelativeSource TemplatedParent}, Path=TemplateSettings.ContainerAnimationStartPosition}' "
    L"To='{Binding RelativeSource={RelativeSource TemplatedParent}, Path=TemplateSettings.ContainerAnimationEndPosition}' Duration='0:0:1.5'/></Storyboard></VisualState>"
    L"<VisualState x:Name='IndeterminatePaused'><VisualState.Setters><Setter Target='IndeterminateProgressBarIndicator.Opacity' Value='1'/><Setter Target='IndeterminateProgressBarIndicator.Fill' Value='#808080'/></VisualState.Setters></VisualState>"
    L"<VisualState x:Name='IndeterminateError'><VisualState.Setters><Setter Target='IndeterminateProgressBarIndicator.Opacity' Value='1'/><Setter Target='IndeterminateProgressBarIndicator.Fill' Value='#800000'/></VisualState.Setters></VisualState>"
    L"</VisualStateGroup></VisualStateManager.VisualStateGroups>"
    L"<Border Background='White' BorderBrush='Black' BorderThickness='1'><Grid>"
    L"<Rectangle x:Name='ProgressBarTrack' Fill='White'/>"
    L"<Rectangle x:Name='DeterminateProgressBarIndicator' Fill='#000080' HorizontalAlignment='Left'/>"
    L"<Rectangle x:Name='IndeterminateProgressBarIndicator' Fill='#000080' HorizontalAlignment='Left' Opacity='0'>"
    L"<Rectangle.RenderTransform><TranslateTransform/></Rectangle.RenderTransform></Rectangle>"
    L"<Rectangle x:Name='IndeterminateProgressBarIndicator2' Opacity='0'/>"
    L"</Grid></Border></Grid></ControlTemplate></Setter.Value></Setter></Style>";
}
