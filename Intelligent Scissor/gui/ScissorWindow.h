#ifndef SCISSORWINDOW_H
#define SCISSORWINDOW_H

#include <gtkmm.h>
#include <gdk/gdkkeysyms.h>

class ScissorWindow : public Gtk::ApplicationWindow
{
public:
  ScissorWindow();
  virtual ~ScissorWindow();

protected:
  GtkWidget *mainWindow;
  GtkWidget *mainBox;
  GtkWidget *image;

  // Top menu
  GtkWidget *menubar;

  GtkWidget *fileMenu;
  GtkWidget *fileList;
  GtkWidget *fileSaveContour;
  GtkWidget *fileSaveMask;

  GtkWidget *toolMenu;
  GtkWidget *toolList;
  GtkWidget *toolScissor;
  GtkWidget *toolDebug;

  // // Popup menu
  // GtkWidget *eventBox;

  // GtkWidget *popupMenu;
  // GtkWidget *popupActive;
  // GtkWidget *popupDebug;
};

#endif // SCISSORWINDOW_H
