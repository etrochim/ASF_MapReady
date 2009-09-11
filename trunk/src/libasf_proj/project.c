#include "projects.h"
#include "libasf_proj.h"
#include "asf.h"
#include "asf_nan.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "proj_api.h"
#include "spheroids.h"

#define DEFAULT_AVERAGE_HEIGHT 0.0;

/* WGS84 is hard-coded on the lat/lon side for now! */
static const char *latlon_description = "+proj=latlong +datum=WGS84";

#ifndef linux
#ifndef win32
static double
round (double arg)
{
  return floor (arg + 0.5);
}
#endif /* #ifndef win32 */
#endif /* #ifndef linux */

#ifdef linux
/* Some missing prototypes */
int putenv(char *);
int setenv(const char *, const char *, int);
#endif

static void set_proj_lib_path(datum_type_t datum)
{
    static int set_path_already = FALSE;
    if (datum == NAD27_DATUM) {
        if (!set_path_already) {
            // point to the grid shift files
            char proj_putenv[255];
            sprintf(proj_putenv, "PROJ_LIB=%s/proj", get_asf_share_dir());
            putenv(proj_putenv);
            //sprintf(proj_putenv, "%s/proj", get_asf_share_dir());
            //setenv("PROJ_LIB", proj_putenv, TRUE);
            set_path_already = TRUE;
        }
    }
}

static const char * datum_str(datum_type_t datum)
{
    set_proj_lib_path(datum);

    switch (datum)
    {
    case NAD27_DATUM:    /* North American Datum 1927 (Clarke 1866) */
        return "NAD27";
    case NAD83_DATUM:    /* North American Datum 1983 (GRS 1980)    */
        return "NAD83";
    case HUGHES_DATUM:
        return "HUGHES";
    default:
    case WGS84_DATUM:    /* World Geodetic System 1984 (WGS84)      */
        return "WGS84";
    }
}

// Returns TRUE if we have grid shift files available for the given point,
// and returns FALSE if not.  If this returns FALSE for any point in a
// scene, the NAD27 datum shouldn't be used.
int test_nad27(double lat, double lon)
{
    projPJ ll_proj, utm_proj;
    ll_proj = pj_init_plus(latlon_description);

    char desc[255];
    int zone = utm_zone(lon);
    sprintf(desc, "+proj=utm +zone=%d +datum=NAD27", zone);
    utm_proj = pj_init_plus(desc);
/*
    double *px, *py, *pz;
    px = MALLOC(sizeof(double));
    py = MALLOC(sizeof(double));
    pz = MALLOC(sizeof(double));
*/
    double px[1], py[1], pz[1];
    py[0] = lat*D2R;
    px[0] = lon*D2R;
    pz[0] = 0;

    pj_transform (ll_proj, utm_proj, 1, 1, px, py, pz);

    int ret = TRUE;
    if (pj_errno == -38) // -38 indicates error with the grid shift files
    {
        ret = FALSE;
    }
    else if (pj_errno != 0) // some other error (pj errors are negative,
    {                       // system errors are positive)
        asfPrintError("libproj Error: %s\n", pj_strerrno(pj_errno));
    }

    pj_free(ll_proj);
    pj_free(utm_proj);

    return ret;
}

static double sHeight = DEFAULT_AVERAGE_HEIGHT;
void project_set_avg_height(double h)
{
    sHeight = h;
}

static int height_was_set()
{
    return !ISNAN(sHeight);
}

static double get_avg_height()
{
    if (height_was_set())
  return sHeight;
    else
  return DEFAULT_AVERAGE_HEIGHT;
}

