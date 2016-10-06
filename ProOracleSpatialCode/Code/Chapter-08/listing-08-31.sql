-- Listing 8-31. Creating the Sales Region (Area of Influence) for Each Competitor/Branch

CREATE TABLE COMPETITORS_SALES_REGIONS AS
  SELECT id, name, SDO_GEOM.SDO_BUFFER (a.location, 0.25, 0.5, 'UNIT=MILE ARC_TOLERANCE=0.005') geom
  FROM competitors a;

CREATE TABLE SALES_REGIONS AS
  SELECT id, name, SDO_GEOM.SDO_BUFFER (a.location, 0.25, 0.5, 'UNIT=MILE ARC_TOLERANCE=0.005') geom
  FROM branches a;
