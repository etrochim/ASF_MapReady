#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <limits.h>

#include <asf.h>
#include <asf_endian.h>
#include <asf_meta.h>
#include <asf_raster.h>
#include <asf_reporting.h>
#include <asf_contact.h>
#include <asf_copyright.h>
#include <asf_license.h>
#include <asf_sar.h>
#include <poly.h>

int
int_rnd(double x)
{
  return (int)floor(x+0.5);
}

char * change_extension(const char * file, const char * ext)
{
  char * replaced = (char *)
    MALLOC(sizeof(char) * (strlen(file) + strlen(ext) + 10));
  
  strcpy(replaced, file);
  char * p = findExt(file);
  
  if (p)
    *p = '\0';
  
  strcat(replaced, ".");
  strcat(replaced, ext);
  
  return replaced;
}

char * appendSuffix(const char *inFile, const char *suffix)
{
  char *suffix_pid = MALLOC(sizeof(char)*(strlen(suffix)+25));
  sprintf(suffix_pid, "%s_tctmp%d", suffix, (int)getpid());

  char *ret = MALLOC(sizeof(char)*(strlen(inFile)+strlen(suffix_pid)+5));
  strcpy(ret, inFile);
  char *p = findExt(ret);
  if (p && p != ret) {
    char *ext;
    *p++ = '\0';
    ext = strdup(p);
    strcat(ret, suffix_pid);
    strcat(ret, ".");
    strcat(ret, ext);
    free(ext);
  } else {
    strcat(ret, suffix_pid);
  }

  free(suffix_pid);
  return ret;
}

void ensure_ext(char **filename, const char *ext)
{
  char *ret = MALLOC(sizeof(char)*(strlen(*filename)+strlen(ext)+5));
  strcpy(ret, *filename);

  // blow away current extension if necessary
  char *p = findExt(ret);
  if (p) *p = '\0';

  if (ext[0] != '.') strcat(ret, ".");
  strcat(ret, ext);

  free(*filename);
  *filename = ret;
}

void read_corr(const char *corrFile, double *dx, double *dy)
{
  FILE *cf = FOPEN(corrFile, "rt");
  if (cf) {
    fscanf(cf, "%lf %lf", dx, dy);
  } else {
    asfPrintError("Couldn't open fftMatch correlation file: %s!\n", corrFile);
    *dx = *dy = 0;
  }
  fclose(cf);
}

int file_exists(const char * file)
{
  int fd = open(file, 0);
  int stat = fd >= 3;
  close(fd);
  return stat;
}

void remove_file(const char * file)
{
  if (file_exists(file)) {
    // asfPrintStatus("Removing intermediate file: %s\n", file);
    unlink(file);
  }
}

// attempt to remove "<file>.img" and "<file>.meta", etc files
void clean(const char *file)
{
  char * img_file = change_extension(file, "img");
  char * meta_file = change_extension(file, "meta");
  char * ddr_file = change_extension(file, "ddr");

  remove_file(img_file);
  remove_file(meta_file);
  remove_file(ddr_file);
  remove_file(file);

  free(img_file);
  free(meta_file);
  free(ddr_file);
}

#define NUM_ARGS 3
void usage(const char *name)
{
  printf("Usage: %s [-log <logfile>] [-quiet] [-keep] [-no-resample]\n"
         "          [-no-verify-fftMatch] [-pixel-size <size>]\n"
         "          [-dem-grid-size <size>]\n"
         "          <inFile> <demFile> <outFile>\n", name);
  exit(EXIT_FAILURE);
}

int strmatches(const char *key, ...)
{
  va_list ap;
  char *arg = NULL;
  int found = FALSE;

  va_start(ap, key);
  do {
    arg = va_arg(ap, char *);
    if (arg) {
      if (strcmp(key, arg) == 0) {
	found = TRUE;
	break;
      }
    }
  } while (arg);

  return found;
}

