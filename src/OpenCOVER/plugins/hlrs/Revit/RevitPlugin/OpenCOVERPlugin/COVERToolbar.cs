using System;
using Autodesk;
using Autodesk.Revit;
using System.Threading;
using System.IO;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Windows.Media;
using System.Windows.Forms;
using System.Windows.Media.Imaging;
using Autodesk.Revit.DB;
using Autodesk.Revit.UI;



namespace OpenCOVERPlugin
{
    public sealed class geometrySender
    {
        public static Autodesk.Revit.UI.Result DoSend(ExternalCommandData commandData)
        {

            using (Transaction transaction = new Transaction(commandData.Application.ActiveUIDocument.Document))
            {
                if (transaction.Start("changeParameters") == TransactionStatus.Started)
                {
                    // ElementId activeOptId = Autodesk.Revit.DB.DesignOption.GetActiveDesignOptionId(commandData.Application.ActiveUIDocument.Document);

                    //ElementDesignOptionFilter filter = new ElementDesignOptionFilter(activeOptId);

                    Autodesk.Revit.DB.FilteredElementCollector collector = new Autodesk.Revit.DB.FilteredElementCollector(commandData.Application.ActiveUIDocument.Document);
                    COVER.Instance.SendGeometry(collector./*WherePasses(filter).*/WhereElementIsNotElementType().GetElementIterator(), commandData.Application);

                    ElementClassFilter FamilyFilter = new ElementClassFilter(typeof(FamilySymbol));
                    FilteredElementCollector FamilyCollector = new FilteredElementCollector(commandData.Application.ActiveUIDocument.Document);
                    ICollection<Element> AllFamilies = FamilyCollector.WherePasses(FamilyFilter).ToElements();
                    foreach (FamilySymbol Fmly in AllFamilies)
                    {
                        COVER.Instance.sendFamilySymbolParameters(Fmly);
                        /* string FamilyName = Fmly.Name;
                         foreach (Parameter Param in Fmly.Parameters)
                         {
                             string ParamName = Param.Definition.Name;
                         }*/
                    }
                    /*
                    IEnumerable<Element> familiesCollector =
                    new FilteredElementCollector(commandData.Application.ActiveUIDocument.Document).OfClass(typeof (FamilyInstance))
                    .WhereElementIsNotElementType()
                    .Cast<FamilyInstance>()
                    .GroupBy(fi => fi.Symbol.Family) // (family,familyInstances)
                    .Select(f=>f.Key);

                    foreach (var f in familiesCollector)
                    {
                        COVER.Instance.sendParameters(f);
                    }*/

                    if (TransactionStatus.Committed != transaction.Commit())
                    {
                        TaskDialog.Show("Failure", "Transaction could not be committed");
                    }
                    return Autodesk.Revit.UI.Result.Succeeded;
                }
            }
            return Autodesk.Revit.UI.Result.Failed;
        }
    }
    /// <summary>
    /// Send all Elements to OpenCOVER.
    /// </summary>
    [Autodesk.Revit.Attributes.Transaction(Autodesk.Revit.Attributes.TransactionMode.Manual)]
    [Autodesk.Revit.Attributes.Regeneration(Autodesk.Revit.Attributes.RegenerationOption.Manual)]
    public class SendGeometry : IExternalCommand
    {
        #region IExternalCommand Members
        /// <summary>
        /// Implement this method as an external command for Revit.
        /// </summary>
        /// <param name="commandData">An object that is passed to the external application
        /// which contains data related to the command,
        /// such as the application object and active view.</param>
        /// <param name="message">A message that can be set by the external application
        /// which will be displayed if a failure or cancellation is returned by
        /// the external command.</param>
        /// <param name="elements">A set of elements to which the external application
        /// can add elements that are to be highlighted in case of failure or cancellation.</param>
        /// <returns>Return the status of the external command.
        /// A result of Succeeded means that the API external method functioned as expected.
        /// Cancelled can be used to signify that the user cancelled the external operation 
        /// at some point. Failure should be returned if the application is unable to proceed with
        /// the operation.</returns>
        public Autodesk.Revit.UI.Result Execute(ExternalCommandData commandData,
            ref string message, ElementSet elements)
        {

            return geometrySender.DoSend(commandData);

        }
        #endregion
    }

    /// <summary>
    /// connect to local Opencover.
    /// </summary>
    [Autodesk.Revit.Attributes.Transaction(Autodesk.Revit.Attributes.TransactionMode.Manual)]
    [Autodesk.Revit.Attributes.Regeneration(Autodesk.Revit.Attributes.RegenerationOption.Manual)]
    public class ConnectToCOVER : IExternalCommand
    {
        #region IExternalCommand Members