static int project_worker_arr(const char * projection_description,
                              double *lat, double *lon, double *height,
                              double **projected_x, double **projected_y,
                              double **projected_z, long length)
{
  projPJ geographic_projection, output_projection;
  int i, ok = TRUE;

  // This section is a bit confusing.  The interfaces to the single
  // point functions allow the user to a pass ASF_PROJ_NO_HEIGHT value
  // if they don't care about height.  The array functions allow the
  // user to pass NULL for height.  Both interfaces end up calling
  // this routine, which has to detect what has happened.  The single
  // point functions require NULL to be passed to as the third return
  // value pointer, to ensure that the user isn't confused about what
  // will be returned.  The array functions require that NULL be
  // passed as the return array pointer.  We need to allocate some
  // space for z values for temporary use in these cases, which is
  // also a bit tricky.

  // Flag true iff we "don't care about heights" (or a willing to use
  // the set average height), which happens if the user passes
  // ASF_PROJ_NO_HEIGHT for *height or NULL for height.
  int dcah = 0;

  if ( height == NULL ) {
    assert (projected_z == NULL);
    dcah = 1;
  }
  else if ( *height == ASF_PROJ_NO_HEIGHT ) {
    assert (*projected_z == NULL);
    dcah = 1;
  }

  // Flag true iff argument projected_z was not NULL on function entry.
  int projected_z_not_null = 0;
  if ( projected_z != NULL ) {
    projected_z_not_null = 1;
  }

  if ( dcah ) {
    // User should either pre-allocate both, or want us to allocate
    // both -- to preallocate one but not the other is weird and
    // probably a mistake.
    if ( !(*projected_x) || !(*projected_y) ) {
      assert (!(*projected_x) && !(*projected_y));

      *projected_x = (double *) MALLOC (sizeof(double) * length);
      *projected_y = (double *) MALLOC (sizeof(double) * length);
    }

    if ( !projected_z_not_null ) {
      projected_z = (double **) MALLOC (sizeof (double *));
    }
    *projected_z = (double *) MALLOC (sizeof(double) * length);
  }

  else {
    if (!(*projected_x) || !(*projected_y) || !(*projected_z)) {
      /* user should either pre-allocate all, or want us to allocate
   all -- to preallocate some but not the all is weird and
   probably a mistake */
      assert(!(*projected_x) && !(*projected_y) && !(*projected_z));

      *projected_x = (double *) MALLOC (sizeof(double) * length);
      *projected_y = (double *) MALLOC (sizeof(double) * length);
      *projected_z = (double *) MALLOC (sizeof(double) * length);
    }
  }

  double *px = *projected_x;
  double *py = *projected_y;
  double *pz = *projected_z;

  assert(px);
  assert(py);
  assert(pz);

  for (i = 0; i < length; ++i)
  {
    px[i] = lon[i];
    py[i] = lat[i];

    // If user passed height values, use those, otherwise, use the
    // average height or the default average height.
    if ( !dcah ) {
      pz[i] = height[i];
    }
    else {
      if ( height_was_set () ) {
  pz[i] = get_avg_height ();
      }
      else {
  pz[i] = 0.0;
      }
    }
  }

  //printf("proj: +from %s +to %s\n",
  //       latlon_description, projection_description);

  geographic_projection = pj_init_plus (latlon_description);

  if (pj_errno != 0)
  {
      asfPrintError("libproj Error: %s\n", pj_strerrno(pj_errno));
      ok = FALSE;
  }

  if (ok)
  {
      assert (geographic_projection != NULL);

      output_projection = pj_init_plus (projection_description);

      if (pj_errno != 0)
      {
    asfPrintError("libproj Error: %s\n", pj_strerrno(pj_errno));
    ok = FALSE;
      }

      if (ok)
      {
    assert (output_projection != NULL);

    pj_transform (geographic_projection, output_projection, length, 1,
      px, py, pz);

    if (pj_errno != 0)
    {
        asfPrintWarning("libproj error: %s\n", pj_strerrno(pj_errno));
        ok = FALSE;
    }

    pj_free(output_projection);
      }

      pj_free(geographic_projection);
  }

  // Free memory temporarily allocated for height values that we don't
  // really care about.
  if ( dcah ) {
    FREE (*projected_z);
    if ( !projected_z_not_null ) {
      FREE (projected_z);
    }
  }

  return ok;
}

