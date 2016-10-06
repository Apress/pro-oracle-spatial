-- Listing A-3. Searching for Regions (Tiles) That Have a Population Greater Than 30,000
SELECT REGION_ID, AGGREGATE_VALUE, GEOMETRY
FROM TABLE
     (
       SDO_SAM.TILED_AGGREGATES
         ('ZIP5_DC', 'GEOM','SUM', 'POPULATION', 2)
     )
WHERE aggregate_value > 30000;
