#pragma once

#include "MainPage.g.h"
#include "ICommand.h"
#include <vector>

namespace winrt::NamesUWP::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
    private:
        std::vector<std::unique_ptr<ICommand>> undos;
        std::vector<std::unique_ptr<ICommand>> redos;
    public:
        MainPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void Clear();

        void miNew_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        fire_and_forget miOpen_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        fire_and_forget miSave_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void miExit_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void miUndo_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void miRedo_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void miAbout_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
        void btnAdd_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::NamesUWP::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
