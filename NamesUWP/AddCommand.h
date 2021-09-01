#pragma once

#include "MainPage.h"
#include "ICommand.h"

namespace winrt::NamesUWP::implementation
{
    class AddCommand : public ICommand
    {
    private:
        // buzi fos visual studio
        winrt::Windows::UI::Xaml::Controls::ItemCollection listItems;
        winrt::hstring name;
    public:
        AddCommand(winrt::Windows::UI::Xaml::Controls::ItemCollection listItems, winrt::hstring name) : listItems(listItems), name(name) {}
        virtual void Execute();
        virtual void Unexecute();
    };
}
