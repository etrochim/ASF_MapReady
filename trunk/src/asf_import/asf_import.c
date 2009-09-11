#include <asf_contact.h>
#include <asf_license.h>
/*==================BEGIN ASF DOCUMENTATION==================*/
/*
ABOUT EDITING THIS DOCUMENTATION:
If you wish to edit the documentation for this program, you need to change the
following defines.
*/

#define ASF_NAME_STRING \
"asf_import"

#define ASF_USAGE_STRING \
"   "ASF_NAME_STRING" [-amplitude | -sigma | -gamma | -beta | -power] [-db]\n"\
"              [-format <inputFormat>] [-ancillary-file <file>]\n"\
"              [-colormap <colormap_file>] [-band <band_id | all>]\n"\
"              [-no-ers2-gain-fix] [-image-data-type <type>] [-lut <file>]\n"\
"              [-lat <lower> <upper>] [-prc] [-log <logFile>]\n"\
"              [-quiet] [-real-quiet] [-license] [-version] [-multilook]\n"\
"              [-azimuth-scale[=<scale>] | -fix-meta-ypix[=<pixsiz>]]\n"\
"              [-range-scale[=<scale>] [-complex] [-metadata <file>]\n"\
"              [-line <start line subset>] [-sample <start sample subset>]\n"\
"              [-width <subset width>] [-height <subset height>]\n"\
"              [-help]\n"\
"              <inBaseName> <outBaseName>\n"

#define ASF_DESCRIPTION_STRING \
"   Ingests all varieties of CEOS, STF, AIRSAR, BIL, GRIDFLOAT, VP\n"\
"   (Vexcel-Plain), JAXA Level 0 (ALOS AVNIR-2), PolSARpro, and GeoTIFF data\n"\
"   formats and outputs ASF Internal format metadata and data files. When\n"\
"   the calibration parameters are applied using the -sigma, -gamma, or the\n"\
"   -beta option the resulting image will have power scale values (or\n"\
"   decibels if the -db option is utilized). "ASF_NAME_STRING" can also\n"\
"   perform several other tasks during look up such as multilooking when\n"\
"   ingesting single-look complex (SLC) data, ingest individual bands at a\n"\
"   time, apply a look-up table, apply latitude constraints, etcetera.\n"\

#define ASF_INPUT_STRING \
"   The format of the input file must be specified as STF, AIRSAR, BIL,\n"\
"   GRIDFLOAT, VP, JAXA_L0, POLSARPRO, or GEOTIFF.  Otherwise,\n"\
"   "ASF_NAME_STRING" assumes the input file is in CEOS format by default.\n"\
"   See the -format option below.\n"

#define ASF_OUTPUT_STRING \
"   Outputs data and metadata files with the user-provided base name and\n"\
"   appropriate extensions (.img and .meta)\n"

