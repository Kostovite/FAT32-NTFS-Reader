using DiskEntityInfo.ViewModels;

using Microsoft.UI.Xaml.Controls;

namespace DiskEntityInfo.Views;

public sealed partial class MainPage : Page
{
    public MainViewModel ViewModel
    {
        get;
    }

    public MainPage()
    {
        ViewModel = App.GetService<MainViewModel>();
        InitializeComponent();
    }
}
