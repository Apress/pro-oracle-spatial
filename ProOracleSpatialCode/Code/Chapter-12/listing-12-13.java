// Listing 12-13. distSearch Action

// -----------------------------------------------------------------------
// [distSearch] button clicked
// Search for all neighbors within distance D from the current set mark.
// -----------------------------------------------------------------------
else if (userAction.equals("distSearch")) {
  if (markX == 0 && markY == 0)
    mapError = "No address or mark set";
  else if (!mv.getThemeEnabled(distSearchTheme))
    mapError = "Theme "+distSearchTheme+" is not visible";
  else if (distSearchParam <= 0)
    mapError = "Enter search distance";
  else {

    // Construct point object from current location mark
    JGeometry geom = new JGeometry(markX, markY, mapSrid);

    // Construct spatial query
    String sqlQuery = "SELECT "+geoColumn+" FROM " + distSearchTheme
      + " WHERE SDO_WITHIN_DISTANCE ("+ geoColumn + ","
      + " SDO_GEOMETRY (2001," + mapSrid + ", SDO_POINT_TYPE("
      + markX + "," + markY + ",NULL), NULL, NULL), "
      + "'DISTANCE="+distSearchParam+" UNIT=M') = 'TRUE'";
    mapError = "Executing query: "+ sqlQuery;

    // Add a JDBC theme to highlight the results of the query
    mv.addJDBCTheme (
      dataSource,                 // Data source
      "SEARCH RESULTS",           // Theme to search
      sqlQuery,                   // SQL Query
      geoColumn,                  // Name of spatial column
      null,                       // srid
      queryStyle,                 // renderStyle
      null,                       // labelColumn
      null,                       // labelStyle
      true                        // passThrough
    );

    // Perform the query
    String[][] f = mv.queryWithinRadius(
      dataSource,                 // Data source
      distSearchTheme,            // Theme to search
      colsToSelect,               // Names of columns to select
      null,                       // Extra condition
      markX, markY,               // Center point (current mark)
      distSearchParam,            // Distance to search
      false                       // Center point is in ground coordinates
    );

    if (f!= null && f.length > 0) {

      // The result is one row per matching record, but we want to display
      // results as one column per record.
      featureInfo = new String[f[0].length][f.length];
      for (int i=0; i<f.length; i++)
        for (int j=0; j<f[i].length; j++)
          featureInfo[j][i] = f [i][j];
      featuresFound = f.length-1;

      // Refresh map
      mv.run();

    } else
      mapError = "No matching " + distSearchTheme + " found";

  }
}
