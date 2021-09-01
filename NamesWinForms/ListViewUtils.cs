using System;
using System.Collections.Generic;
using System.Text;
using static System.Windows.Forms.ListView;

namespace NamesWinForms
{
    public static class ListViewUtils
    {
        public static int FindInListItems(ListViewItemCollection listItems, int index, string value)
        {
            for (int i = 0; i < listItems.Count; i++)
            {
                if (listItems[i].SubItems[index].Text == value)
                {
                    return i;
                }
            }
            return -1;
        }
    }
}
