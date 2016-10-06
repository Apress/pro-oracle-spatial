-- Listing 12-3. Loading Map Definitions
$imp spatial/spatial file=styles.dmp full=y
INSERT INTO USER_SDO_STYLES
  SELECT * FROM my_styles;
INSERT INTO USER_SDO_THEMES
  SELECT * FROM my_themes;
INSERT INTO USER_SDO_MAPS
  SELECT * FROM my_maps;
COMMIT;
