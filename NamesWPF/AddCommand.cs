using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Controls;

namespace NamesWPF
{
    public class AddCommand : ICommand
    {
        private ItemCollection listItems;
        private string name;

        public AddCommand(ItemCollection listItems, string name)
        {
            this.listItems = listItems;
            this.name = name;
        }

        public void Execute()
        {
            listItems.Add(name);
        }

        public void Unexecute()
        {
            listItems.Remove(name);
        }
    }
}
