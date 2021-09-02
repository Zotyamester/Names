using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NamesWinForms
{
    public partial class MainForm : Form
    {
        private readonly Stack<ICommand> undos;
        private readonly Stack<ICommand> redos;

        public MainForm()
        {
            undos = new Stack<ICommand>();
            redos = new Stack<ICommand>();

            InitializeComponent();
        }

        public void Clear()
        {
            lvNames.Items.Clear();
            undos.Clear();
            miUndo.Enabled = false;
            redos.Clear();
            miRedo.Enabled = false;
        }

        private void miNew_Click(object sender, EventArgs e)
        {
            Clear();
        }

        private void miOpen_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                Clear();
                using (StreamReader sr = new StreamReader(ofd.FileName, Encoding.Unicode))
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        if (!string.IsNullOrWhiteSpace(line) && ListViewUtils.FindInListItems(lvNames.Items, 0, line) == -1)
                        {
                            lvNames.Items.Add(line);
                        }
                    }
                }
            }
        }

        private void miSave_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            if (sfd.ShowDialog() == DialogResult.OK)
            {
                using (StreamWriter sw = new StreamWriter(sfd.FileName, false, Encoding.Unicode))
                {
                    for (int i = 0; i < lvNames.Items.Count; i++)
                    {
                        sw.WriteLine(lvNames.Items[i].SubItems[0].Text);
                    }
                }
            }
        }

        private void miExit_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void miUndo_Click(object sender, EventArgs e)
        {
            ICommand command = undos.Pop();
            command.Unexecute();
            if (undos.Count == 0)
            {
                miUndo.Enabled = false;
            }

            redos.Push(command);
            miRedo.Enabled = true;
        }

        private void miRedo_Click(object sender, EventArgs e)
        {
            ICommand command = redos.Pop();
            command.Execute();
            if (redos.Count == 0)
            {
                miRedo.Enabled = false;
            }

            undos.Push(command);
            miUndo.Enabled = true;
        }

        private void miAbout_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Thank you for using our software!", "About");
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            string name = tbName.Text;
            if (!string.IsNullOrWhiteSpace(name) && ListViewUtils.FindInListItems(lvNames.Items, 0, name) == -1)
            {
                ICommand command = new AddCommand(lvNames.Items, name);
                command.Execute();
                undos.Push(command);
                miUndo.Enabled = true;
                redos.Clear();
                miRedo.Enabled = false;

                tbName.Clear();
            }
        }
    }
}
