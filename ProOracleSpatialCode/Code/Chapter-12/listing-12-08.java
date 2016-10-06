// Listing 12-8. reCenter Action

// -----------------------------------------------------------------------
// Map clicked to recenter
// Use the coordinates of the clicked point as new map center
// -----------------------------------------------------------------------
else if (userAction.equals("reCenter")) {
  // Extract coordinates of mouse click
  int imgCX = Integer.parseInt(request.getParameter("mapImage.x"));
  int imgCY = Integer.parseInt(request.getParameter("mapImage.y"));
  // Pan to that position
  mv.pan (imgCX, imgCY);
}
