-- Listing A-5. Estimating the Population for All Rows in the sales_regions Table Using Demographic Information in the zip5_dc Table
SELECT b.id, aggregate_value population
FROM   TABLE
       (
         SDO_SAM.AGGREGATES_FOR_LAYER
           ('ZIP5_DC', 'GEOM','SUM', 'POPULATION', 'SALES_REGIONS', 'GEOM')
       ) a,
       sales_regions b
WHERE  b.rowid = a.region_id;