static int
project_worker_arr_inv(const char * projection_description,
                       double *x, double *y, double *z,
                       double **lat, double **lon, double **height,
                       long length)
{
  projPJ geographic_projection, output_projection;
  int i, ok = TRUE;

  // Same issue here as above.  Because both single and array
  // functions ultimately call this routine, and we allow the user to
  // specify that they don't care about heights by passing certain
  // arguments, things get complicated.  See above.

  // Flag true iff we "don't care about heights" (or a willing to use
  // the set average height), which happens if the user passes
  // ASF_PROJ_NO_HEIGHT for *z or NULL for z.
  int dcah = 0;

  if ( z == NULL ) {
    assert (height == NULL);
    dcah = 1;
  }
  else if ( *z == ASF_PROJ_NO_HEIGHT ) {
    assert (*height == NULL);
    dcah = 1;
  }

  // Flag true iff argument height was not NULL on function entry.
  int height_not_null = 0;
  if ( height != NULL ) {
    height_not_null = 1;
  }

  if ( dcah ) {
    // User should either pre-allocate both, or want us to allocate
    // both -- to preallocate one but not the other is weird and
    // probably a mistake.
    if ( !(*lat) || !(*lon) ) {
      assert (!(*lat) && !(*lon));

      *lat = (double *) MALLOC (sizeof(double) * length);
      *lon = (double *) MALLOC (sizeof(double) * length);
    }

    if ( !height_not_null ) {
      height = (double **) MALLOC (sizeof (double *));
    }
    *height = (double *) MALLOC (sizeof(double) * length);
  }

  else {
    if (!(*lat) || !(*lon) || !(*height)) {
      /* user should either pre-allocate all, or want us to allocate
   all -- to preallocate some but not the all is weird and
   probably a mistake */
      assert(!(*lat) && !(*lon) && !(*height));

      *lat = (double *) MALLOC (sizeof(double) * length);
      *lon = (double *) MALLOC (sizeof(double) * length);
      *height = (double *) MALLOC (sizeof(double) * length);
    }
  }

  double *plat = *lat;
  double *plon = *lon;
  double *pheight = *height;

  assert(plat);
  assert(plon);
  assert(pheight);

  for (i = 0; i < length; ++i)
  {
    plat[i] = y[i];
    plon[i] = x[i];

    // If user passed z values, use those, otherwise, use the
    // average height or the default average height.
    if ( !dcah ) {
      pheight[i] = z[i];
    }
    else {
      if ( height_was_set () ) {
  pheight[i] = get_avg_height ();
      }
      else {
  pheight[i] = 0.0;
      }
    }
  }

  //printf("proj: +from %s +to %s\n",
  //       projection_description, latlon_description);

  geographic_projection = pj_init_plus ( latlon_description );

  if (pj_errno != 0)
  {
      asfPrintError("libproj Error: %s\n", pj_strerrno(pj_errno));
      ok = FALSE;
  }

  if (ok)
  {
      assert (geographic_projection != NULL);

      output_projection = pj_init_plus (projection_description);

      if (pj_errno != 0)
      {
    asfPrintError("libproj Error: %s\n", pj_strerrno(pj_errno));
    ok = FALSE;
      }

      if (ok)
      {
    assert (output_projection != NULL);

    pj_transform (output_projection, geographic_projection, length, 1,
      plon, plat, pheight);

    if (pj_errno != 0)
    {
        asfPrintWarning("libproj error: %s\n", pj_strerrno(pj_errno));
        ok = FALSE;
    }

    pj_free(output_projection);
      }

      pj_free(geographic_projection);
  }

  // Free memory temporarily allocated for height values that we don't
  // really care about.
  if ( dcah ) {
    FREE (*height);
    if ( !height_not_null ) {
      FREE (height);
    }
  }

  return ok;
}

/****************************************************************************
 Universal Transverse Mercator (UTM)
****************************************************************************/
static double utm_nudge(double lon_0)
{
  double tiny_value;

  /* Nudge cases which are marginal in terms of which utm zone they
     fall in towards zero a bit.  The proj documentation tells us we
     should avoid the marginal cases. */
  tiny_value = 0.00001;    /* Random small number of degrees. */
  if ( fabs(round(lon_0 / 6.0) - lon_0 / 6) < tiny_value ) {
    if ( lon_0 > 0 ) {
      lon_0 -= tiny_value;
    }
    else {
      lon_0 += tiny_value;
    }
  }

  return lon_0;
}

static const char *
utm_projection_description(project_parameters_t * pps, datum_type_t datum)
{
  static char utm_projection_description[128];

  /* Establish description of output projection. */
  if (datum != HUGHES_DATUM) {
    if (pps->utm.zone == MAGIC_UNSET_INT)
    {
      sprintf(utm_projection_description,
        "+proj=utm +lon_0=%f +datum=%s",
        utm_nudge(pps->utm.lon0), datum_str(datum));
    } else {
      sprintf(utm_projection_description,
        "+proj=utm +zone=%d %s+datum=%s",
        pps->utm.zone, pps->utm.false_northing == 10000000 ? "+south " : "",
          datum_str(datum));
    }
  }
  else { // HUGHES
    if (pps->utm.zone == MAGIC_UNSET_INT) {
      sprintf(utm_projection_description,
              "+proj=utm +lon_0=%f +a=%f +rf=%f",
              utm_nudge(pps->utm.lon0 * RAD_TO_DEG),
              (float)HUGHES_SEMIMAJOR,
              (float)HUGHES_INV_FLATTENING);
    } else {
      sprintf(utm_projection_description,
              "+proj=utm +zone=%d %s+a=%f +rf=%f",
              pps->utm.zone, pps->utm.false_northing == 10000000 ? "+south " : "",
              (float)HUGHES_SEMIMAJOR,
              (float)HUGHES_INV_FLATTENING);
    }
  }

  return utm_projection_description;
}

