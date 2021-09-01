#pragma once

namespace winrt::NamesUWP::implementation
{
    class ICommand
    {
    public:
        virtual ~ICommand() {}
        virtual void Execute() = 0;
        virtual void Unexecute() = 0;
    };
}
