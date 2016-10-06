// Listing 12-9. updateMap Action

// -----------------------------------------------------------------------
// [Update Map] button clicked
// Enable the themes selected by the user and refresh the map
// -----------------------------------------------------------------------
else if (userAction.equals("Update Map")) {
  if (checkedThemes == null)
    mv.setAllThemesEnabled(false);
  else
    mv.enableThemes(checkedThemes);
  mv.run();
}
