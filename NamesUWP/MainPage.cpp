#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

#include "AddCommand.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::NamesUWP::implementation
{
    MainPage::MainPage()
    {
        InitializeComponent();
        winrt::Windows::Foundation::Size size(600, 340);
        winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView().SetPreferredMinSize(size);
    }

    int32_t MainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainPage::Clear()
    {
        lstNames().Items().Clear();
        undos.clear();
        miUndo().IsEnabled(false);
        redos.clear();
        miRedo().IsEnabled(false);
    }

    bool IsWhiteSpace(winrt::hstring rtStr)
    {
        std::wstring stdStr = rtStr.c_str();
        for (int i = 0; i < stdStr.size(); i++)
        {
            if (!std::isspace(stdStr[i]))
            {
                return false;
            }
        }
        return true;
    }

    bool ListViewContains(winrt::Windows::UI::Xaml::Controls::ListBox list, winrt::hstring str)
    {
        for (int i = 0; i < list.Items().Size(); i++)
        {
            if (winrt::unbox_value<winrt::hstring>(list.Items().GetAt(i)) == str)
            {
                return true;
            }
        }
        return false;
    }

    void MainPage::miNew_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        Clear();
    }

    fire_and_forget MainPage::miOpen_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        winrt::Windows::Storage::Pickers::FileOpenPicker openPicker;
        openPicker.ViewMode(winrt::Windows::Storage::Pickers::PickerViewMode::Thumbnail);
        openPicker.FileTypeFilter().ReplaceAll({ L".txt" });
        winrt::Windows::Storage::StorageFile file = co_await openPicker.PickSingleFileAsync();
        if (file != nullptr)
        {
            Clear();

            winrt::Windows::Foundation::Collections::IVector<winrt::hstring> lines = co_await winrt::Windows::Storage::FileIO::ReadLinesAsync(file, winrt::Windows::Storage::Streams::UnicodeEncoding::Utf16LE);
            for (int i = 0; i < lines.Size(); i++)
            {
                const winrt::hstring line = lines.GetAt(i);
                if (!IsWhiteSpace(line) && !ListViewContains(lstNames(), line))
                {
                    lstNames().Items().Append(winrt::box_value(line));
                }
            }
        }
    }

    fire_and_forget MainPage::miSave_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        winrt::Windows::Storage::Pickers::FileSavePicker savePicker;
        savePicker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);
        savePicker.FileTypeChoices().Insert(L"Text Documents", single_threaded_vector<hstring>({ L".txt" }));
        winrt::Windows::Storage::StorageFile file = co_await savePicker.PickSaveFileAsync();
        if (file != nullptr)
        {
            winrt::Windows::Foundation::Collections::IVector<winrt::hstring> lines{ winrt::single_threaded_vector<winrt::hstring>() };
            for (int i = 0; i < lstNames().Items().Size(); i++)
            {
                lines.Append(winrt::unbox_value<winrt::hstring>(lstNames().Items().GetAt(i)));
            }

            winrt::Windows::Storage::CachedFileManager::DeferUpdates(file);

            co_await winrt::Windows::Storage::FileIO::WriteLinesAsync(file, lines, winrt::Windows::Storage::Streams::UnicodeEncoding::Utf16LE);

            winrt::Windows::Storage::Provider::FileUpdateStatus status = co_await winrt::Windows::Storage::CachedFileManager::CompleteUpdatesAsync(file);
            if (status == winrt::Windows::Storage::Provider::FileUpdateStatus::Failed)
            {
                winrt::Windows::UI::Popups::MessageDialog msg{ L"Could not write to file.", L"Error!" };
                msg.ShowAsync();
            }
        }
    }

    void MainPage::miExit_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        exit(0);
    }

    void MainPage::miUndo_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        std::unique_ptr<ICommand> command = std::move(undos.back());
        command->Unexecute();
        undos.pop_back();
        if (undos.empty())
        {
            miUndo().IsEnabled(false);
        }
        redos.push_back(std::move(command));
        miRedo().IsEnabled(true);
    }

    void MainPage::miRedo_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        std::unique_ptr<ICommand> command = std::move(redos.back());
        command->Execute();
        redos.pop_back();
        if (redos.empty())
        {
            miRedo().IsEnabled(false);
        }
        undos.push_back(std::move(command));
        miUndo().IsEnabled(true);
    }

    void MainPage::miAbout_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        winrt::Windows::UI::Popups::MessageDialog msg{ L"Thank you for using our software!", L"About" };
        msg.ShowAsync();
    }

    void MainPage::btnAdd_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        winrt::hstring txt = txtName().Text();
        if (!IsWhiteSpace(txt) && !ListViewContains(lstNames(), txt))
        {
            std::unique_ptr<ICommand> command = std::make_unique<AddCommand>(lstNames().Items(), txt);
            command->Execute();
            undos.push_back(std::move(command));
            miUndo().IsEnabled(true);
            redos.clear();
            miRedo().IsEnabled(false);

            txtName().Text(L"");
        }
    }
}