        /// <summary>
        /// Implement this method as an external command for Revit.
        /// </summary>
        /// <param name="commandData">An object that is passed to the external application
        /// which contains data related to the command,
        /// such as the application object and active view.</param>
        /// <param name="message">A message that can be set by the external application
        /// which will be displayed if a failure or cancellation is returned by
        /// the external command.</param>
        /// <param name="elements">A set of elements to which the external application
        /// can add elements that are to be highlighted in case of failure or cancellation.</param>
        /// <returns>Return the status of the external command.
        /// A result of Succeeded means that the API external method functioned as expected.
        /// Cancelled can be used to signify that the user cancelled the external operation 
        /// at some point. Failure should be returned if the application is unable to proceed with
        /// the operation.</returns>
        public Autodesk.Revit.UI.Result Execute(ExternalCommandData commandData,
            ref string message, ElementSet elements)
        {
            ConnectionDialog d = new ConnectionDialog();
            d.ShowDialog();
            if (d.connect())
            {
                COVER.Instance.ConnectToOpenCOVER(d.getHostname(), d.getPort(), commandData.Application.ActiveUIDocument.Document);
            }
            return geometrySender.DoSend(commandData);
            //return Autodesk.Revit.UI.Result.Succeeded;
        }

        #endregion
    }
    /// <summary>
    /// connect to OpenCOVER on visent.hlrs.de.
    /// </summary>
    /// 
    [Autodesk.Revit.Attributes.Transaction(Autodesk.Revit.Attributes.TransactionMode.Manual)]
    [Autodesk.Revit.Attributes.Regeneration(Autodesk.Revit.Attributes.RegenerationOption.Manual)]
    public class ConnectToCAVE : IExternalCommand
    {
        #region IExternalCommand Members

        /// <summary>
        /// Implement this method as an external command for Revit.
        /// </summary>
        /// <param name="commandData">An object that is passed to the external application
        /// which contains data related to the command,
        /// such as the application object and active view.</param>
        /// <param name="message">A message that can be set by the external application
        /// which will be displayed if a failure or cancellation is returned by
        /// the external command.</param>
        /// <param name="elements">A set of elements to which the external application
        /// can add elements that are to be highlighted in case of failure or cancellation.</param>
        /// <returns>Return the status of the external command.
        /// A result of Succeeded means that the API external method functioned as expected.
        /// Cancelled can be used to signify that the user cancelled the external operation 
        /// at some point. Failure should be returned if the application is unable to proceed with
        /// the operation.</returns>
        public Autodesk.Revit.UI.Result Execute(ExternalCommandData commandData,
            ref string message, ElementSet elements)
        {
            string CAVEHost = "visent.hlrs.de";
            try
            {
                CAVEHost = System.Environment.GetEnvironmentVariable("CAVEHOST");
            }
            catch
            {
            }
            if (CAVEHost == null || CAVEHost.Length == 0)
            {
                CAVEHost = "visent.hlrs.de";
            }
            COVER.Instance.ConnectToOpenCOVER(CAVEHost, 31821, commandData.Application.ActiveUIDocument.Document);
            return geometrySender.DoSend(commandData);
            //return Autodesk.Revit.UI.Result.Succeeded;
        }

        #endregion
    }
    /// <summary>
     /// connect to OpenCOVER on localhost.
     /// </summary>
     /// 
    [Autodesk.Revit.Attributes.Transaction(Autodesk.Revit.Attributes.TransactionMode.Manual)]
    [Autodesk.Revit.Attributes.Regeneration(Autodesk.Revit.Attributes.RegenerationOption.Manual)]
    public class ConnectToLocalHost : IExternalCommand
    {
        #region IExternalCommand Members
        
        public Autodesk.Revit.UI.Result Execute(ExternalCommandData commandData,
            ref string message, ElementSet elements)
        {
            COVER.Instance.ConnectToOpenCOVER("localhost", 31821, commandData.Application.ActiveUIDocument.Document);
            return geometrySender.DoSend(commandData);
            //return Autodesk.Revit.UI.Result.Succeeded;
        }

        #endregion
    }
    /// <summary>
    /// Class implements the Revit interface IExternalApplication to create a custom tool bar.
    /// </summary
    /// 
    [Autodesk.Revit.Attributes.Transaction(Autodesk.Revit.Attributes.TransactionMode.Manual)]
    [Autodesk.Revit.Attributes.Regeneration(Autodesk.Revit.Attributes.RegenerationOption.Manual)]
    [Autodesk.Revit.Attributes.Journaling(Autodesk.Revit.Attributes.JournalingMode.NoCommandData)]
    class COVERToolbar : IExternalApplication
    {

