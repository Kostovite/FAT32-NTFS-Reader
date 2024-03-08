using Microsoft.UI.Xaml;
using Windows.Storage.AccessCache;
using Windows.Storage.Pickers;
using Windows.Storage;
using Microsoft.UI.Xaml.Controls;
using System.Collections.ObjectModel;
using Microsoft.UI.Xaml.Media.Imaging;
using Windows.Storage.FileProperties;
using System.Diagnostics;

namespace DiskEntityInfo;

public sealed partial class MainWindow : WindowEx
{
    public MainWindow()
    {
        Title = "Disk Entity Info";

        // no UIElement is set for titlebar, default titlebar is created which extends to entire non client area

        InitializeComponent();
        InitializeSampleTreeView();
    }

    public class TreeItem
    {
        public BitmapImage? ImageSource { get; set; }
        public string? Text { get; set; }
        public string? Path { get; set; }
    }
    public ObservableCollection<TreeItem> treeItems = new();

    private async void InitializeSampleTreeView()
    {
        foreach (var drive in DriveInfo.GetDrives())
        {
            try
            {
                StorageFolder folder = await StorageFolder.GetFolderFromPathAsync(drive.RootDirectory.FullName);
                StorageItemThumbnail thumbnail = await folder.GetThumbnailAsync(ThumbnailMode.SingleItem, 32, ThumbnailOptions.UseCurrentScale);
                BitmapImage iconImage = new();
                iconImage.SetSource(thumbnail);

                TreeItem item = new();

                item.ImageSource = iconImage;
                item.Text = drive.Name;
                item.Path = drive.Name;
                treeItems.Add(item);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }

        treeView.ItemsSource = treeItems;
    }

    private void treeView_Expanding(TreeView sender, TreeViewExpandingEventArgs args)
    {
        if (args.Node.Content is TreeItem treeItem)
        {
            try
            {
                var subfolderPaths = Directory.GetDirectories(treeItem.Path);

                foreach (var subfolder in subfolderPaths)
                {
                    TreeViewNode subfolderNode = new();

                    //Using the folder icon for the subfolder
                    BitmapImage iconImage = new BitmapImage(new Uri("ms-appx:///Assets/folder.png"));

                    TreeItem item = new();
                    item.ImageSource = iconImage;
                    item.Text = Path.GetFileName(subfolder);
                    item.Path = subfolder;
                    subfolderNode.Content = item;
                    args.Node.Children.Add(subfolderNode);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }
    }

    public void treeView_DoubleTapped(object sender, Microsoft.UI.Xaml.Input.DoubleTappedRoutedEventArgs e)
    {
        if (treeView.SelectedNode == null)
        {
            return;
        }

        if (!treeView.SelectedNode.IsExpanded)
        {
        
            treeView.SelectedNode.IsExpanded = true;
        }
        else
        {
            treeView.SelectedNode.IsExpanded = false;
        }
    }

    private void treeView_Collapsed(TreeView sender, TreeViewCollapsedEventArgs args)
    {
        if (args.Node.HasChildren)
        {
            args.Node.Children.Clear();
            args.Node.HasUnrealizedChildren = true;
        }
    }

    private async void PickAFileButton_Click(object sender, RoutedEventArgs e)
    {
        // Clear previous returned file name, if it exists, between iterations of this scenario
        PickAFileOutputTextBlock.Document.SetText((Microsoft.UI.Text.TextSetOptions)Windows.UI.Text.TextSetOptions.None, "");

        // Create a file picker
        var openPicker = new Windows.Storage.Pickers.FileOpenPicker();

        // See the sample code below for how to make the window accessible from the App class.
        var window = App.MainWindow;

        // Retrieve the window handle (HWND) of the current WinUI 3 window.
        var hWnd = WinRT.Interop.WindowNative.GetWindowHandle(window);

        // Initialize the file picker with the window handle (HWND).
        WinRT.Interop.InitializeWithWindow.Initialize(openPicker, hWnd);

        // Set options for your file picker
        openPicker.ViewMode = PickerViewMode.Thumbnail;
        openPicker.FileTypeFilter.Add("*");

        // Open the picker for the user to pick a file
        var file = await openPicker.PickSingleFileAsync();
        if (file != null)
        {
            PickAFileOutputTextBlock.Document.SetText((Microsoft.UI.Text.TextSetOptions)Microsoft.UI.Text.TextSetOptions.None, "Picked file: " + file.Name);
            PickFolderOutputTextBlock.Document.SetText((Microsoft.UI.Text.TextSetOptions)Microsoft.UI.Text.TextSetOptions.None, "");
        }
        else
        {
            PickAFileOutputTextBlock.Document.SetText((Microsoft.UI.Text.TextSetOptions)Microsoft.UI.Text.TextSetOptions.None, "Operation cancelled.");
        }
    }

    private async void PickFolderButton_Click(object sender, RoutedEventArgs e)
    {
        // Clear previous returned file name, if it exists, between iterations of this scenario
        PickFolderOutputTextBlock.Document.SetText(Microsoft.UI.Text.TextSetOptions.None, "");

        // Create a folder picker
        FolderPicker openPicker = new Windows.Storage.Pickers.FolderPicker();

        // See the sample code below for how to make the window accessible from the App class.
        var window = App.MainWindow;

        // Retrieve the window handle (HWND) of the current WinUI 3 window.
        var hWnd = WinRT.Interop.WindowNative.GetWindowHandle(window);

        // Initialize the folder picker with the window handle (HWND).
        WinRT.Interop.InitializeWithWindow.Initialize(openPicker, hWnd);

        // Set options for your folder picker
        openPicker.SuggestedStartLocation = PickerLocationId.Desktop;
        openPicker.FileTypeFilter.Add("*");

        // Open the picker for the user to pick a folder
        StorageFolder folder = await openPicker.PickSingleFolderAsync();
        if (folder != null)
        {
            StorageApplicationPermissions.FutureAccessList.AddOrReplace("PickedFolderToken", folder);
            PickFolderOutputTextBlock.Document.SetText((Microsoft.UI.Text.TextSetOptions)Microsoft.UI.Text.TextSetOptions.None, "Picked folder: " + folder.Name);
            PickAFileOutputTextBlock.Document.SetText((Microsoft.UI.Text.TextSetOptions)Microsoft.UI.Text.TextSetOptions.None, "");
        }
        else
        {
            PickFolderOutputTextBlock.Document.SetText((Microsoft.UI.Text.TextSetOptions)Microsoft.UI.Text.TextSetOptions.None, "Operation cancelled.");
        }
    }
}