int
project_utm (project_parameters_t * pps, double lat, double lon, double height,
       double *x, double *y, double *z, datum_type_t datum)
{
    return project_worker_arr(utm_projection_description(pps, datum),
                              &lat, &lon, &height, &x, &y, &z, 1);
}

int
project_utm_arr (project_parameters_t * pps,
     double *lat, double *lon, double *height,
     double **projected_x, double **projected_y,
     double **projected_z, long length,
         datum_type_t datum)
{
  return project_worker_arr(
      utm_projection_description(pps, datum),
      lat, lon, height, projected_x, projected_y, projected_z, length);
}

int
project_utm_inv (project_parameters_t * pps,
     double x, double y, double z,  double *lat, double *lon,
     double *height, datum_type_t datum)
{
  return project_worker_arr_inv(
      utm_projection_description(pps, datum),
      &x, &y, &z, &lat, &lon, &height, 1);
}

int
project_utm_arr_inv (project_parameters_t * pps,
         double *x, double *y, double *z,
         double **lat, double **lon, double **height,
         long length, datum_type_t datum)
{
  return project_worker_arr_inv(
      utm_projection_description(pps, datum),
      x, y, z, lat, lon, height, length);
}

/****************************************************************************
 Polar Sterographic (PS)
****************************************************************************/
static const char *
ps_projection_desc(project_parameters_t * pps, datum_type_t datum)
{
  static char ps_projection_description[128];

  /* Establish description of output projection. */
  if (datum != HUGHES_DATUM) {
    sprintf(ps_projection_description,
      "+proj=stere +lat_0=%s +lat_ts=%f +lon_0=%f "
        "+k_0=%f +datum=%s",
      pps->ps.is_north_pole ? "90" : "-90",
      pps->ps.slat,
      pps->ps.slon,
      1.0 /* pps->ps.scale_factor */,
      datum_str(datum));
  }
  else { // HUGHES
    sprintf(ps_projection_description,
            "+proj=stere +lat_0=%s +lat_ts=%f +lon_0=%f "
                "+k_0=%f +a=%f +rf=%f",
            pps->ps.is_north_pole ? "90" : "-90",
            pps->ps.slat,
            pps->ps.slon,
            1.0 /* pps->ps.scale_factor */,
            (float)HUGHES_SEMIMAJOR,
            (float)HUGHES_INV_FLATTENING);
  }

  return ps_projection_description;
}

int
project_ps(project_parameters_t * pps, double lat, double lon, double height,
           double *x, double *y, double *z, datum_type_t datum)
{
  return project_worker_arr(ps_projection_desc(pps, datum), &lat, &lon,
                &height, &x, &y, &z, 1);
}

int
project_ps_arr(project_parameters_t * pps,
         double *lat, double *lon, double *height,
         double **projected_x, double **projected_y,
         double **projected_z, long length, datum_type_t datum)
{
  return project_worker_arr(ps_projection_desc(pps, datum), lat, lon, height,
          projected_x, projected_y, projected_z, length);
}

int
project_ps_inv(project_parameters_t * pps, double x, double y, double z,
         double *lat, double *lon, double *height, datum_type_t datum)
{
    return project_worker_arr_inv(ps_projection_desc(pps, datum),
                  &x, &y, &z, &lat, &lon, &height, 1);
}

int
project_ps_arr_inv(project_parameters_t * pps,
       double *x, double *y, double *z,
       double **lat, double **lon, double **height,
       long length, datum_type_t datum)
{
    return project_worker_arr_inv(ps_projection_desc(pps, datum),
          x, y, z, lat, lon, height, length);
}