        #region Class Member Variables
        #endregion
        /// <summary>
        /// Implement this method to implement the external application which should be called when 
        /// Revit starts before a file or default template is actually loaded.
        /// </summary>
        /// <param name="application">An object that is passed to the external application 
        /// which contains the controlled application.</param>
        /// <returns>Return the status of the external application. 
        /// A result of Succeeded means that the external application successfully started. 
        /// Cancelled can be used to signify that the user cancelled the external operation at 
        /// some point.
        /// If false is returned then Revit should inform the user that the external application 
        /// failed to load and the release the internal reference.</returns>
        public Autodesk.Revit.UI.Result OnStartup(UIControlledApplication application)
        {
            try
            {
                // application path
                string path = this.GetType().Assembly.Location;


                // image of toolbar
                string imagePath = path.Replace("OpenCOVERPlugin.dll", "Toolbar.bmp");


                // add menu item in Revit menu bar to provide an approach to 
                // retrieve events setting form. User can change his choices 
                // by calling the setting form again.
                AddCustomPanel(application);


                // register the DocumentChanged event
                application.ControlledApplication.DocumentChanged += new EventHandler<Autodesk.Revit.DB.Events.DocumentChangedEventArgs>(CtrlApp_DocumentChanged);

                OpenCOVERPlugin.COVER.Instance.startup(application);


            }
            catch (System.Exception)
            {
                return Autodesk.Revit.UI.Result.Failed;
            }

            return Autodesk.Revit.UI.Result.Succeeded;
        }/// <summary>
        /// Add custom menu.
        /// </summary>
        static private void AddCustomPanel(UIControlledApplication application)
        {
            // create a panel named "Events Monitor";
            string panelName = "Virtual Reality";
            
            // create a button on the panel.
            RibbonPanel ribbonPanelPushButtons = application.CreateRibbonPanel(panelName);

            COVER.Instance.pushButtonConnectToOpenCOVER = ribbonPanelPushButtons.AddItem(new PulldownButtonData("Connect To OpenCOVER",
                "Connection")) as PulldownButton;
            COVER.Instance.pushButtonConnectToOpenCOVER.ToolTip = Path.Combine(COVER.Instance.ButtonIconsFolder, "cover_disconnected.png"); // "Connect to OpenCOVER";

            COVER.Instance.pushButtonConnectToOpenCOVER.LargeImage = new BitmapImage(new Uri(Path.Combine(COVER.Instance.ButtonIconsFolder, "cover_disconnected_32.png"), UriKind.Absolute));


            PushButton pushButtonSendGeometry = ribbonPanelPushButtons.AddItem(new PushButtonData("resend",
                "resend", System.Reflection.Assembly.GetExecutingAssembly().Location,
                "OpenCOVERPlugin.SendGeometry")) as PushButton;
            pushButtonSendGeometry.ToolTip = "Send Geometry to OpenCOVER";

            pushButtonSendGeometry.LargeImage = new BitmapImage(new Uri(Path.Combine(COVER.Instance.ButtonIconsFolder, "resend_32.png"), UriKind.Absolute));

            //pushButtonConnectToOpenCOVER.Image = new BitmapImage(new Uri(Path.Combine(ButtonIconsFolder, "cover_disconnected_16.png"), UriKind.Absolute));



            PushButton pushButtonConnectToOpenCAVE = COVER.Instance.pushButtonConnectToOpenCOVER.AddPushButton(new PushButtonData("Connect To CAVE",
                "CAVE", System.Reflection.Assembly.GetExecutingAssembly().Location,
                "OpenCOVERPlugin.ConnectToCAVE")) as PushButton;
            pushButtonConnectToOpenCAVE.ToolTip = "Connect to CAVE";
            pushButtonConnectToOpenCAVE.LargeImage = new BitmapImage(new Uri(Path.Combine(COVER.Instance.ButtonIconsFolder, "cave32.png"), UriKind.Absolute));

            PushButton pushButtonConnectToLocalHost = COVER.Instance.pushButtonConnectToOpenCOVER.AddPushButton(new PushButtonData("Connect To LocalHost",
                 "LocalHost", System.Reflection.Assembly.GetExecutingAssembly().Location,
                 "OpenCOVERPlugin.ConnectToLocalHost")) as PushButton;
            pushButtonConnectToLocalHost.ToolTip = "Connect to Revit on localhost";
            pushButtonConnectToLocalHost.LargeImage = new BitmapImage(new Uri(Path.Combine(COVER.Instance.ButtonIconsFolder, "localhost.png"), UriKind.Absolute));

            PushButton pushButtonConnectToAnyHost = COVER.Instance.pushButtonConnectToOpenCOVER.AddPushButton(new PushButtonData("Connect To OpenCOVER Prompt",
                 "Details", System.Reflection.Assembly.GetExecutingAssembly().Location,
                 "OpenCOVERPlugin.ConnectToCOVER")) as PushButton;
            pushButtonConnectToAnyHost.ToolTip = "Connect to OpenCOVER";
            pushButtonConnectToAnyHost.LargeImage = new BitmapImage(new Uri(Path.Combine(COVER.Instance.ButtonIconsFolder, "prompt.png"), UriKind.Absolute));





            /* PushButton pushButtonExportCommand = ribbonPanelPushButtons.AddItem(new PushButtonData("RenderToCOVER",
                 "ResendRender", System.Reflection.Assembly.GetExecutingAssembly().Location,
                 "OpenCOVERPlugin.ExportCommand")) as PushButton;
             pushButtonExportCommand.ToolTip = "Send Geometry to OpenCOVER through the rendering interface";
             pushButtonExportCommand.LargeImage = new BitmapImage(new Uri(Path.Combine(COVER.Instance.ButtonIconsFolder, "resend_32.png"), UriKind.Absolute));*/


        }
        /// <summary>
        /// This method is the event handler, which will dump the change information to tracking dialog
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void CtrlApp_DocumentChanged(object sender, Autodesk.Revit.DB.Events.DocumentChangedEventArgs e)
        {
            // get the current document.
            Document doc = e.GetDocument();

            List<int> IDs = new List<int>();

            // dump the element information
            ICollection<Autodesk.Revit.DB.ElementId> addedElem = e.GetAddedElementIds();
            foreach (ElementId id in addedElem)
            {
                OpenCOVERPlugin.COVER.Instance.SendElement(doc.GetElement(id));
            }

            ICollection<Autodesk.Revit.DB.ElementId> deletedElem = e.GetDeletedElementIds();
            foreach (ElementId id in deletedElem)
            {
                OpenCOVERPlugin.COVER.Instance.deleteElement(id);
            }

            ICollection<ElementId> modifiedElem = e.GetModifiedElementIds();
            foreach (ElementId id in modifiedElem)
            {
                OpenCOVERPlugin.COVER.Instance.deleteElement(id);
                OpenCOVERPlugin.COVER.Instance.SendElement(doc.GetElement(id));
            }

        }