void fftMatchQ(char *file1, char *file2, char *out1, char *out2)
{
  int qf_saved = quietflag;
  quietflag = 1;
  fftMatch(file1, file2, out1, out2);
  quietflag = qf_saved;
}

// Main program body.
int
main (int argc, char *argv[])
{
  char *inFile, *demFile, *resampleFile, *srFile, *outFile;
  char *demGridFile, *demPolyFile, *demClipped, *demSlant, *demSimAmp;
  char *demTrimSimAmp, *corrFile, *corrFile2, *demTrimSlant;
  double demRes, sarRes;
  int demWidth, demHeight;
  meta_parameters *metaSAR, *metaDEM;
  double dx, dy, azScale, pixel_size = -1, dem_grid_size = 20;
  int currArg, idx, idy;
  int polyOrder = 5, clean_files = TRUE, do_resample = TRUE,
    do_fftMatch_verification = TRUE;

  currArg = 1;

  while (currArg < (argc-NUM_ARGS)) {
    char *key = argv[currArg++];
    if (strmatches(key,"-log","--log",NULL)) {
      CHECK_ARG(1);
      strcpy(logFile,GET_ARG(1));
      fLog = FOPEN(logFile, "a");
      logflag = TRUE;
    }
    else if (strmatches(key,"-quiet","--quiet","-q",NULL)) {
      quietflag = TRUE;
    }
    else if (strmatches(key,"-keep","--keep","-k",NULL)) {
      clean_files = FALSE;
    }
    else if (strmatches(key,"-no-resample","--no-resample",NULL)) {
      do_resample = FALSE;
    }
    else if (strmatches(key,"-no-verify-match","--no-verify-match",NULL)) {
      do_fftMatch_verification = FALSE;
    }
    else if (strmatches(key,"-pixel-size","--pixel-size","-ps",NULL)) {
      CHECK_ARG(1);
      pixel_size = atof(GET_ARG(1));
    }
    else if (strmatches(key,"-dem-grid-size","--dem-grid-size",NULL)) {
      CHECK_ARG(1);
      dem_grid_size = atof(GET_ARG(1));
    }
    else {
      printf( "\n**Invalid option:  %s\n", argv[currArg-1]);
      usage(argv[0]);
    }
  }
  if ((argc-currArg) < NUM_ARGS) {
    printf("Insufficient arguments.\n");
    usage(argv[0]);
  }

  inFile = argv[currArg];
  demFile = argv[currArg+1];
  outFile = argv[currArg+2];

  asfPrintStatus("Reading metadata...\n");
  metaSAR = meta_read(inFile);
  metaDEM = meta_read(demFile);

  demRes = metaDEM->general->x_pixel_size;
  sarRes = metaSAR->general->x_pixel_size;
    
  asfPrintStatus("SAR Image is %dx%d, %gm pixels.\n",
		 metaSAR->general->line_count, metaSAR->general->sample_count,
		 sarRes);
  asfPrintStatus("DEM Image is %dx%d, %gm pixels.\n",
		 metaDEM->general->line_count, metaDEM->general->sample_count,
		 demRes);

  // Downsample the SAR image closer to the reference DEM if needed.
  // Otherwise, the quality of the resulting terrain corrected SAR image 
  // suffers. We put in a threshold of 1.5 times the resolution of the SAR
  // image. The -no-resample option overwrites this default behavior.
  if (do_resample && (demRes > 1.5 * sarRes || pixel_size > 0)) {
    if (pixel_size < 0)
    {
      asfPrintStatus(
	"DEM resolution is significantly higher than SAR resolution.\n");
      pixel_size = demRes;
    }

    asfPrintStatus("Resampling SAR image to pixel size of %g meters.\n",
		   pixel_size);

    resampleFile = appendSuffix(inFile, "_resample");
    resample_to_square_pixsiz(inFile, resampleFile, pixel_size);
    meta_free(metaSAR);
    metaSAR = meta_read(resampleFile);

    asfPrintStatus("After resmapling, SAR Image is %dx%d, %gm pixels.\n",
		   metaSAR->general->line_count,
		   metaSAR->general->sample_count,
		   metaSAR->general->x_pixel_size);
  } else {
    resampleFile = strdup(inFile);
  }

  // Calculate the slant range pixel size to pass into the ground range to
  // slant range conversion. This ensures that we have square pixels in the
  // output and don't need to scale the image afterwards.
  if (metaSAR->sar->image_type != 'S') {
    srFile = appendSuffix(inFile, "_slant");
    double sr_pixel_size = 
      (meta_get_slant(metaSAR,0,metaSAR->general->sample_count) -
       meta_get_slant(metaSAR,0,0)) / metaSAR->general->sample_count;
    asfPrintStatus("Converting to Slant Range...\n");

    gr2sr_pixsiz(resampleFile, srFile, sr_pixel_size);
    //asfSystem("gr2sr -p %.8f %s %s\n", sr_pixel_size, resampleFile, srFile);

    meta_free(metaSAR);
    metaSAR = meta_read(srFile);

    asfPrintStatus("In slant range, SAR Image is %dx%d, %gm pixels.\n",
		   metaSAR->general->line_count,
		   metaSAR->general->sample_count,
		   metaSAR->general->x_pixel_size);
  } else {
    srFile = strdup(resampleFile);
  }

  // Generate a point grid for the DEM extraction.
  // The width and height of the grid is defined in slant range image
  // coordinates, while the grid is actually calculated in DEM space.
  // There is a buffer of 400 pixels in far range added to have enough
  // DEM around when we get to correcting the terrain.
  asfPrintStatus("Generating %dx%d DEM grid...\n",
		 (int)dem_grid_size, (int)dem_grid_size);
  demGridFile = appendSuffix(inFile, "_demgrid");
  create_dem_grid_ext(demFile, srFile, demGridFile,
		      metaSAR->general->sample_count,
		      metaSAR->general->line_count, dem_grid_size);


  // Fit a fifth order polynomial to the grid points.
  // This polynomial is then used to extract a subset out of the reference 
  // DEM.
  asfPrintStatus("Fitting order %d polynomial to DEM...\n", polyOrder);
  double maxErr;
  poly_2d *fwX, *fwY, *bwX, *bwY;
  fit_poly(demGridFile, polyOrder, &maxErr, &fwX, &fwY, &bwX, &bwY);
  asfPrintStatus("Maximum error in polynomial fit: %g.\n", maxErr);

  int delete_this_old_crap = 0;
  char *demPolyFileOld;
  if (delete_this_old_crap) {
    demPolyFileOld = appendSuffix(inFile, "_dempoly_old");
    asfSystem("fit_poly %s %d %s", demGridFile, polyOrder, demPolyFileOld);
  }

  // Here is the actual work done for cutting out the DEM.
  // The adjustment of the DEM width by 400 pixels (originated in
  // create_dem_grid) needs to be factored in.

  demClipped = appendSuffix(demFile, "_clip");
  demWidth = metaSAR->general->sample_count + 400;
  demHeight = metaSAR->general->line_count;

  if (delete_this_old_crap) {
    char *demClippedOld = appendSuffix(demFile, "_clip_old");
    asfSystem("remap -translate 0 0 -poly %s -width %d -height %d "
	      "-bilinear -float %s %s", demPolyFileOld, demWidth, demHeight,
	      demFile, demClippedOld);
  }

  asfPrintStatus("Clipping DEM to %dx%d LxS using polynomial fit...\n",
		 demHeight, demWidth);
  remap_poly(fwX, fwY, bwX, bwY, demWidth, demHeight, demFile, demClipped);
  poly_delete(fwX);
  poly_delete(fwY);
  poly_delete(bwX);
  poly_delete(bwY);

  // Generate a slant range DEM and a simulated amplitude image.
  asfPrintStatus("Generating slant range DEM and simulated sar image...\n");
  demSlant = appendSuffix(demFile, "_slant");
  demSimAmp = appendSuffix(demFile, "_sim_amp");
  reskew_dem(srFile, demClipped, demSlant, demSimAmp);

  // Resize the simulated amplitude to match the slant range SAR image.
  asfPrintStatus("Resizing simulated sar image...\n");
  demTrimSimAmp = appendSuffix(demFile, "_sim_amp_trim");
  trim(demSimAmp, demTrimSimAmp, 0, 0, metaSAR->general->sample_count,
       demHeight);

  // Match the real and simulated SAR image to determine the offset.
  // Read the offset out of the offset file.
  asfPrintStatus("Determining image offsets...\n");

  corrFile = appendSuffix(inFile, "_corr");
  fftMatchQ(srFile, demTrimSimAmp, NULL, corrFile);

  read_corr(corrFile, &dx, &dy);
  asfPrintStatus("Correlation: dx=%g dy=%g\n", dx, dy);
  idx = - int_rnd(dx);
  idy = - int_rnd(dy);

  // Apply the offset to the simulated amplitude image.
  asfPrintStatus("Applying offsets to simulated sar image...\n");
  trim(demSimAmp, demTrimSimAmp, idx, idy, metaSAR->general->sample_count,
       demHeight);

  // Verify that the applied offset in fact does the trick.
  if (do_fftMatch_verification) {
    double dx2, dy2;

    asfPrintStatus("Verifying offsets are now close to zero...\n");
    corrFile2 = appendSuffix(inFile, "_corr2");
    fftMatchQ(srFile, demTrimSimAmp, NULL, corrFile2);

    read_corr(corrFile2, &dx2, &dy2);
    asfPrintStatus("Correlation after shift: dx=%g dy=%g\n", dx2, dy2);

    double match_tolerance = 1.0;
    if (sqrt(dx2*dx2 + dy2*dy2) > match_tolerance) {
      asfPrintError("Correlated images failed to match!\n"
		    " Original fftMatch offset: (dx,dy) = %14.9lf,%14.9lf\n"
		    "   After shift, offset is: (dx,dy) = %14.9lf,%14.9lf\n",
		    dx, dy, dx2, dy2);
    }
  }

  // Apply the offset to the slant range DEM.
  asfPrintStatus("Applying offsets to slant range DEM...\n");
  demTrimSlant = appendSuffix(demFile, "_slant_trim");
  trim(demSlant, demTrimSlant, idx, idy, metaSAR->general->sample_count,
       demHeight);

  // Terrain correct the slant range image while bringing it back to
  // ground range geometry. This is done without radiometric correction
  // of the values.
  ensure_ext(&demTrimSlant, "img");
  ensure_ext(&srFile, "img");
  asfPrintStatus("Terrain correcting slant range image...\n");
  deskew_dem(demTrimSlant, outFile, srFile, 0);

  if (clean_files) {
    asfPrintStatus("Removing intermediate files...\n");
    clean(resampleFile);
    clean(srFile);
    clean(demClipped);
    //clean(demPolyFile);
    clean(demGridFile);
    clean(demTrimSlant);
    clean(demTrimSimAmp);
    clean(corrFile2);
    clean(corrFile);
    clean(demSimAmp);
    clean(demSlant);
  }

  asfPrintStatus("Terrain Correction Complete!\n");

  free(resampleFile);
  free(srFile);
  free(demClipped);
  //free(demPolyFile);
  free(demGridFile);
  free(demTrimSlant);
  free(demTrimSimAmp);
  free(corrFile2);
  free(corrFile);
  free(demSimAmp);
  free(demSlant);

  meta_free(metaSAR);
  meta_free(metaDEM);

  return EXIT_SUCCESS;
}
