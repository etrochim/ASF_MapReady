#include "asf.h"
#include "asf_vector.h"
#include <ctype.h>

static int strindex(char s[], char t[])
{
  int i, j, k;

  for (i=0; s[i]!='\0'; i++) {
    for (j=i, k=0; t[k]!='\0' && s[j]==t[k]; j++, k++)
      ;
    if (k>0 && t[k]=='\0')
      return i;
  }
  return -1;
}

/*
static void apply_default_dirs(c2v_config *cfg)
{
  // apply default dir to input name if there isn't a dir there already
  char file_dir[4096], file[4096];
  split_dir_and_file(cfg->general->in_name, file_dir, file);
  int file_len = strlen(file);
  int file_dir_len = strlen(file_dir);
  int def_dir_len = strlen(cfg->general->default_in_dir);
  if (file_dir_len==0 && def_dir_len!=0 && file_len>0) {
    char *tmpstr = (char*)MALLOC(sizeof(char)*(file_len+def_dir_len+2));
    strcpy(tmpstr, cfg->general->default_in_dir);
    strcat(tmpstr, DIR_SEPARATOR_STR);
    strcat(tmpstr, file);
    cfg->general->in_name = (char*)realloc(cfg->general->in_name,
                                           sizeof(char)*(strlen(tmpstr)+2));
    strcpy(cfg->general->in_name, tmpstr);
    FREE(tmpstr);
  }
  // Sneak a directory separator on to the end of the default in dir
  if (strlen(cfg->general->default_in_dir) &&
      strncmp(cfg->general->default_in_dir, MAGIC_UNSET_STRING, strlen(MAGIC_UNSET_STRING)) != 0 &&
      DIR_SEPARATOR != cfg->general->default_in_dir[def_dir_len-1]) {
    strcat(cfg->general->default_in_dir, DIR_SEPARATOR_STR);
  }

  // apply default dir to output name if there isn't a dir there already
  split_dir_and_file(cfg->general->out_name, file_dir, file);
  file_len = strlen(file);
  file_dir_len = strlen(file_dir);
  def_dir_len = strlen(cfg->general->default_out_dir);
  if (file_dir_len==0 && def_dir_len!=0 && file_len>0) {
    char *tmpstr = (char*)MALLOC(sizeof(char)*(file_len+def_dir_len+2));
    strcpy(tmpstr, cfg->general->default_out_dir);
    strcat(tmpstr, DIR_SEPARATOR_STR);
    strcat(tmpstr, file);
    cfg->general->out_name = (char*)realloc(cfg->general->out_name,
                                            sizeof(char)*(strlen(tmpstr)+2));
    strcpy(cfg->general->out_name, tmpstr);
    FREE(tmpstr);
  }
  // Sneak a directory separator on to the end of the default out dir
  if (strlen(cfg->general->default_out_dir) &&
      strncmp(cfg->general->default_out_dir, MAGIC_UNSET_STRING, strlen(MAGIC_UNSET_STRING)) != 0 &&
      DIR_SEPARATOR != cfg->general->default_out_dir[def_dir_len-1]) {
    strcat(cfg->general->default_out_dir, DIR_SEPARATOR_STR);
  }
}
*/

static char *read_param(char *line)
{
  int i, k;
  char *value=(char *)CALLOC(256, sizeof(char));

  strcpy(value, "");
  i=strindex(line, "]");
  k=strindex(line, "=");
  if (i>0) strncpy(value, line, i+1);
  if (k>0) strncpy(value, line, k);
  return value;
}

static char *read_str(char *line, char *param)
{
  static char value[255];
  char *p = strchr(line, '=');

  // skip past the '=' sign, and eat up any whitespace
  ++p;
  while (isspace(*p))
      ++p;

  strcpy(value, p);

  // eat up trailing whitespace, too
  p = value + strlen(value) - 1;
  while (isspace(*p))
      *p-- = '\0';

  return value;
}

static int read_int(char *line, char *param)
{
  char *tmp;
  int value;

  tmp = read_str(line, param);
  sscanf(tmp, "%i", &value);

  return value;
}

static double read_double(char *line, char *param)
{
  char *tmp;
  double value;

  tmp = read_str(line, param);
  sscanf(tmp, "%lf", &value);

  return value;
}

