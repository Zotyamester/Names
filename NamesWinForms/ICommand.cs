using System;
using System.Collections.Generic;
using System.Text;

namespace NamesWinForms
{
    public interface ICommand
    {
        public void Execute();
        public void Unexecute();
    }
}