#define ASF_OPTIONS_STRING \
"   -amplitude\n"\
"        Create an amplitude image. This is the default behavior.\n"\
"   -sigma\n"\
"        Create a calibrated image (sigma power scale values).\n"\
"   -gamma\n"\
"        Create a calibrated image (gamma power scale values).\n"\
"   -beta\n"\
"        Create a calibrated image (beta power scale values).\n"\
"   -power\n"\
"        Create a power image.\n"\
"   -complex\n"\
"        Create a complex image (I+Q) (only for single look complex data).\n"\
"   -multilook\n"\
"        Create a multilooked image when ingesting single look complex data\n"\
"        into a two-banded image with amplitude and phase\n"\
"   -db  Output calibrated image in decibels. This can only be used with\n"\
"        -sigma, -beta, or -gamma. When performing statistics on the imagery\n"\
"        it is highly recommended that the image is left in power scale\n"\
"        (ie do not use the -db flag if you plan on statistical analysis)\n"\
"   -format <inputFormat>\n"\
"        Force input data to be read as the given format type. Valid formats\n"\
"        are 'ceos', 'stf', 'geotiff', 'airsar', 'bil', 'gridfloat', 'vp',\n"\
"        'polsarpro', 'gamma', 'alos_mosaic' and 'jaxa_L0'. The 'jaxa_L0'\n"\
"        format refers to the ALOS AVNIR-2 Level 0 dataset format. 'CEOS' is\n"\
"        the default behavior.\n"\
"   -ancillary-file <file>\n"\
"        For PolSARpro format files, the ingest process needs access to the\n"\
"        original COES or AIRSAR format data that the PolSARpro images were\n"\
"        created from.  The original dataset is necessary for the purpose of\n"\
"        extracting original SAR parameters that are not otherwise available\n"\
"        in the PolSARpro format files as they are.\n"\
"   - colormap <colormap_file>\n"\
"        Associates a color map (RGB index) with the input file.  Does not\n"\
"        apply to multi-band images or images containing floating point data\n"\
"        (other than PolSARpro .bin/.bin.hdr images.)  The colormap files\n"\
"        must exist in the application installation 'share' directory's look\n"\
"        up table directory.  You may provide your own colormap by placing\n"\
"        it in this folder.  File format must either be in ASF format (.lut)\n"\
"        or in JASC-PAL (.pal) format.  See existing look-up tables for\n"\
"        examples.  Note that the use of the colormap option will override\n"\
"        any embedded RGB index type colormap in the file that is being\n"\
"        imported, e.g. a TIFF or GeoTIFF with an embedded color palette.\n"\
"   -metadata <metadata file>\n"\
"        Allows the ingest of metadata that does not have the same basename\n"\
"        as the image data.\n"\
"   -band <band_id | all>\n"\
"        If the data contains multiple data files, one for each band (channel)\n"\
"        then import the band identified by 'band_id' (only).  If 'all' is\n"\
"        specified rather than a band_id, then import all available bands into\n"\
"        a single ASF-format file.  Default is '-band all'.\n"\
"   -image-data-type <type>\n"\
"        Force input data to be interpreted as the given image data type. Valid\n"\
"        formats are 'amplitude_image', 'phase_image', 'coherence_image',\n"\
"        'lut_image', 'elevation', 'dem', and 'image', 'mask'.  NOTE: This\n"\
"        option only applies to GeoTIFFs (-format option is \"geotiff\").\n"\
"        If the input format is not \"geotiff\", then this option is ignored.\n"\
"   -lut <file>\n"\
"        Applies a user defined look up table to the data. Look up contains\n"\
"        incidence angle dependent scaling factor.\n\n"\
"        asf_import may apply a look up table automatically.  Automatically\n"\
"        applied look up tables are in the asf share directory, in the\n"\
"        look_up_table/import subdirectory.  asf_import will check to see\n"\
"        if the first line of any file in that directory has the format:\n"\
"           # ASF Import YYYY/MM/DD <sensor>\n"\
"        and, if so, if the imported file's sensor matches that given in the\n"\
"        file, and the acquisition date is on or after what is given in the\n"\
"        file, the look up table will be automatically applied.\n"\
"   -lat <lower> <upper>\n"\
"        Specify lower and upper latitude contraints (only available\n"\
"        for STF). Note that the program is not able to verify whether\n"\
"        the chosen latitude constraint is within the image.\n"\
"   -prc\n"\
"        Replace the restituted state vectors from the original raw data\n"\
"        acquired by the ERS satellites with precision vectors\n"\
"   -no-ers2-gain-fix\n"\
"        Do not apply the ERS2 gain correction.  (See the 'Notes' section\n"\
"        below.)\n"\
"   -save-intermediates\n"\
"        Save any intermediate files which may have been created during the\n"\
"        import process.  At this time, this only applies to the ingest of \n"\
"        'jaxa_L0' format since this type of dataset is first imported into\n"\
"        a standard JPEG format then the JPEG is converted to ASF Internal\n"\
"        format.\n"\
"   -log <logFile>\n"\
"        Output will be written to a specified log file.\n"\
"   -quiet\n"\
"        Supresses all non-essential output.\n"\
"   -real-quiet\n"\
"        Supresses all output.\n"\
"   -license\n"\
"        Print copyright and license for this software then exit.\n"\
"   -version\n"\
"        Print version and copyright then exit.\n"\
"   -help\n"\
"        Print a help page and exit.\n\n"\
"The following options allow correction of scaling errors in the original\n"\
"data.\n\n"\
"   -range-scale[=<scale-factor>]\n"\
"        Apply the provided range scale factor to the imported data.  If\n"\
"        the option is specified without an argument, a default value of\n"\
"        %f will be used.\n\n"\
"        The metadata will not be updated after scaling - this option is\n"\
"        intended to be used to correct errors in the data.\n\n"\
"   -azimuth-scale[=<scale-factor>]\n"\
"        Apply the provided azimuth scale factor to the imported data.  If\n"\
"        the option is specified without an argument, a default value will\n"\
"        be calculated from the metadata.\n\n"\
"        This option cannot be used with -fix-meta-ypix\n\n"\
"        The metadata will not be updated after scaling - this option is\n"\
"        intended to be used to correct errors in the data.\n\n"\
"   -fix-meta-ypix[=<pixel-size>]\n"\
"        This option is similar to -azimuth-scale, but does not resample the\n"\
"        input data, it just changes the y pixel size in the metadata.\n"\
"        This option cannot be used with -azimuth-scale.\n"\

#define ASF_EXAMPLES_STRING \
"   To import CEOS format to the ASF tools internal format run:\n"\
"        example> asf_import fileCEOS fileASF\n"\
"\n"\
"   To import a STF fileset (fileSTF.000 & file.000.par) you will need to\n"\
"   specify the -format option since STF is not the default.\n"\
"        example> asf_import -format stf fileSTF.000 fileASF\n" \
"\n"\
"   To import an ALOS Palsar fileset (IMG-HH-file, IMG-HV-file, IMG-VH-file,\n"\
"   IMG-VV-file, and LED-file) you may specify the input basename 'file',\n"\
"   or the full name of the leader (LED-file) file.\n"\
"   When importing optical or other multi-band CEOS-formatted data such as ALOS\n"\
"   Avnir optical images, you can specify the file names in the same way.\n"\
"        example> asf_import file outfile\n"\
"\n"\
"   To import a single band of an ALOS fileset (IMG-HH-file, IMG-HV-file,\n"\
"   IMG-VH-file,IMG-VV-file, and LED-file; or IMG-01-file, IMG-02-file, etc) you\n"\
"   will need to specify the band_id and input basename 'file'.\n"\
"        example1> asf_import -band VH file outfile\n"\
"        example2> asf_import -band 04 file outfile\n"

#define ASF_NOTES_STRING \
"   The ERS2 satellite has a known gain loss problem that this program\n"\
"   will attempt to correct by applying a scale correction factor uniformly\n"\
"   to all pixels in the image.  The correction is dependent on the date,\n"\
"   and is only applied to calibrated data (i.e., everything but amplitude)\n"\
"   For more information, see section 4 of:\n"\
"   <http://www.asf.alaska.edu/reference/dq/Envisat_symp_ers2_performance.pdf>\n"

#define ASF_LIMITATIONS_STRING \
"   CEOS base name issue:\n"\
"        If you have two or more CEOS filesets ([*.D & *.L], [*.RAW & *.LDR],\n"\
"        or [dat.* & lea.*]) with the same base name, then this program will\n"\
"        automatically fetch the first set in the aforementioned list and ignore\n"\
"        the rest.  The workaround is to move files with similar basenames, but\n"\
"        different extensions, to different work folders before processing.\n"

