-- Listing 11-2. Loading Maps, Themes, and Style Definitions
-- imp spatial/spatial file=styles.dmp full=y
INSERT INTO USER_SDO_STYLES
  SELECT * FROM MY_STYLES;
INSERT INTO USER_SDO_THEMES
  SELECT * FROM MY_THEMES;
INSERT INTO USER_SDO_MAPS
  SELECT * FROM MY_MAPS;
COMMIT;