/****************************************************************************
 Lambert Azimuthal Equal Area
****************************************************************************/
static char * lamaz_projection_desc(project_parameters_t * pps,
                                    datum_type_t datum)
{
  static char lamaz_projection_description[128];

  /* Establish description of output projection. */
  if (datum != HUGHES_DATUM) {
    sprintf(lamaz_projection_description,
        "+proj=laea +lat_0=%f +lon_0=%f +datum=%s",
        pps->lamaz.center_lat,
        pps->lamaz.center_lon,
        datum_str(datum));
  }
  else {
    sprintf(lamaz_projection_description,
        "+proj=laea +lat_0=%f +lon_0=%f +a=%f +rf=%f",
        pps->lamaz.center_lat,
        pps->lamaz.center_lon,
            (float)HUGHES_SEMIMAJOR,
            (float)HUGHES_INV_FLATTENING);
  }

  return lamaz_projection_description;
}

int
project_lamaz(project_parameters_t * pps,
        double lat, double lon, double height,
        double *x, double *y, double *z, datum_type_t datum)
{
    return project_worker_arr(lamaz_projection_desc(pps, datum),
                  &lat, &lon, &height, &x, &y, &z, 1);
}

int
project_lamaz_arr(project_parameters_t *pps,
      double *lat, double *lon, double *height,
      double **projected_x, double **projected_y,
      double **projected_z, long length, datum_type_t datum)
{
    return project_worker_arr(lamaz_projection_desc(pps, datum), lat, lon,
                  height, projected_x, projected_y, projected_z, length);
}

int
project_lamaz_inv(project_parameters_t *pps, double x, double y, double z,
      double *lat, double *lon, double *height, datum_type_t datum)
{
  return project_worker_arr_inv(lamaz_projection_desc(pps, datum),
                &x, &y, &z, &lat, &lon, &height, 1);
}

int
project_lamaz_arr_inv(project_parameters_t *pps,
          double *x, double *y, double *z,
          double **lat, double **lon, double **height,
          long length, datum_type_t datum)
{
  return project_worker_arr_inv(lamaz_projection_desc(pps, datum), x, y, z,
        lat, lon, height, length);
}

/****************************************************************************
 Lambert Conformal Conic
****************************************************************************/
static char * lamcc_projection_desc(project_parameters_t * pps,
                                    datum_type_t datum)
{
  static char lamcc_projection_description[128];

  /* Establish description of output projection. */
  if (datum != HUGHES_DATUM) {
    sprintf(lamcc_projection_description,
        "+proj=lcc +lat_1=%f +lat_2=%f +lat_0=%f +lon_0=%f +datum=%s",
        pps->lamcc.plat1,
        pps->lamcc.plat2,
        pps->lamcc.lat0,
        pps->lamcc.lon0,
        datum_str(datum));
  }
  else {
    sprintf(lamcc_projection_description,
        "+proj=lcc +lat_1=%f +lat_2=%f +lat_0=%f +lon_0=%f +a=%f +rf=%f",
        pps->lamcc.plat1,
        pps->lamcc.plat2,
        pps->lamcc.lat0,
        pps->lamcc.lon0,
            (float)HUGHES_SEMIMAJOR,
            (float)HUGHES_INV_FLATTENING);
  }

  return lamcc_projection_description;
}

int
project_lamcc(project_parameters_t * pps,
        double lat, double lon, double height,
        double *x, double *y, double *z, datum_type_t datum)
{
    return project_worker_arr(lamcc_projection_desc(pps, datum),
                  &lat, &lon, &height, &x, &y, &z, 1);
}

int
project_lamcc_arr(project_parameters_t *pps,
      double *lat, double *lon, double *height,
      double **projected_x, double **projected_y,
      double **projected_z, long length, datum_type_t datum)
{
  return project_worker_arr(lamcc_projection_desc(pps, datum), lat, lon,
                height, projected_x, projected_y, projected_z, length);
}

int
project_lamcc_inv(project_parameters_t *pps, double x, double y, double z,
      double *lat, double *lon, double *height, datum_type_t datum)
{
  return project_worker_arr_inv(lamcc_projection_desc(pps, datum),
                &x, &y, &z, &lat, &lon, &height, 1);
}

int
project_lamcc_arr_inv(project_parameters_t *pps,
          double *x, double *y, double *z,
          double **lat, double **lon, double **height,
          long length, datum_type_t datum)
{
  return project_worker_arr_inv(lamcc_projection_desc(pps, datum),
                x, y, z, lat, lon, height, length);
}