#define ASF_SEE_ALSO_STRING \
"   asf_mapready, asf_export\n"

/* Option -old removed from the help and description above ...but still supported.
// Just not advertised
//"   -old\n"\
//"        Output in old style ASF internal format.\n"\
*/

/*===================END ASF DOCUMENTATION===================*/

#include "asf_import.h"
#include "asf_meta.h"
#include "asf_nan.h"
#include "ceos.h"
#include "get_ceos_names.h"
#include "get_stf_names.h"
#include "asf_raster.h"
#include <ctype.h>

#ifdef linux
char *strdup(char *);
#endif

#define REQUIRED_ARGS 2

#define FLAG_SET 1
#define FLAG_NOT_SET -1

/* Index keys for all flags used in this program via a 'flags' array */
typedef enum {
    f_AMP=1,
    f_SIGMA,
    f_BETA,
    f_GAMMA,
    f_POWER,
    f_COMPLEX,
    f_MULTILOOK,
    f_AMP0,
    f_DB,
    f_SPROCKET,
    f_LUT,
    f_LAT_CONSTRAINT,
    f_PRC,
    f_NO_ERS2_GAIN_FIX,
    f_FORMAT,
    f_ANCILLARY_FILE,
    f_OLD_META,
    f_METADATA_FILE,
    f_LOG,
    f_QUIET,
    f_REAL_QUIET,
    f_RANGE_SCALE,
    f_AZIMUTH_SCALE,
    f_FIX_META_YPIX,
    f_DATA_TYPE,
    f_IMAGE_DATA_TYPE,
    f_BAND,
    f_LINE,
    f_SAMPLE,
    f_WIDTH,
    f_HEIGHT,
    f_SAVE_INTERMEDIATES,
    f_COLORMAP,
    NUM_IMPORT_FLAGS
} import_flag_indices_t;

/* Helpful functions */

/* Check to see if an option was supplied or not
   If it was found, return its argument number
   Otherwise, return FLAG_NOT_SET */
static int checkForOption(char* key, int argc, char* argv[])
{
  int ii = 0;
  while(ii < argc)
  {
    if(strmatch(key, argv[ii]))
      return(ii);
    ++ii;
  }
  return(FLAG_NOT_SET);
}

/* Check to see if an option with (or without) an argument
   was supplied.  If it was found, return its argument
   number.  otherwise return FLAG_NOT_SET.

   The argument is assumed to be of the form "<key>[=value]"
   The [=value] part is optional. */
static int checkForOptionWithArg(char *key, int argc, char *argv[])
{
  int ii = 0;
  while (ii < argc)
  {
    /* make a copy of the arg, and remove anything past the "=" */
    char *arg = STRDUP(argv[ii]);
    char *eq = strchr(arg, '=');
    if (eq) *eq = '\0';

    /* now check for an exact match */
    int match = !strcmp(key, arg);
    FREE(arg);
    if (match) {
      return ii;
    }
    ++ii;
  }
  return FLAG_NOT_SET;
}

static double getDoubleOptionArgWithDefault(char *arg, double def)
{
  double val = def;
  char *arg_cpy = strdup(arg);
  char *eq = strchr(arg_cpy, '=');
  if (eq) {
    ++eq;
    char *endptr;
    double d = strtod(eq, &endptr);
    if (endptr != eq) val = d;
  }
  free(arg_cpy);

  return val;
}

static void
pixel_type_flag_looker(int *flag_count, char *flags_used, char *flagName)
{
  if (*flag_count==0)
    strcat(flags_used, flagName);
  else if (*flag_count==1)
    strcat(strcat(flags_used, " and "), flagName);
  else if (*flag_count>1)
    strcat(strcat(flags_used, ", and "), flagName);
  else
    asfPrintError("Programmer error dealing with the %s flag.\n", flagName);
  (*flag_count)++;
}

// Print minimalistic usage info & exit
static void print_usage(void)
{
  asfPrintStatus("\n"
      "Usage:\n"
      ASF_USAGE_STRING
      "\n");
  exit(EXIT_FAILURE);
}