int init_c2v_config(char *configFile)
{
  FILE *fConfig;

  if ( fileExists(configFile) ) {
    asfPrintError("Cannot create file, %s, because it already exists.\n",
                  configFile);
  }
  fConfig = FOPEN(configFile, "w");

  fprintf(fConfig, "convert2vector configuration file\n\n");

  fprintf(fConfig, "[General]\n\n");
  // directory
  fprintf(fConfig, "# This parameter defines the default directory where the\n");
  fprintf(fConfig, "# files are located.\n\n");
  fprintf(fConfig, "directory = \n\n");
  // input file
  fprintf(fConfig, "# This parameter defines the name of the input file or\n");
  fprintf(fConfig, "# the name of the list file.\n\n");
  fprintf(fConfig, "input file = \n\n");
  // output file
  fprintf(fConfig, "# This parameter defines the name of the output file.\n\n");
  fprintf(fConfig, "output file = \n\n");
  // input format
  fprintf(fConfig, "# This parameter defines the input format for the vector conversion.\n");
  fprintf(fConfig, "# The available formats are: meta, leader, shape, point, polygon, geotiff, terrasar, csv, and stf.\n\n");
  fprintf(fConfig, "input format = \n\n");
  // output format
  fprintf(fConfig, "# This parameter defines the output format for the vector conversion.\n");
  fprintf(fConfig, "# The available formats are: shape, kml, and text.\n\n");
  fprintf(fConfig, "output format = \n\n");
  // list
  fprintf(fConfig, "# The list flag indicates whether the input is a file (value set to 0)\n");
  fprintf(fConfig, "# or a list of files (value set to 1). The default value is 0\n\n");
  fprintf(fConfig, "list = 0\n\n");
  // short configuration file flag
  fprintf(fConfig, "# The short configuration file flag allows the experienced user to generate\n"
          "# configuration files without the verbose comments that explain all entries for\n"
          "# the parameters in the configuration file (1 for a configuration without comments,\n"
          "# 0 for a configuration file with verbose comments)\n\n");
  fprintf(fConfig, "short configuration file = 0\n\n");

  fprintf(fConfig, "[KML]\n\n");
  // time
  fprintf(fConfig, "# This parameter specifies that the list of input files is\n"
	  "# treated as a time series. Each file in the list will have a\n"
	  "# time stamp associated with it.\n\n");
  fprintf(fConfig, "time = \n\n");
  // boundary
  fprintf(fConfig, "# This parameter defines the type of boundary should be used in KML.\n");
  fprintf(fConfig, "# The recognized boundary types are 'polygon' and 'line'.\n\n");
  fprintf(fConfig, "boundary = \n\n");
  // height
  fprintf(fConfig, "# This parameter defines the height references for KML.\n");
  fprintf(fConfig, "# The recognized height references are 'clampToGround' and 'relativeToGround'.\n\n");
  fprintf(fConfig, "height = \n\n");
  // width
  fprintf(fConfig, "# This parameter defines the width of the boundary for KML.\n\n");
  fprintf(fConfig, "width = 1\n\n");
  // color
  fprintf(fConfig, "# This parameter defines the color of the boundary. It is expressed as aabbggrr,\n");
  fprintf(fConfig, "# where aa=alpha (00 to ff); bb=blue (00 to ff); gg=green (00 to ff); rr=red (00 to ff).\n\n");
  fprintf(fConfig, "color = \n\n");

  FCLOSE(fConfig);

  asfPrintStatus("   Initialized configuration file\n\n");

  return(0);
}

c2v_config *init_fill_c2v_config()
{
#define newStruct(type) (type *)MALLOC(sizeof(type))
#define LINE_LEN 8196

  FILE *fConfig, *fDefaults;
  char line[LINE_LEN], params[25];
  char *test;
  int i;

  // Create structure
  strcpy(params, "");
  c2v_config *cfg = newStruct(c2v_config);

  // Initialize structure
  strcpy(cfg->comment, "convert2vector configuration file");
  strcpy(cfg->directory, "./");
  strcpy(cfg->input_file, "");
  strcpy(cfg->output_file, "");
  strcpy(cfg->input_format, "META");
  strcpy(cfg->output_format, "KML");
  cfg->list = 0;
  cfg->time = 0;
  strcpy(cfg->boundary, "polygon");
  strcpy(cfg->height, "clampToGround");
  cfg->width = 5;
  strcpy(cfg->color, "ffff9900");
  cfg->short_config = 0;  

  return cfg;
}

