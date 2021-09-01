using System;
using System.Collections.Generic;
using System.Text;
using static System.Windows.Forms.ListView;

namespace NamesWinForms
{
    public class AddCommand : ICommand
    {
        private ListViewItemCollection listItems;
        private string name;

        public AddCommand(ListViewItemCollection listItems, string name)
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
            listItems.RemoveAt(ListViewUtils.FindInListItems(listItems, 0, name));
        }
    }
}
