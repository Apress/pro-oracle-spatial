// Listing 12-14. nnSearch Action

// -----------------------------------------------------------------------
// [nnSearch] button clicked
// Search the N nearest neighbors from the current set mark.
// -----------------------------------------------------------------------
else if (userAction.equals("nnSearch")) {
  if (markX == 0 && markY == 0)
    mapError = "No address or mark set";
  else if (!mv.getThemeEnabled(nnSearchTheme))
    mapError = "Theme "+nnSearchTheme+" is not visible";
  else if (nnSearchParam <= 0)
    mapError = "Enter number of matches to search";
  else {

    // Construct spatial query
    String sqlQuery = "SELECT "+geoColumn+", SDO_NN_DISTANCE(1) DISTANCE"
      + " FROM " + nnSearchTheme
      + " WHERE SDO_NN ("+ geoColumn + ","
      + " SDO_GEOMETRY (2001," + mapSrid + ", SDO_POINT_TYPE("
      + markX + "," + markY + ",NULL), NULL, NULL), "
      + "'SDO_NUM_RES="+nnSearchParam+"',1) = 'TRUE'"
      + " ORDER BY DISTANCE";
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
    String[][] f = mv.queryNN(
      dataSource,                 // Data source
      nnSearchTheme,              // Theme to search
      colsToSelect,               // Names of columns to select
      nnSearchParam,              // Number of neighbors
      markX, markY,               // Center point (current mark)
      null,                       // Extra condition
      false,                      // Center point is in ground coordinates
      null
    );

    if (f== null || f.length == 0)
      mapError = "No matching " + nnSearchTheme + " found";
    else {

      // The result is one row per matching record, but we want to display
      // results as one column per record.
      featureInfo = new String[f[0].length][f.length];
      for (int i=0; i<f.length; i++)
        for (int j=0; j<f[i].length; j++)
          featureInfo[j][i] = f [i][j];
      featuresFound = f.length-1;

      // Remove any existing route
      mv.removeAllLinearFeatures();

      // Refresh map
      mv.run();
    }
  }
}