// Mercator
static char * mer_projection_desc(project_parameters_t * pps,
                                     datum_type_t datum)
{
  static char mer_projection_description[128];

  /* Establish description of output projection. */
  if (datum != HUGHES_DATUM) {
    sprintf(mer_projection_description,
	    "+proj=merc +lat_ts=%f +lon_0=%f +x_0=%f +y_0=%f +datum=%s",
	    pps->mer.standard_parallel,
	    pps->mer.central_meridian,
	    pps->mer.false_easting,
	    pps->mer.false_northing,
	    datum_str(datum));
  }
  else {
    sprintf(mer_projection_description,
	    "+proj=merc +lat_ts=%f +lon_0=%f +x_0=%f +y_0=%f +a=%f +rf=%f",
	    pps->mer.standard_parallel,
	    pps->mer.central_meridian,
	    pps->mer.false_easting,
	    pps->mer.false_northing,
            (float)HUGHES_SEMIMAJOR,
            (float)HUGHES_INV_FLATTENING);
  }

  return mer_projection_description;
}

int
project_mer(project_parameters_t *pps,
	    double lat, double lon, double height,
	    double *x, double *y, double *z, datum_type_t datum)
{
    return project_worker_arr(mer_projection_desc(pps, datum),
            &lat, &lon, &height, &x, &y, &z, 1);
}

int
project_mer_arr(project_parameters_t *pps,
		double *lat, double *lon, double *height,
		double **projected_x, double **projected_y,
		double **projected_z, long length, datum_type_t datum)
{
  return project_worker_arr(mer_projection_desc(pps, datum),
                lat, lon, height, projected_x, projected_y, projected_z,
                length);
}

int
project_mer_inv(project_parameters_t *pps, double x, double y, double z,
		double *lat, double *lon, double *height, datum_type_t datum)
{
    return project_worker_arr_inv(mer_projection_desc(pps, datum),
                  &x, &y, &z, &lat, &lon, &height, 1);
}

int
project_mer_arr_inv(project_parameters_t *pps,
           double *x, double *y, double *z,
           double **lat, double **lon, double **height,
           long length, datum_type_t datum)
{
    return project_worker_arr_inv(mer_projection_desc(pps, datum),
                  x, y, z, lat, lon, height, length);
}

// Equirectangular
static char * eqr_projection_desc(project_parameters_t * pps,
				  datum_type_t datum)
{
  static char eqr_projection_description[128];

  /* Establish description of output projection. */
  if (datum != HUGHES_DATUM) {
    sprintf(eqr_projection_description,
	    "+proj=eqc +lat_ts=%f +lon_0=%f +x_0=%f +y_0=%f +datum=%s",
	    pps->eqr.orig_latitude,
	    pps->eqr.central_meridian,
	    pps->eqr.false_easting,
	    pps->eqr.false_northing,
	    datum_str(datum));
  }
  else {
    sprintf(eqr_projection_description,
	    "+proj=eqc +lat_ts=%f +lon_0=%f +x_0=%f +y_0=%f +a=%f +rf=%f",
	    pps->eqr.orig_latitude,
	    pps->eqr.central_meridian,
	    pps->eqr.false_easting,
	    pps->eqr.false_northing,
            (float)HUGHES_SEMIMAJOR,
            (float)HUGHES_INV_FLATTENING);
  }

  return eqr_projection_description;
}

int
project_eqr(project_parameters_t *pps,
	    double lat, double lon, double height,
	    double *x, double *y, double *z, datum_type_t datum)
{
    return project_worker_arr(eqr_projection_desc(pps, datum),
            &lat, &lon, &height, &x, &y, &z, 1);
}

int
project_eqr_arr(project_parameters_t *pps,
		double *lat, double *lon, double *height,
		double **projected_x, double **projected_y,
		double **projected_z, long length, datum_type_t datum)
{
  return project_worker_arr(eqr_projection_desc(pps, datum),
                lat, lon, height, projected_x, projected_y, projected_z,
                length);
}

int
project_eqr_inv(project_parameters_t *pps, double x, double y, double z,
		double *lat, double *lon, double *height, datum_type_t datum)
{
    return project_worker_arr_inv(eqr_projection_desc(pps, datum),
                  &x, &y, &z, &lat, &lon, &height, 1);
}

