// Listing 12-12. setMark Action

// -----------------------------------------------------------------------
// Map clicked to set a mark
// Get the coordinates of the clicked point and use them to set a mark
// at that point
// -----------------------------------------------------------------------
else if (userAction.equals("setMark")) {

  // Extract coordinates of mouse click
  int imgCX = Integer.parseInt(request.getParameter("mapImage.x"));
  int imgCY = Integer.parseInt(request.getParameter("mapImage.y"));

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
