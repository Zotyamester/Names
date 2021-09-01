using System;
using System.Collections.Generic;
using System.Text;

namespace NamesWPF
{
    public interface ICommand
    {
        public void Execute();
        public void Unexecute();
    }
}
