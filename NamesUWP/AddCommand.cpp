#include "pch.h"
#include "AddCommand.h"

namespace winrt::NamesUWP::implementation
{
    void AddCommand::Execute()
    {
        listItems.Append(winrt::box_value(name));
    }

    void AddCommand::Unexecute()
    {
        listItems.RemoveAtEnd();
    }
}
