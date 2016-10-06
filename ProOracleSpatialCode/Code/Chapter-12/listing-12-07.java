// Listing 12-7. Pan Actions

// -----------------------------------------------------------------------
// [Pan XXX] button clicked
// Shift map 50% in the desired direction.
// -----------------------------------------------------------------------
else if (userAction.equals("Pan W"))
  mv.pan (0, mapHeight/2);
else if (userAction.equals("Pan N"))
  mv.pan (mapWidth/2, 0);
else if (userAction.equals("Pan S"))
  mv.pan (mapWidth/2, mapHeight);
else if (userAction.equals("Pan E"))
  mv.pan (mapWidth, mapHeight/2);
