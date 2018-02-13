#include "ScissorWindow.h"
#include <gtk/gtk.h>

int main(int argc, char *argv[])
{
      gtk_init(&argc, &argv);
      ScissorWindow sw;
      gtk_main();
      return 0;
}