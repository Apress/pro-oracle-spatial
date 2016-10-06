-- Listing 9-2. Creating Buffers Around Competitor Locations
CREATE TABLE COMPETITORS_SALES_REGIONS AS
  SELECT id,
         SDO_GEOM.SDO_BUFFER(a.location, 0.25, 0.5, 'unit=mile arc_tolerance=0.005') geom
  FROM competitors a;