c2v_config *read_c2v_config(char *configFile)
{
  FILE *fConfig;
  c2v_config *cfg=NULL;
  char line[255], params[50], tmp[50];
  char *test;

  strcpy(params, "");
  cfg = init_fill_c2v_config(configFile);
  if (cfg == NULL) check_return(1, "Creating configuration structure.\n");
  fConfig = fopen(configFile, "r");
  if (!fConfig) return NULL;
  while (fgets(line, 255, fConfig) != NULL) {
    
    if (strncmp(line, "[General]", 9)==0) strcpy(params, "general");
    if (strcmp(params, "general")==0) {
      test = read_param(line);
      if (strncmp(test, "directory", 9)==0)
	strcpy(cfg->directory, read_str(line, "directory"));
      if (strncmp(test, "input file", 10)==0)
	strcpy(cfg->input_file, read_str(line, "input file"));
      if (strncmp(test, "output file", 11)==0)
	strcpy(cfg->output_file, read_str(line, "output file"));
      if (strncmp(test, "input format", 12)==0)
	strcpy(cfg->input_format, read_str(line, "input format"));
      if (strncmp(test, "output format", 13)==0)
	strcpy(cfg->output_format, read_str(line, "output format"));
      if (strncmp(test, "list", 4)==0) 
	cfg->list = read_int(line, "list");
      FREE(test);
    }
    if (strncmp(line, "[KML]", 5)==0) strcpy(params, "kml");
    if (strcmp(params, "kml")==0) {
      test = read_param(line);
      if (strncmp(test, "time", 4)==0)
	cfg->time = read_int(line, "time");
      if (strncmp(test, "boundary", 8)==0)
	strcpy(cfg->boundary, read_str(line, "boundary"));
      if (strncmp(test, "height", 6)==0)
	strcpy(cfg->height, read_str(line, "height"));
      if (strncmp(test, "width", 5)==0)
	cfg->width = read_int(line, "width");
      if (strncmp(test, "color", 5)==0)
	strcpy(cfg->color, read_str(line, "color"));
      FREE(test);
    } 
  }

  FCLOSE(fConfig);

  return cfg;
}

int write_c2v_config(char *configFile, c2v_config *cfg)
{
  FILE *fConfig;
  int shortFlag=FALSE;

  if (cfg == NULL)
    check_return(1, "No configuration structure to write.\n");
  if (cfg->short_config)
    shortFlag = TRUE;

  fConfig = FOPEN(configFile, "w");
  fprintf(fConfig, "%s\n", cfg->comment);


  fprintf(fConfig, "convert2vector configuration file\n\n");

  fprintf(fConfig, "[General]\n");
  // directory
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the default directory where the\n"
	    "# files are located.\n\n");
  fprintf(fConfig, "directory = %s\n", cfg->directory);
  // input file
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the name of the input file or\n"
	    "# the name of the list file.\n\n");
  fprintf(fConfig, "input file = \n");
  // output file
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the name of the output file.\n\n");
  fprintf(fConfig, "output file = \n");
  // input format
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the input format for the vector conversion.\n"
	    "# The available formats are: meta, leader, shape, point, polygon, geotiff, terrasar, csv, and stf.\n\n");
  fprintf(fConfig, "input format = %s\n", cfg->input_format);
  // output format
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the output format for the vector conversion.\n"
	    "# The available formats are: shape, kml, and text.\n\n");
  fprintf(fConfig, "output format = %s\n", cfg->output_format);
  // list
  if (!shortFlag)
    fprintf(fConfig, "\n# The list flag indicates whether the input is a file (value set to 0)\n"
	    "# or a list of files (value set to 1). The default value is 0\n\n");
  fprintf(fConfig, "list = %d\n", cfg->list);
  // short configuration file flag
  if (!shortFlag)
    fprintf(fConfig, "\n# The short configuration file flag allows the experienced user to generate\n"
	    "# configuration files without the verbose comments that explain all entries for\n"
	    "# the parameters in the configuration file (1 for a configuration without comments,\n"
	    "# 0 for a configuration file with verbose comments)\n\n");
  fprintf(fConfig, "short configuration file = %d\n", cfg->short_config);

  fprintf(fConfig, "\n[KML]\n");
  // time
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter specifies that the list of input files is\n"
	    "# treated as a time series. Each file in the list will have a\n"
	    "# time stamp associated with it.\n");
  fprintf(fConfig, "time = %d\n", cfg->time);
  // boundary
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the type of boundary should be used in KML.\n"
	    "# The recognized boundary types are 'polygon' and 'line'.\n\n");
  fprintf("boundary = %s\n", cfg->boundary);
  // height
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the height references for KML.\n"
	    "# The recognized height references are 'clampToGround' and 'relativeToGround'.\n\n");
  fprintf(fConfig, "height = %s\n", cfg->height);
  // width
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the width of the boundary for KML.\n\n");
  fprintf(fConfig, "width = %d\n", cfg->width);
  // color
  if (!shortFlag)
    fprintf(fConfig, "\n# This parameter defines the color of the boundary. It is expressed as aabbggrr,\n"
	    "# where aa=alpha (00 to ff); bb=blue (00 to ff); gg=green (00 to ff); rr=red (00 to ff).\n\n");
  fprintf(fConfig, "color = %s\n", cfg->color);

  FCLOSE(fConfig);

  return(0);
}