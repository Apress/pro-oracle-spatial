// Listing 12-11. identify Action

// -----------------------------------------------------------------------
// Map clicked to identify a feature.
// Get the coordinates of the clicked point
// use them to query the feature from the selected theme
// -----------------------------------------------------------------------
else if (userAction.equals("identify")) {

  // Extract coordinates of mouse click
  int imgCX = Integer.parseInt(request.getParameter("mapImage.x"));
  int imgCY = Integer.parseInt(request.getParameter("mapImage.y"));

  if (identifyTheme == null)
    mapError = "No theme selected to identify";
  else if (!mv.getThemeEnabled(identifyTheme))
    mapError = "Theme "+identifyTheme+" is not visible";
  else {

    // Locate the feature and get details
    // Notes:
    //   1. The identify() method needs a TABLE NAME, not a theme name.
    //      We just assume that the theme and table name are the same.
    //   2. We query a rectangle of 4 pixels around the user click. Notice,
    //      however, that pixels have their origin at the UPPER-LEFT corner
    //      of the image, whereas ground coordinates use the LOWER-LEFT
    //      corner.
    String[][] f = mv.identify(dataSource, identifyTheme, colsToSelect,
      geoColumn, mapSrid,
      imgCX-4, imgCY+4,
      imgCX+4, imgCY-4,
      false);

    // The result is one row per matching record, but we want to display
    // results as one column per record.
    if (f!= null && f.length > 0) {
      featureInfo = new String[f[0].length][f.length];
      for (int i=0; i<f.length; i++)
        for (int j=0; j<f[i].length; j++)
          featureInfo[j][i] = f [i][j];
      featuresFound = f.length-1;
    } else
      mapError = "No matching " + identifyTheme + " found";

    if (featuresFound > 0) {

      // Remove any existing marker
      mv.removeAllPointFeatures();

      // Remove any route from previous mark
      mv.removeAllLinearFeatures();

      // Add a marker at the point clicked
      Point2D p = mv.getUserPoint(imgCX,imgCY);
      mv.addPointFeature (p.getX(), p.getY(),
        mapSrid, markerStyle, null, null, null);

      // Save new mark
      markX = p.getX();
      markY = p.getY();

      // Refresh map
      mv.run();
    }
  }
}
