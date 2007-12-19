#ifdef win32

/* ugly hack here... windef.h and asf_meta.h both define a BYTE symbol. */
/* since we don't use the BYTE from asf_meta.h here, we'll #define BYTE */
/* to something else during the processing of that header, leaving BYTE */
/* defined in windef.h alone (that's the one we want)                   */

#define BYTE __byte
#include "asf.h"
#include "asf_meta.h"
#undef BYTE
#include <windows.h>
#undef DIR_SEPARATOR
#endif

#include "asf_view.h"

extern int stats_calced;
extern int stats_generated;

#ifndef win32

static GtkWidget *browse_widget = NULL;

// called when "cancel" clicked on the GtkFileChooser
SIGNAL_CALLBACK void new_cancel_clicked()
{
    gtk_widget_hide(browse_widget);
}

// called when "ok" clicked on the GtkFileChooser
SIGNAL_CALLBACK void new_ok_clicked()
{
    GSList *files = gtk_file_chooser_get_filenames(
        GTK_FILE_CHOOSER(browse_widget));

    gtk_widget_hide(browse_widget);
    if (files)
    {
        GSList *iter = files;

        do {
          gchar *s = (gchar *) iter->data;
          load_file(s);
          g_free(s);
          iter =  iter->next;
        }
        while(iter);

        g_slist_free(files);
    }
}

SIGNAL_CALLBACK void browse_widget_destroy()
{
    gtk_widget_destroy(browse_widget);
    browse_widget = NULL;
}

// sets up the file chooser dialog
static void create_file_chooser_dialog()
{
    GtkWidget *parent = get_widget_checked("ssv_main_window");

    browse_widget = gtk_file_chooser_dialog_new(
        "Open Image File", GTK_WINDOW(parent),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, //Cancel button
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,   //Open button
        NULL);

    // we need to extract the buttons, so we can connect them to our
    // button handlers, above
    GtkHButtonBox *box =
        (GtkHButtonBox*)(((GtkDialog*)browse_widget)->action_area);
    GList *buttons = box->button_box.box.children;

    GtkWidget *cancel_btn = ((GtkBoxChild*)buttons->data)->widget;
    GtkWidget *ok_btn = ((GtkBoxChild*)buttons->next->data)->widget;

    g_signal_connect((gpointer)cancel_btn, "clicked",
        G_CALLBACK(new_cancel_clicked), NULL);
    g_signal_connect((gpointer)ok_btn, "clicked",
        G_CALLBACK(new_ok_clicked), NULL);
    g_signal_connect(browse_widget, "destroy",
        G_CALLBACK(browse_widget_destroy), NULL);
    g_signal_connect(browse_widget, "destroy_event",
        G_CALLBACK(browse_widget_destroy), NULL);
    g_signal_connect(browse_widget, "delete_event",
        G_CALLBACK(browse_widget_destroy), NULL);

    // add the filters
    GtkFileFilter *D_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(D_filt, "CEOS Data Files (*.D)");
    gtk_file_filter_add_pattern(D_filt, "*.D");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), D_filt);

    GtkFileFilter *alos2_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(alos2_filt, "ALOS Image Files (IMG-*)");
    gtk_file_filter_add_pattern(alos2_filt, "IMG-*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), alos2_filt);

    GtkFileFilter *img_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(img_filt, "ASF Internal Files (*.img)");
    gtk_file_filter_add_pattern(img_filt, "*.img");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), img_filt);

    GtkFileFilter *tiff_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(tiff_filt, "GeoTIFF Image Files (*.tif)");
    gtk_file_filter_add_pattern(tiff_filt, "*.tif");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), tiff_filt);

    GtkFileFilter *jpg_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(jpg_filt, "JPEG Image Files (*.jpg)");
    gtk_file_filter_add_pattern(jpg_filt, "*.jpg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), jpg_filt);

    GtkFileFilter *png_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(png_filt, "PNG Image Files (*.png)");
    gtk_file_filter_add_pattern(png_filt, "*.png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), png_filt);

    GtkFileFilter *pgm_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(pgm_filt, "PGM Image Files (*.pgm)");
    gtk_file_filter_add_pattern(pgm_filt, "*.pgm");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), pgm_filt);

    //GtkFileFilter *ppm_filt = gtk_file_filter_new();
    //gtk_file_filter_set_name(ppm_filt, "PPM Image Files (*.ppm)");
    //gtk_file_filter_add_pattern(ppm_filt, "*.ppm");
    //gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), ppm_filt);

    GtkFileFilter *all_filt = gtk_file_filter_new();
    gtk_file_filter_set_name(all_filt, "All Files (*.*)");
    gtk_file_filter_add_pattern(all_filt, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(browse_widget), all_filt);

    // allow multi-select
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(browse_widget), TRUE);

    // we need to make these modal -- if the user opens multiple "open"
    // dialogs, we'll get confused on the callbacks
    gtk_window_set_modal(GTK_WINDOW(browse_widget), TRUE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(browse_widget), TRUE);
    gtk_dialog_set_default_response(GTK_DIALOG(browse_widget),
                                    GTK_RESPONSE_OK);
}
#endif