// Print the help info & exit
static void print_help(void)
{
  asfPrintStatus(
      "\n"
      "Tool name:\n   " ASF_NAME_STRING "\n\n"
      "Usage:\n" ASF_USAGE_STRING "\n"
      "Description:\n" ASF_DESCRIPTION_STRING "\n"
      "Input:\n" ASF_INPUT_STRING "\n"
      "Output:\n"ASF_OUTPUT_STRING "\n"
      "Options:\n" ASF_OPTIONS_STRING "\n"
      "Examples:\n" ASF_EXAMPLES_STRING "\n"
      "Notes:\n" ASF_NOTES_STRING "\n"
      "Limitations:\n" ASF_LIMITATIONS_STRING "\n"
      "See also:\n" ASF_SEE_ALSO_STRING "\n"
      "Contact:\n" ASF_CONTACT_STRING "\n"
      "Version:\n   " SVN_REV " (part of " TOOL_SUITE_NAME " " MAPREADY_VERSION_STRING ")\n\n",
      DEFAULT_RANGE_SCALE);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    char inBaseName[256]="";
    char ancillary_file[256]="";
    char outBaseName[256]="";
    char *inMetaNameOption=NULL;
    char *lutName=NULL;
    char *colormapName=NULL;
    char *prcPath=NULL;
    char format_type_str[256]="";
    input_format_t format_type;
    char band_id[256]="";
    char data_type[256]="";
    char image_data_type[256]="";
    int ii;
    int flags[NUM_IMPORT_FLAGS];
    double lowerLat=-99.0, upperLat=-99.0;
    int line=0, sample=0, height=-99, width=-99;
    double range_scale=NAN, azimuth_scale=NAN, correct_y_pixel_size=NAN;
    int do_resample;
    int do_metadata_fix;

    /* Set all flags to 'not set' */
    for (ii=0; ii<NUM_IMPORT_FLAGS; ii++) {
        flags[ii] = FLAG_NOT_SET;
    }

    /**********************BEGIN COMMAND LINE PARSING STUFF**********************/
    if (   (checkForOption("--help", argc, argv) != FLAG_NOT_SET)
        || (checkForOption("-h", argc, argv) != FLAG_NOT_SET)
        || (checkForOption("-help", argc, argv) != FLAG_NOT_SET) ) {
        print_help();
    }
    handle_license_and_version_args(argc, argv, ASF_NAME_STRING);

    /*Check to see if any options were provided*/
    flags[f_AMP] = checkForOption("-amplitude", argc, argv);
    flags[f_SIGMA] = checkForOption("-sigma", argc, argv);
    flags[f_BETA] = checkForOption("-beta", argc, argv);
    flags[f_GAMMA] = checkForOption("-gamma", argc, argv);
    flags[f_POWER] = checkForOption("-power", argc, argv);
    flags[f_COMPLEX] = checkForOption("-complex", argc, argv);
    flags[f_MULTILOOK] = checkForOption("-multilook", argc, argv);
    flags[f_AMP0] = checkForOption("-amp0", argc, argv);
    flags[f_DB] = checkForOption("-db", argc, argv);
    flags[f_SPROCKET] = checkForOption("-sprocket", argc, argv);
    flags[f_LUT] = checkForOption("-lut", argc, argv);
    flags[f_LAT_CONSTRAINT] = checkForOption("-lat", argc, argv);
    flags[f_PRC] = checkForOption("-prc", argc, argv);
    flags[f_NO_ERS2_GAIN_FIX] = checkForOption("-no-ers2-gain-fix", argc, argv);
    flags[f_OLD_META] = checkForOption("-old", argc, argv);
    flags[f_METADATA_FILE] = checkForOption("-metadata", argc, argv);
    flags[f_LOG] = checkForOption("-log", argc, argv);
    flags[f_QUIET] = checkForOption("-quiet", argc, argv);
    flags[f_REAL_QUIET] = checkForOption("-real-quiet", argc, argv);
    flags[f_FORMAT] = checkForOption("-format", argc, argv);
    flags[f_DATA_TYPE] = checkForOption("-data-type", argc, argv);
    flags[f_IMAGE_DATA_TYPE] = checkForOption("-image-data-type", argc, argv);
    flags[f_BAND] = checkForOption("-band", argc, argv);
    flags[f_LINE] = checkForOption("-line", argc, argv);
    flags[f_SAMPLE] = checkForOption("-sample", argc, argv);
    flags[f_WIDTH] = checkForOption("-width", argc, argv);
    flags[f_HEIGHT] = checkForOption("-height", argc, argv);
    flags[f_SAVE_INTERMEDIATES] = checkForOption("-save-intermediates", argc, argv);
    flags[f_COLORMAP] = checkForOption("-colormap", argc, argv);

    flags[f_ANCILLARY_FILE] = checkForOption("-ancillary-file", argc, argv);
    if (flags[f_ANCILLARY_FILE] == FLAG_NOT_SET)
        flags[f_ANCILLARY_FILE] = checkForOption("-ancillary_file", argc, argv);

    flags[f_RANGE_SCALE] = checkForOptionWithArg("-range-scale", argc, argv);
    if (flags[f_RANGE_SCALE] == FLAG_NOT_SET)
        flags[f_RANGE_SCALE] = checkForOptionWithArg("-range_scale", argc, argv);

    flags[f_AZIMUTH_SCALE] = checkForOptionWithArg("-azimuth-scale", argc, argv);
    if (flags[f_AZIMUTH_SCALE] == FLAG_NOT_SET)
        flags[f_AZIMUTH_SCALE] = checkForOptionWithArg("-azimuth_scale", argc, argv);

    flags[f_FIX_META_YPIX] = checkForOptionWithArg("-fix-meta-ypix", argc, argv);
    if (flags[f_FIX_META_YPIX] == FLAG_NOT_SET)
        flags[f_FIX_META_YPIX] = checkForOptionWithArg("-fix_meta_ypix", argc, argv);

    do_resample = flags[f_RANGE_SCALE] != FLAG_NOT_SET ||
        flags[f_AZIMUTH_SCALE] != FLAG_NOT_SET;

    if (flags[f_SPROCKET] != FLAG_NOT_SET)
        asfPrintError("Sprocket layers are not yet supported.\n");

    if (do_resample)
    {
        range_scale = flags[f_RANGE_SCALE] == FLAG_NOT_SET ? 1.0 :
      getDoubleOptionArgWithDefault(argv[flags[f_RANGE_SCALE]], -1);

        azimuth_scale = flags[f_AZIMUTH_SCALE] == FLAG_NOT_SET ? 1.0 :
      getDoubleOptionArgWithDefault(argv[flags[f_AZIMUTH_SCALE]], -1);
    }

    do_metadata_fix = flags[f_FIX_META_YPIX] != FLAG_NOT_SET;

    if (do_metadata_fix)
    {
        correct_y_pixel_size =
      getDoubleOptionArgWithDefault(argv[flags[f_FIX_META_YPIX]], -1);
    }

    { /*Check for mutually exclusive options: we can only have one of these*/
        int temp = 0;
        if(flags[f_AMP] != FLAG_NOT_SET)      temp++;
        if(flags[f_SIGMA] != FLAG_NOT_SET)    temp++;
        if(flags[f_BETA] != FLAG_NOT_SET)     temp++;
        if(flags[f_GAMMA] != FLAG_NOT_SET)    temp++;
        if(flags[f_POWER] != FLAG_NOT_SET)    temp++;
        if(flags[f_SPROCKET] != FLAG_NOT_SET) temp++;
        if(flags[f_LUT] != FLAG_NOT_SET)      temp++;
        if(temp > 1)/*If more than one option was selected*/

            print_usage();
    }

    /* Cannot specify the fix-meta-ypix & the resampling of azimuth
    options at the same time */
    if (flags[f_FIX_META_YPIX] != FLAG_NOT_SET &&
        flags[f_AZIMUTH_SCALE] != FLAG_NOT_SET)
    {
        asfPrintStatus("You cannot specify both -azimuth-scale "
            "and -fix-meta-ypix.\n");
        print_usage();
    }

    { /*We need to make sure the user specified the proper number of arguments*/
        int needed_args = 1 + REQUIRED_ARGS;    /*command + REQUIRED_ARGS*/
        if(flags[f_AMP] != FLAG_NOT_SET)      needed_args += 1;/*option*/
        if(flags[f_SIGMA] != FLAG_NOT_SET)    needed_args += 1;/*option*/
        if(flags[f_BETA] != FLAG_NOT_SET)     needed_args += 1;/*option*/
        if(flags[f_GAMMA] != FLAG_NOT_SET)    needed_args += 1;/*option*/
        if(flags[f_POWER] != FLAG_NOT_SET)    needed_args += 1;/*option*/
        if(flags[f_COMPLEX] != FLAG_NOT_SET)  needed_args += 1;/*option*/
        if(flags[f_MULTILOOK] != FLAG_NOT_SET) needed_args += 1;/*option*/
        if(flags[f_AMP0] != FLAG_NOT_SET)     needed_args += 1;/*option*/
        if(flags[f_DB] != FLAG_NOT_SET)       needed_args += 1;/*option*/
        if(flags[f_SPROCKET] != FLAG_NOT_SET) needed_args += 1;/*option*/
        if(flags[f_LUT] != FLAG_NOT_SET)      needed_args += 2;/*option & parameter*/
        if(flags[f_LAT_CONSTRAINT] != FLAG_NOT_SET)
            needed_args += 3;/*option & parameter & parameter*/
        if(flags[f_PRC] != FLAG_NOT_SET)      needed_args += 2;/*option & parameter*/
        if(flags[f_NO_ERS2_GAIN_FIX] != FLAG_NOT_SET) needed_args += 1;/*option*/
        if(flags[f_OLD_META] != FLAG_NOT_SET) needed_args += 1;/*option*/
        if(flags[f_METADATA_FILE] != FLAG_NOT_SET)  needed_args += 2;/*option & parameter*/
        if(flags[f_LOG] != FLAG_NOT_SET)      needed_args += 2;/*option & parameter*/
        if(flags[f_QUIET] != FLAG_NOT_SET)    needed_args += 1;/*option*/
        if(flags[f_REAL_QUIET] != FLAG_NOT_SET) needed_args += 1;/*option*/
        if(flags[f_FORMAT] != FLAG_NOT_SET)   needed_args += 2;/*option & parameter*/
        if(flags[f_ANCILLARY_FILE] != FLAG_NOT_SET) needed_args += 2;/*option & parameter*/
        if(flags[f_BAND] != FLAG_NOT_SET)     needed_args += 2;/*option & parameter*/
        if(flags[f_LINE] != FLAG_NOT_SET)     needed_args += 2;/*option & parameter*/
        if(flags[f_SAMPLE] != FLAG_NOT_SET)   needed_args += 2;/*option & parameter*/
        if(flags[f_WIDTH] != FLAG_NOT_SET)    needed_args += 2;/*option & parameter*/
        if(flags[f_HEIGHT] != FLAG_NOT_SET)   needed_args += 2;/*option & parameter*/
        if(flags[f_SAVE_INTERMEDIATES] != FLAG_NOT_SET)  needed_args += 1;/*option*/
        if(flags[f_DATA_TYPE] != FLAG_NOT_SET)  needed_args += 2; /*option & parameter*/
        if(flags[f_IMAGE_DATA_TYPE] != FLAG_NOT_SET)  needed_args += 2; /*option & parameter*/
        if(flags[f_RANGE_SCALE] != FLAG_NOT_SET)   needed_args += 1;/*option*/
        if(flags[f_AZIMUTH_SCALE] != FLAG_NOT_SET)   needed_args += 1;/*option*/
        if(flags[f_FIX_META_YPIX] != FLAG_NOT_SET)   needed_args += 1;/*option*/
        if(flags[f_COLORMAP] != FLAG_NOT_SET)   needed_args += 2; /*option & parameter*/

        /*Make sure we have enough arguments*/
        if(argc != needed_args)
            print_usage();/*This exits with a failure*/
    }

    /*We also need to make sure any options that have parameters are specified
    correctly.  This includes: -lat, -prc, -log, -lut etc */
    if(flags[f_LAT_CONSTRAINT] != FLAG_NOT_SET)
        /*Make sure there's no "bleeding" into the required arguments
        No check for '-' in the two following fields because negative numbers
        are legit (eg -lat -67.5 -70.25)*/
        if(flags[f_LAT_CONSTRAINT] >= argc - (REQUIRED_ARGS+1))
            print_usage();
    if(flags[f_LINE] != FLAG_NOT_SET)
        if(   argv[flags[f_LINE]+1][0] == '-'
            || flags[f_LINE] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_SAMPLE] != FLAG_NOT_SET)
        if(   argv[flags[f_SAMPLE]+1][0] == '-'
            || flags[f_SAMPLE] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_HEIGHT] != FLAG_NOT_SET)
        if(   argv[flags[f_HEIGHT]+1][0] == '-'
            || flags[f_HEIGHT] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_WIDTH] != FLAG_NOT_SET)
        if(   argv[flags[f_WIDTH]+1][0] == '-'
            || flags[f_WIDTH] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_PRC] != FLAG_NOT_SET)
        /*Make sure the field following -prc isn't another option
        Also check for bleeding into required arguments*/
        if(   argv[flags[f_PRC]+1][0] == '-'
            || flags[f_PRC] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_METADATA_FILE] != FLAG_NOT_SET)
        /*Make sure the field following -metadata isn't another option*/
        if(   argv[flags[f_METADATA_FILE] + 1][0] == '-'
            || flags[f_METADATA_FILE] >= argc - REQUIRED_ARGS)
            print_usage();
    if(flags[f_LOG] != FLAG_NOT_SET)
        /*Make sure the field following -log isn't another option*/
        if(   argv[flags[f_LOG]+1][0] == '-'
            || flags[f_LOG] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_LUT] != FLAG_NOT_SET)
        /*Make sure the field following -lut isn't another option*/
        if(   argv[flags[f_LUT]+1][0] == '-'
            || flags[f_LUT] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_FORMAT] != FLAG_NOT_SET)
        /*Make sure the field following -format isn't another option*/
        if(   argv[flags[f_FORMAT]+1][0] == '-'
            || flags[f_FORMAT] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_ANCILLARY_FILE] != FLAG_NOT_SET)
      /*Make sure the field following -format isn't another option*/
      if(   argv[flags[f_ANCILLARY_FILE]+1][0] == '-'
            || flags[f_ANCILLARY_FILE] >= argc-REQUIRED_ARGS)
        print_usage();
    if(flags[f_BAND] != FLAG_NOT_SET)
      /*Make sure the field following -format isn't another option*/
      if(   argv[flags[f_BAND]+1][0] == '-'
            || flags[f_BAND] >= argc-REQUIRED_ARGS)
        print_usage();
    if(flags[f_IMAGE_DATA_TYPE] != FLAG_NOT_SET)
        /*Make sure the field following -format isn't another option*/
        if(   argv[flags[f_IMAGE_DATA_TYPE]+1][0] == '-'
            || flags[f_IMAGE_DATA_TYPE] >= argc-REQUIRED_ARGS)
            print_usage();
    if(flags[f_COLORMAP] != FLAG_NOT_SET)
      /*Make sure the field following -colormap isn't another option*/
      if(   argv[flags[f_COLORMAP]+1][0] == '-'
            || flags[f_COLORMAP] >= argc-REQUIRED_ARGS)
        print_usage();

    /* Be sure to open log ASAP */
    if(flags[f_LOG] != FLAG_NOT_SET)
        strcpy(logFile, argv[flags[f_LOG] + 1]);
    else /*default behavior: log to tmp<pid>.log*/
        sprintf(logFile, "tmp%i.log", (int)getpid());
    logflag = TRUE; /* Since we always log, set the old school logflag to true */

    // Open log file in output folder
    char path[1024], tmp[1024];
    split_dir_and_file(argv[argc-1], path, tmp);
    strcpy(tmp, logFile);
    sprintf(logFile, "%s%s", path, tmp);
    fLog = fopen(logFile, "a");
    if ( fLog == NULL ) {
      logflag = FALSE;
    }

    /* Set old school quiet flag (for use in our libraries) */
    quietflag = flags[f_QUIET] != FLAG_NOT_SET;
    if (flags[f_REAL_QUIET] != FLAG_NOT_SET) quietflag = 2;

    /*We must be close to good enough at this point... log & quiet flags are set
    Report what was retrieved at the command line */
    asfSplashScreen(argc, argv);

    if(flags[f_PRC] != FLAG_NOT_SET) {
        prcPath = (char *)MALLOC(sizeof(char)*256);
        strcpy(prcPath, argv[flags[f_PRC] + 1]);
    }

    if(flags[f_LAT_CONSTRAINT] != FLAG_NOT_SET) {
        lowerLat = strtod(argv[flags[f_LAT_CONSTRAINT] + 2],NULL);
        upperLat = strtod(argv[flags[f_LAT_CONSTRAINT] + 1],NULL);
        if(lowerLat > upperLat) {
            float swap = upperLat;
            upperLat = lowerLat;
            lowerLat = swap;
        }
        if(lowerLat < -90.0 || lowerLat > 90.0 || upperLat < -90.0 || upperLat > 90.0)
        {
            asfPrintError("Invalid latitude constraint (must be -90 to 90)");
        }
    }

    if(flags[f_LINE] != FLAG_NOT_SET)
      line = atoi(argv[flags[f_LINE]+1]);
    if(flags[f_SAMPLE] != FLAG_NOT_SET)
      sample = atoi(argv[flags[f_SAMPLE]+1]);
    if(flags[f_WIDTH] != FLAG_NOT_SET)
      width = atoi(argv[flags[f_WIDTH]+1]);
    if(flags[f_HEIGHT] != FLAG_NOT_SET)
      height = atoi(argv[flags[f_HEIGHT]+1]);

    if(flags[f_LUT] != FLAG_NOT_SET) {
        lutName = (char *) MALLOC(sizeof(char)*256);
        strcpy(lutName, argv[flags[f_LUT] + 1]);
    }

    if(flags[f_COLORMAP] != FLAG_NOT_SET) {
      colormapName = (char *) MALLOC(sizeof(char)*256);
      strcpy(colormapName, argv[flags[f_COLORMAP] + 1]);
    }

    { /* BEGIN: Check for conflict between pixel type flags */
        char flags_used[256] = "";
        int flag_count=0;

        if (flags[f_AMP] != FLAG_NOT_SET) {
            pixel_type_flag_looker(&flag_count, flags_used, "amplitude");
        }
        if (flags[f_SIGMA] != FLAG_NOT_SET) {
            pixel_type_flag_looker(&flag_count, flags_used, "sigma");
        }
        if (flags[f_GAMMA] != FLAG_NOT_SET) {
            pixel_type_flag_looker(&flag_count, flags_used, "gamma");
        }
        if (flags[f_BETA] != FLAG_NOT_SET) {
            pixel_type_flag_looker(&flag_count, flags_used, "beta");
        }
        if (flags[f_POWER] != FLAG_NOT_SET) {
            pixel_type_flag_looker(&flag_count, flags_used, "power");
        }
        if (flags[f_LUT] != FLAG_NOT_SET) {
            pixel_type_flag_looker(&flag_count, flags_used, "lut");
        }
        if (flag_count > 1) {
            sprintf(logbuf, "Cannot mix the %s flags.", flags_used);
            asfPrintError(logbuf);
        }
    } /* END: Check for conflict between pixel type flags */

    if (   flags[f_DB] != FLAG_NOT_SET
        && !(   flags[f_SIGMA] != FLAG_NOT_SET
        || flags[f_GAMMA] != FLAG_NOT_SET
        || flags[f_BETA] != FLAG_NOT_SET ) )
    {
        asfPrintWarning("-db flag must be specified with -sigma, -gamma, or -beta. Ignoring -db.\n");
    }

    /* Get the input metadata name if the flag was specified (probably for a meta
    * name with a different base name than the data name) */
    if(flags[f_METADATA_FILE] != FLAG_NOT_SET) {
        inMetaNameOption=(char *)MALLOC(sizeof(char)*256);
        strcpy(inMetaNameOption, argv[flags[f_METADATA_FILE] + 1]);
    }

    /* Deal with input format type */
    if(flags[f_FORMAT] != FLAG_NOT_SET) {
        strcpy(format_type_str, argv[flags[f_FORMAT] + 1]);
    if (strncmp_case(format_type_str, "STF", 3) == 0)
      format_type = STF;
    else if (strncmp_case(format_type_str, "CEOS", 4) == 0)
      format_type = CEOS;
    else if (strncmp_case(format_type_str, "GEOTIFF", 7) == 0)
      format_type = GENERIC_GEOTIFF;
    else if (strncmp_case(format_type_str, "BIL", 3) == 0)
      format_type = BIL;
    else if (strncmp_case(format_type_str, "GRIDFLOAT", 9) == 0)
      format_type = GRIDFLOAT;
    else if (strncmp_case(format_type_str, "AIRSAR", 6) == 0)
      format_type = AIRSAR;
    else if (strncmp_case(format_type_str, "VP", 2) == 0)
      format_type = VP;
    else if (strncmp_case(format_type_str, "JAXA_L0", 7) == 0)
      format_type = JAXA_L0;
    else if (strncmp_case(format_type_str, "ALOS_MOSAIC", 11) == 0)
      format_type = ALOS_MOSAIC;
    else if (strncmp_case(format_type_str, "TERRASAR", 8) == 0)
      format_type = TERRASAR;
    else if (strncmp_case(format_type_str, "RADARSAT2", 9) == 0)
      format_type = RADARSAT2;
    else if (strncmp_case(format_type_str, "POLSARPRO", 9) == 0)
      format_type = POLSARPRO;
    else if (strncmp_case(format_type_str, "GAMMA", 5) == 0)
      format_type = GAMMA;
    else
      asfPrintError("Unsupported format: %s\n", format_type_str);
    }
    else
      format_type = CEOS;

    
    // Will need to check the ancillary file input at a lower level.
    // If the PolSARpro header includes map projection information,
    // we don't need to know the SAR geometry.

    /*
    // Process PolSARpro options (-format and -ancillary_flag combos)
    if (format_type == POLSARPRO &&
        flags[f_ANCILLARY_FILE] == FLAG_NOT_SET) {
      // PolSARpro requires the ancillary file
      asfPrintError("PolSARpro ingest requires the original CEOS or AIRSAR format\n"
          "dataset that was used to create the PolSARpro data files.  Please\n"
          "specify the original CEOS or AIRSAR format dataset with the -ancillary_file\n"
          "option.  See asf_import -help for more information.\n");
    }
    */
    
    // Process Gamma options (-format, -metadata, and -ancillary_flag combos)
    if (format_type == GAMMA &&
        flags[f_ANCILLARY_FILE] == FLAG_NOT_SET) {
      // GAMMA requires the ancillary file
      asfPrintError("GAMMA ingest requires the original CEOS format\n"
          "dataset that was used to create the GAMMA data files.  Please\n"
          "specify the original CEOS format dataset with the -ancillary_file\n"
          "option.  See asf_import -help for more information.\n");
    }
    if (format_type == GAMMA &&
        flags[f_METADATA_FILE] == FLAG_NOT_SET) {
      // GAMMA requires the metadata file
      asfPrintError("GAMMA ingest requires the specific name of the GAMMA\n"
	  "metadata file. Please specify the original CEOS format dataset \n"
	  "with the -metadata option.  See asf_import -help for more information.\n");
    }
    if (format_type == GAMMA &&
	flags[f_IMAGE_DATA_TYPE] == FLAG_NOT_SET) {
      // GAMMA requires the image data type
      asfPrintError("GAMMA ingest requires the image data type to figure out\n"
		    "what data type to apply during the ingest. Coherence\n"
		    "images are regular floating point data sets, while\n"
		    "interferograms are stored in complex form.\n");
    }

    if(flags[f_ANCILLARY_FILE] != FLAG_NOT_SET) {
      strcpy(ancillary_file, argv[flags[f_ANCILLARY_FILE] + 1]);
    }


    /* Deal with band_id */
    if(flags[f_BAND] != FLAG_NOT_SET) {
      strcpy(band_id, argv[flags[f_BAND] + 1]);
      if (strlen(band_id) && strcmp("ALL", uc(band_id)) == 0) {
        strcpy(band_id, "");
      }
    }

    // Deal with input data type
    if (flags[f_DATA_TYPE] != FLAG_NOT_SET) {
        asfPrintWarning("Use of the -data-type option is currently disabled.\n"
                       "The data type (pixel) will be interpreted as described in\n"
                       "the data set metadata instead.\n");
      //strcpy(data_type, argv[flags[f_DATA_TYPE] + 1]);
      //for (ii=0; ii<strlen(data_type); ii++) {
//        data_type[ii] = (char)toupper(data_type[ii]);
      //}
    }

    /* Deal with input image data type */
    if(flags[f_IMAGE_DATA_TYPE] != FLAG_NOT_SET &&
       (format_type == GENERIC_GEOTIFF || format_type == GAMMA || 
	format_type == POLSARPRO))
    {
      strcpy(image_data_type, argv[flags[f_IMAGE_DATA_TYPE] + 1]);
      for (ii=0; ii<strlen(image_data_type); ii++) {
        image_data_type[ii] = (char)toupper(image_data_type[ii]);
      }
    }
    else
    {
      if (flags[f_IMAGE_DATA_TYPE] != FLAG_NOT_SET &&
          format_type != GENERIC_GEOTIFF)
      {
        asfPrintWarning("The -image-data-type option is only valid for GeoTIFFs.\n"
                "If the input format is not \"geotiff\", then the -image-data-type\n"
                "option is ignored.  If your dataset is a geotiff, please use\n"
                "the \"-format geotiff\" option on the command line.\n");
      }
      strcpy(image_data_type, "???");
    }
    /* Make sure STF specific options are not used with other data types */
    if (format_type == STF) {
        if (flags[f_PRC] != FLAG_NOT_SET) {
            asfPrintWarning("Precision state vectors only work with STF data\n"
                "and will not be used with this data set!\n");
            flags[f_PRC]=FLAG_NOT_SET;
        }
        if (flags[f_LAT_CONSTRAINT] != FLAG_NOT_SET) {
            asfPrintWarning("No latitude constraints only work with STF data\n"
                "and will not be used with this data set!\n");
            flags[f_LAT_CONSTRAINT]=FLAG_NOT_SET;
        }
    }

    /* Fetch required arguments */
    strcpy(inBaseName, argv[argc - 2]);
    strcpy(outBaseName,argv[argc - 1]);

    /***********************END COMMAND LINE PARSING STUFF***********************/

    { // scoping block
        int db_flag = flags[f_DB] != FLAG_NOT_SET;
        int complex_flag = flags[f_COMPLEX] != FLAG_NOT_SET;
        int multilook_flag = flags[f_MULTILOOK] != FLAG_NOT_SET;
        int amp0_flag = flags[f_AMP0] != FLAG_NOT_SET;
        int apply_ers2_gain_fix = flags[f_NO_ERS2_GAIN_FIX] == FLAG_NOT_SET;
        int save_intermediates = flags[f_SAVE_INTERMEDIATES] != FLAG_NOT_SET;

        double *p_correct_y_pixel_size = NULL;
        if (do_metadata_fix)
            p_correct_y_pixel_size = &correct_y_pixel_size;

        double *p_range_scale = NULL;
        if (flags[f_RANGE_SCALE] != FLAG_NOT_SET)
            p_range_scale = &range_scale;

        double *p_azimuth_scale = NULL;
        if (flags[f_AZIMUTH_SCALE] != FLAG_NOT_SET)
            p_azimuth_scale = &azimuth_scale;

        radiometry_t radiometry = r_AMP;
        if(flags[f_AMP] != FLAG_NOT_SET)      radiometry = r_AMP;
        if(flags[f_SIGMA] != FLAG_NOT_SET)    radiometry = r_SIGMA;
        if(flags[f_BETA] != FLAG_NOT_SET)     radiometry = r_BETA;
        if(flags[f_GAMMA] != FLAG_NOT_SET)    radiometry = r_GAMMA;
        if(flags[f_POWER] != FLAG_NOT_SET)    radiometry = r_POWER;

        asf_import(radiometry, db_flag, complex_flag, multilook_flag,
                   amp0_flag, format_type, band_id, data_type, image_data_type,
                   lutName,prcPath, lowerLat, upperLat, line, sample,
                   width, height, save_intermediates, p_range_scale, p_azimuth_scale,
                   p_correct_y_pixel_size, apply_ers2_gain_fix, inMetaNameOption, inBaseName,
                   ancillary_file, colormapName, outBaseName);
    }

    if (colormapName)
        free(colormapName);
    if (lutName)
        free(lutName);
    if (prcPath)
        free(prcPath);

    /* If the user didn't ask for a log file then we can nuke the one that
       we've been keeping since we've finished everything  */
    if (logflag) {
        fclose (fLog);
        remove(logFile);
    }

    exit(EXIT_SUCCESS);
}
