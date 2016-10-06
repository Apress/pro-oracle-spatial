-- Listing 4-1: Creating a table to store all geometry examples
CREATE TABLE geometry_examples
(
  name             VARCHAR2(100),
  description      VARCHAR2(100),
  geom             SDO_GEOMETRY
);