void new_file(void)
{
#ifdef win32
    OPENFILENAME of;
    int retval;
    char fname[1024];

    fname[0] = '\0';

    memset(&of, 0, sizeof(of));

#ifdef OPENFILENAME_SIZE_VERSION_400
    of.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
    of.lStructSize = sizeof(of);
#endif

    of.hwndOwner = NULL;
    of.lpstrFilter = "CEOS Level 1 Data Files (*.D)\0*.D\0"
        "ALOS Image (IMG-*)\0IMG-*\0"
        "ASF Internal (*.img)\0*.img\0"
        "GeoTIFF Image (*.tif)\0*.tif\0"
        "JPG (*.jpg)\0*.jpg\0"
        "PNG (*.png)\0*.png\0"
        "PGM (*.pgm)\0*.pgm\0"
        "All Files\0*\0";
    of.lpstrCustomFilter = NULL;
    of.nFilterIndex = 1;
    of.lpstrFile = fname;
    of.nMaxFile = sizeof(fname);
    of.lpstrFileTitle = NULL;
    of.lpstrInitialDir = ".";
    of.lpstrTitle = "Select File";
    of.lpstrDefExt = NULL;
    of.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;

    retval = GetOpenFileName(&of);

    if (!retval) {
        if (CommDlgExtendedError())
            message_box("File dialog box error");
        return;
    }

    load_file(fname);

#else // #ifdef win32

    if (!browse_widget)
        create_file_chooser_dialog();

    gtk_widget_show(browse_widget);
#endif // #ifdef win32
}

void set_title(int band_specified, const char *band_in)
{
    char title[256];
    char band[128];

    if (band_in)
        strncpy_safe(band, band_in, sizeof(band));
    else
        strcpy(band, "");

    meta_parameters *meta = curr->meta;
    char *basename = get_basename(curr->filename);
    snprintf(title, 239, "asf_view ver %s: %s", VERSION, basename);

    if (band_specified) {
        sprintf(&title[strlen(title)], " (%s)", band);
    } else if (meta && meta->general && meta->general->band_count > 1) {
        if (strlen(meta->general->bands) > 0) {
            strncpy_safe(band, meta->general->bands, sizeof(band));
            char *p = strchr(band, ',');
            if (p) *p = '\0';
        } else if (strncmp_case(basename, "IMG-", 4) == 0) {
            strncpy_safe(band, basename+4, sizeof(band));
            char *p = strchr(band, '-');
            if (p) *p = '\0';
        } else {
            strcpy(band, "");
        }
        if (strlen(band) > 0)
            sprintf(&title[strlen(title)], " (%s)", band);
    }
    free(basename);

    GtkWidget *widget = get_widget_checked("ssv_main_window");
    gtk_window_set_title(GTK_WINDOW(widget), title);
}

void reset_globals(int reset_location)
{
  //clear_stats();

    if (reset_location) {
        center_line = center_samp = crosshair_samp = crosshair_line = -1;
        zoom = 1;
        g_poly.n = g_poly.c = 0;
    }

    //data_ci = NULL;
    //meta = NULL;
}

static void load_file_banded_imp(const char *file, const char *band,
                                 int reset_location, int multilook)
{
    char *old_file = NULL;
    if (curr->filename) {
      old_file = STRDUP(curr->filename);
      free(curr->filename);
    }

    reset_globals(reset_location);
    asfPrintStatus("\nLoading: %s\n", file);

    // start loading of the new file
    curr->filename = STRDUP(file);

    // strip off a trailing "."
    if (curr->filename[strlen(curr->filename)-1] == '.')
        curr->filename[strlen(curr->filename)-1] = '\0';

    if (read_file(curr->filename, band, multilook, FALSE)) {
      if (reset_location && curr->meta && curr->meta->general)
        set_lut_based_on_image_type(curr->meta->general->image_data_type);
      set_title(band != NULL, band);
      check_lut();

      // load the metadata & image data, other setup
      fill_small_force_reload(curr);
      fill_big(curr);
      update_pixel_info(curr);
      update_zoom();
      fill_meta_info();
      fill_stats(curr);
      setup_bands_tab(curr->meta);

      FREE(old_file);
    }
    else {
      FREE(curr->filename);
      curr->filename = old_file;

      if (reset_location) {
        center_samp = (double)(curr->ns)/2.;
        center_line = (double)(curr->nl)/2.;
        crosshair_samp = (double)(curr->ns)/2.;
        crosshair_line = (double)(curr->nl)/2.;
      }
    }
}

void reload_file_banded(const char *file, const char *band, int multilook)
{
    load_file_banded_imp(file, band, FALSE, multilook);
}

void load_file_banded(const char *file, const char *band, int multilook)
{
    load_file_banded_imp(file, band, TRUE, multilook);
}

void load_file(const char *file)
{
    load_file_banded(file, NULL, FALSE);
}

SIGNAL_CALLBACK void on_new_button_clicked(GtkWidget *w)
{
    new_file();
}
