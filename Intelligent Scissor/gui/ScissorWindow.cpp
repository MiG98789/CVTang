#include "ScissorWindow.h"
#include <iostream>

bool debug = false;
bool controlPressed = false;

void toggleDebug(GtkWidget *widget, gpointer statusbar)
{
	::debug = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
	std::cout << (::debug ? "Debug Mode\n" : "Active Mode\n");
}

void keyPressEvent(GtkWidget *widget, GdkEventKey *event)
{
	switch (event->keyval)
	{
	case GDK_KEY_Control_L:
	case GDK_KEY_Control_R:
		controlPressed = true;
		std::cout << "Ctrl pressed\n";
		break;

	case GDK_KEY_plus:
		if (controlPressed)
		{
			std::cout << "Ctrl + +\n";
		}
		break;

	case GDK_KEY_minus:
		if (controlPressed)
		{
			std::cout << "Ctrl + -\n";
		}
		break;

	case GDK_KEY_Return:
		if (controlPressed)
		{
			std::cout << "Ctrl + Enter\n";
		}
		else
		{
			std::cout << "Enter\n";
		}
		break;

	case GDK_KEY_BackSpace:
		std::cout << "Backspace\n";
		break;

	default:
		break;
	}
}

void keyReleaseEvent(GtkWidget *widget, GdkEventKey *event)
{
	switch (event->keyval)
	{
	case GDK_KEY_Control_L:
	case GDK_KEY_Control_R:
		controlPressed = false;
		std::cout << "Ctrl released\n";
		break;

	default:
		break;
	}
}

void mouseClickEvent(GtkWidget *widget, GdkEvent *event)
{
	const gint LEFT_CLICK = 1;
	const gint RIGHT_CLICK = 3;

	GdkEventButton *bevent = (GdkEventButton *)event;
	GdkEventMotion *mevent = (GdkEventMotion *)event;

	if (event->type == GDK_BUTTON_PRESS && bevent->button == LEFT_CLICK)
	{
		if (controlPressed)
		{
			std::cout << "Ctrl + Left click";
		}
		else
		{
			std::cout << "Left click";
		}
	}
	if (event->type == GDK_BUTTON_PRESS && bevent->button == RIGHT_CLICK)
	{
		std::cout << "Right click";
	}
	std::cout << " at (" << mevent->x << "," << mevent->y << ")\n";
}

ScissorWindow::ScissorWindow()
{
	mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(mainWindow), 300, 200);
	gtk_window_set_title(GTK_WINDOW(mainWindow), "Intelligent Scissors");

	mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(mainWindow), mainBox);

	image = gtk_image_new_from_file("../curless.png");
	gtk_box_pack_start(GTK_BOX(mainBox), image, FALSE, FALSE, 0);

	// Top menu
	menubar = gtk_menu_bar_new();
	fileMenu = gtk_menu_new();
	toolMenu = gtk_menu_new();

	fileList = gtk_menu_item_new_with_label("File");
	fileSaveContour = gtk_menu_item_new_with_label("Save Contour");
	fileSaveMask = gtk_menu_item_new_with_label("Save Mask");

	toolList = gtk_menu_item_new_with_label("Tool");
	toolScissor = gtk_menu_item_new_with_label("Scissor");
	toolDebug = gtk_check_menu_item_new_with_label("Debug Mode");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileList), fileMenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileSaveContour);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), fileSaveMask);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(toolList), toolMenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolMenu), toolScissor);
	gtk_menu_shell_append(GTK_MENU_SHELL(toolMenu), toolDebug);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toolDebug), FALSE);

	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileList);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), toolList);

	gtk_box_pack_start(GTK_BOX(mainBox), menubar, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(mainWindow), "button_press_event", G_CALLBACK(mouseClickEvent), NULL);
	g_signal_connect(G_OBJECT(mainWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(mainWindow), "key_press_event", G_CALLBACK(keyPressEvent), NULL);
	g_signal_connect(G_OBJECT(mainWindow), "key_release_event", G_CALLBACK(keyReleaseEvent), NULL);
	g_signal_connect(G_OBJECT(toolDebug), "activate", G_CALLBACK(toggleDebug), NULL);

	gtk_widget_show_all(mainWindow);
}

ScissorWindow::~ScissorWindow()
{
}