        /// <summary>
        /// Implement this method to implement the external application which should be called when 
        /// Revit is about to exit,Any documents must have been closed before this method is called.
        /// </summary>
        /// <param name="application">An object that is passed to the external application 
        /// which contains the controlled application.</param>
        /// <returns>Return the status of the external application. 
        /// A result of Succeeded means that the external application successfully shutdown. 
        /// Cancelled can be used to signify that the user cancelled the external operation at 
        /// some point.
        /// If false is returned then the Revit user should be warned of the failure of the external 
        /// application to shut down correctly.</returns>
        public Autodesk.Revit.UI.Result OnShutdown(UIControlledApplication application)
        {
            application.ControlledApplication.DocumentChanged -= CtrlApp_DocumentChanged;

            OpenCOVERPlugin.COVER.Instance.shutdown(application);
            return Autodesk.Revit.UI.Result.Succeeded;
        }
    }

    [Autodesk.Revit.Attributes.Transaction(Autodesk.Revit.Attributes.TransactionMode.Manual)]
    public class ExportCommand : IExternalCommand
    {
        /// <summary>
        /// The main (and only :-) command 
        /// of our export sample
        /// </summary>
        public Result Execute(
          ExternalCommandData data,
          ref string msg,
          ElementSet elements)
        {
            // This command requires an active document

            UIDocument uidoc = data.Application
              .ActiveUIDocument;

            if (uidoc == null)
            {
                msg = "Please run this command in an active project document.";
                return Result.Failed;
            }

            Document doc = uidoc.Document;

            // Collect all 3D views in the document 
            // (there must be at least one). The collection
            // will be listed in the option dialog for the
            // user to choose the ones to include in the
            // export.
            List<View3D> views = new List<View3D>(
         new FilteredElementCollector(doc)
           .OfClass(typeof(View3D))
           .Cast<View3D>()
           .Where<View3D>(v =>
             v.CanBePrinted && !v.IsTemplate));

            int n = views.Count;

            if (0 == n)
            {
                msg = "There are no 3D views in the document!";
                return Result.Failed;
            }

            // Initiate output with default path to 
            // MyDocuments and the current document name

            string defaultName = Path.ChangeExtension(
              doc.Title, ".xml");

            string defaultFolder = System.Environment
              .GetFolderPath(
                Environment.SpecialFolder.MyDocuments);




            // Instantiate our custom context

            ExportContext context = new ExportContext(doc);

            // Instantiate a custom exporter with output 
            // context given as the argument

            using (CustomExporter exporter
              = new CustomExporter(doc, context))
            {

                List<ElementId> viewsToExport = new List<ElementId>();

                foreach (View3D v in views)
                {
                    viewsToExport.Add(v.Id);
                }
                try
                {
                    exporter.Export(viewsToExport);
                }
                catch (System.Exception ex)
                {
                    msg = "Exception: " + ex.Message;
                    return Result.Failed;
                }
            }
            return Result.Succeeded;
        }
    }
}