int
project_eqr_arr_inv(project_parameters_t *pps,
		    double *x, double *y, double *z,
		    double **lat, double **lon, double **height,
		    long length, datum_type_t datum)
{
    return project_worker_arr_inv(eqr_projection_desc(pps, datum),
                  x, y, z, lat, lon, height, length);
}

/****************************************************************************
  Albers Equal-Area Conic
****************************************************************************/
static char * albers_projection_desc(project_parameters_t * pps,
                                     datum_type_t datum)
{
  static char albers_projection_description[128];

  /* Establish description of output projection. */
  if (datum != HUGHES_DATUM) {
    sprintf(albers_projection_description,
        "+proj=aea +lat_1=%f +lat_2=%f +lat_0=%f +lon_0=%f +datum=%s",
        pps->albers.std_parallel1,
        pps->albers.std_parallel2,
        pps->albers.orig_latitude,
        pps->albers.center_meridian,
        datum_str(datum));
  }
  else {
    sprintf(albers_projection_description,
        "+proj=aea +lat_1=%f +lat_2=%f +lat_0=%f +lon_0=%f +a=%f +rf=%f",
        pps->albers.std_parallel1,
        pps->albers.std_parallel2,
        pps->albers.orig_latitude,
        pps->albers.center_meridian,
            (float)HUGHES_SEMIMAJOR,
            (float)HUGHES_INV_FLATTENING);
  }

  return albers_projection_description;
}

int
project_albers(project_parameters_t *pps,
         double lat, double lon, double height,
         double *x, double *y, double *z, datum_type_t datum)
{
    return project_worker_arr(albers_projection_desc(pps, datum),
            &lat, &lon, &height, &x, &y, &z, 1);
}

int
project_albers_arr(project_parameters_t *pps,
       double *lat, double *lon, double *height,
       double **projected_x, double **projected_y,
       double **projected_z, long length, datum_type_t datum)
{
  return project_worker_arr(albers_projection_desc(pps, datum),
                lat, lon, height, projected_x, projected_y, projected_z,
                length);
}

int
project_albers_inv(project_parameters_t *pps, double x, double y, double z,
       double *lat, double *lon, double *height, datum_type_t datum)
{
    return project_worker_arr_inv(albers_projection_desc(pps, datum),
                  &x, &y, &z, &lat, &lon, &height, 1);
}

int
project_albers_arr_inv(project_parameters_t *pps,
           double *x, double *y, double *z,
           double **lat, double **lon, double **height,
           long length, datum_type_t datum)
{
    return project_worker_arr_inv(albers_projection_desc(pps, datum),
                  x, y, z, lat, lon, height, length);
}

/******************************************************************************
  Pseudo Projection
******************************************************************************/

static char * pseudo_projection_description(datum_type_t datum)
{
  static char pseudo_projection_description[128];

  asfRequire(datum != HUGHES_DATUM,
             "Using a Hughes-1980 ellipsoid with a pseudo lat/long "
             "projection\nis not supported.\n");
  sprintf(pseudo_projection_description, "+proj=latlong +datum=%s",
          datum_str(datum));

  return pseudo_projection_description;
}

int
project_pseudo (project_parameters_t *pps, double lat, double lon,
    double height, double *x, double *y, double *z, datum_type_t datum)
{
  return project_worker_arr(pseudo_projection_description (datum),
          &lat, &lon, &height, &x, &y, &z, 1);
}

int
project_pseudo_inv (project_parameters_t *pps, double x, double y,
        double z, double *lat, double *lon, double *height, datum_type_t datum)
{
  return project_worker_arr_inv(pseudo_projection_description (datum),
                &x, &y, &z, &lat, &lon, &height, 1);
}

int
project_pseudo_arr (project_parameters_t *pps, double *lat, double *lon,
        double *height, double **x, double **y, double **z,
        long length, datum_type_t datum)
{
  return project_worker_arr(pseudo_projection_description (datum),
          lat, lon, height, x, y, z, length);
}

int project_pseudo_arr_inv (project_parameters_t *pps, double *x, double *y,
          double *z, double **lat, double **lon,
          double **height, long length, datum_type_t datum)
{
  return project_worker_arr_inv(pseudo_projection_description (datum),
        x, y, z, lat, lon, height, length);
}

// a generally useful function
int utm_zone(double lon)
{
    double lon_nudged = utm_nudge(lon);
    return (int) ((lon_nudged + 180.0) / 6.0 + 1.0);
}