using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace NamesWPF
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private readonly Stack<ICommand> undos;
        private readonly Stack<ICommand> redos;

        public MainWindow()
        {
            undos = new Stack<ICommand>();
            redos = new Stack<ICommand>();

            InitializeComponent();
        }

        public void Clear()
        {
            lstNames.Items.Clear();
            undos.Clear();
            miUndo.IsEnabled = false;
            redos.Clear();
            miRedo.IsEnabled = false;
        }

        private void miNew_Click(object sender, RoutedEventArgs e)
        {
            Clear();
        }

        private void miOpen_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            if (ofd.ShowDialog() == true)
            {
                Clear();
                using (StreamReader sr = new StreamReader(ofd.FileName))
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        lstNames.Items.Add(line);
                    }
                }
            }
        }

        private void miSave_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            if (sfd.ShowDialog() == true)
            {
                using (StreamWriter sw = new StreamWriter(sfd.FileName))
                {
                    foreach (string item in lstNames.Items)
                    {
                        sw.WriteLine(item);
                    }
                }
            }
        }

        private void miExit_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void miUndo_Click(object sender, RoutedEventArgs e)
        {
            ICommand command = undos.Pop();
            command.Unexecute();
            if (undos.Count == 0)
            {
                miUndo.IsEnabled = false;
            }

            redos.Push(command);
            miRedo.IsEnabled = true;
        }

        private void miRedo_Click(object sender, RoutedEventArgs e)
        {
            ICommand command = redos.Pop();
            command.Execute();
            if (redos.Count == 0)
            {
                miRedo.IsEnabled = false;
            }
            undos.Push(command);
            miUndo.IsEnabled = true;
        }

        private void miAbout_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Thank you for using our software!", "About");
        }

        private void btnAdd_Click(object sender, RoutedEventArgs e)
        {
            string name = txtName.Text;
            if (!string.IsNullOrWhiteSpace(name) && !lstNames.Items.Contains(name))
            {
                ICommand command = new AddCommand(lstNames.Items, name);
                command.Execute();
                undos.Push(command);
                miUndo.IsEnabled = true;
                redos.Clear();
                miRedo.IsEnabled = false;

                txtName.Clear();
            }
        }
    }
}
