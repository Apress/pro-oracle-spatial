// Listing 12-15. Calculating a Route

if (showRoute) {

  // Extract coordinates of destination node
  double destX = Double.valueOf(f[1][numVisibleCols]).doubleValue();
  double destY = Double.valueOf(f[1][numVisibleCols+1]).doubleValue();

  // Snap destination to nearest network node
  String qd = "SELECT NODE_ID FROM " + net.getNodeTableName()
    + " WHERE SDO_NN ("+net.getNodeGeomColumn() + ","
    + " SDO_GEOMETRY (2001," + mapSrid + ", SDO_POINT_TYPE("
    + destX + "," + destY + ",NULL), NULL, NULL), "
    + "'SDO_NUM_RES=1') = 'TRUE'";
  String [][] sd  = mv.doQuery(dataSource, qd);
  int destNodeId = Integer.parseInt(sd[1][0]);

  // Snap start (mark) to nearest network node
  String qs = "SELECT NODE_ID FROM " + net.getNodeTableName()
    + " WHERE SDO_NN ("+net.getNodeGeomColumn() + ","
    + " SDO_GEOMETRY (2001," + mapSrid + ", SDO_POINT_TYPE("
    + markX + "," + markY + ",NULL), NULL, NULL), "
    + "'SDO_NUM_RES=1') = 'TRUE'";
  String [][] ss  = mv.doQuery(dataSource, qs);
  int startNodeId = Integer.parseInt(ss[1][0]);

  // Get path from mark to destination node
  Path p = NetworkManager.shortestPath(net, startNodeId, destNodeId);

  // Check that we got a valid path back
  if (p != null && p.getNoOfLinks() > 0) {

    // Compute path geometry
    p.computeGeometry(0.5);
    JGeometry g = p.getGeometry();

    // Add route geometry to map
    mv.addLinearFeature (
      g.getOrdinatesArray(),             // Ordinates
      mapSrid,                           // SRID
      "L.TRANSPARENT",                   // Line style
      null,                              // Label column
      null                               // Label style
    );

  } else
    mapError = "Unable to compute route";

}
