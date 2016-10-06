// Listing 12-10. Find Action

// -----------------------------------------------------------------------
// [Find] button clicked:
// Geocode the entered address.
// Center map on the resulting coordinates.
// Set mark on that point.
// -----------------------------------------------------------------------
else if (userAction.equals("Find")) {

  // Extract address details
  String[] addressLines = findAddress.split("\r\n");

  // Construct query to geocoder
  String gcQuery =
    "SELECT "+
    "G.GEO_ADDR.MATCHCODE, G.GEO_ADDR.LONGITUDE, G.GEO_ADDR.LATITUDE, " +
    "G.GEO_ADDR.HOUSENUMBER || ' ' || G.GEO_ADDR.STREETNAME, " +
    "G.GEO_ADDR.SETTLEMENT || ' ' || G.GEO_ADDR.POSTALCODE " +
    "FROM (SELECT SDO_GCDR.GEOCODE(USER ,SDO_KEYWORDARRAY(";
  for (int i=0; i<addressLines.length; i++) {
    gcQuery = gcQuery + "'" + addressLines[i] + "'";
    if (i < addressLines.length-1)
      gcQuery = gcQuery + ",";
  }
  gcQuery = gcQuery + "), 'US', 'DEFAULT') " +
  "GEO_ADDR FROM DUAL) G";

  // Send query
  String[][] f = mv.doQuery(dataSource, gcQuery);

  // Extract match code. Proceed only if > 0
  int matchCode = Integer.parseInt(f[1][0]);
  if (matchCode > 0) {

    // Extract X and Y coordinates from geocode result
    double destX = Double.valueOf(f[1][1]).doubleValue();
    double destY = Double.valueOf(f[1][2]).doubleValue();

    // Extract full street address from result
    String streetAddress = f[1][3];

    // Transform result from row-major to column-major
    geocodeInfo = new String[f[0].length-3];
    for (int i=0; i<f[0].length-3; i++)
      geocodeInfo[i] = f [1][i+3];

    // Center map on the new address and zoom in
    mv.setCenterAndSize(destX, destY, markerMapSize);

    // Remove any existing marker
    mv.removeAllPointFeatures();

    // Remove any route from that mark
    mv.removeAllLinearFeatures();

    // Add a marker at the point clicked and label it
    // with the first address line
    mv.addPointFeature (
      destX, destY,
      mapSrid,
      markerStyle,
      streetAddress,
      markerLabelStyle,
      null,
      true);

    // Save new mark
    markX = destX;
    markY = destY;

    // Show SQL statement
    mapError = gcQuery;

    // Refresh map
    mv.run();
  }
  else
    mapError = "Address not found";
}
