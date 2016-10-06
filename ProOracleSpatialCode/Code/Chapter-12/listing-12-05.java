// Listing 12-5. Reset Action

// -----------------------------------------------------------------------
// [Reset] button clicked
// Initialize the MapViewer object with the original center and size
// Load the network representation
// -----------------------------------------------------------------------
if (userAction.equals("Reset")) {

  if (mv == null)
    mapError = "Session lost - Resetting";

  // Create and initialize new MapViewer object)
  mv = new MapViewer(mapViewerURL);
  mv.setDataSourceName(dataSource);                     // Data source
  mv.setBaseMapName(baseMap);                           // Base map
  for(int i=0; i<appThemes.length; i++) {               // Additional themes
    mv.addPredefinedTheme(appThemes[i]);                // Theme name
    mv.setThemeScale(appThemes[i],
      appThemeMinScale, 0.0);                           // Scale limits
  }
  mv.setAllThemesEnabled(false);                        // Themes disabled
  mv.setMapTitle("  ");                                 // No title
  mv.setImageFormat(MapViewer.FORMAT_PNG_URL);          // Map format
  mv.setDeviceSize(new Dimension(mapWidth, mapHeight)); // Map size

  // Save MapViewer object in session
  session.setAttribute("MapviewerHandle", mv);

  // Get JDBC database connection
  InitialContext ic = new InitialContext();
  DataSource ds = (DataSource) ic.lookup("jdbc/"+dataSource.toLowerCase());
  Connection conn = ds.getConnection();

  // Load network
  net = NetworkManager.readNetwork(conn, networkName);

  // Save Network object in session
  session.setAttribute("NetworkHandle", net);

  // Release database connection
  conn.close();

  // Set initial map position and display it
  mv.setCenterAndSize(initialCx, initialCy, initialSize);
  mv.run();

  // Set default options
  clickAction = "reCenter";
  showRoute = false;
  markX = 0;
  markY = 0;
}
