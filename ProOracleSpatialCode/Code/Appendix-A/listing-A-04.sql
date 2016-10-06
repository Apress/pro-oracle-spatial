-- Listing A-4. Estimating the Population in Sales Region 1 Using the Demographic Information in the zip5_dc Table
SELECT SDO_SAM.AGGREGATES_FOR_GEOMETRY ('ZIP5_DC', 'GEOM', 'SUM', 'POPULATION', geom) population
FROM sales_regions
WHERE id=1;
