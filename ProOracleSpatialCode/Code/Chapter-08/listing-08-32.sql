-- Listing 8-32. Creating Indexes on Sales Regions of Competitors/Branches
Rem Metadata for Sales_regions table
INSERT INTO USER_SDO_GEOM_METADATA
  SELECT 'SALES_REGIONS','GEOM', DIMINFO, SRID
  FROM USER_SDO_GEOM_METADATA
  WHERE TABLE_NAME='BRANCHES';

Rem Metadata for Competitors_regions table
INSERT INTO USER_SDO_GEOM_METADATA
  SELECT 'COMPETITORS_SALES_REGIONS','GEOM', DIMINFO, SRID
  FROM USER_SDO_GEOM_METADATA
  WHERE TABLE_NAME='COMPETITORS';

Rem Index-creation for Sales_regions table
CREATE INDEX sr_sidx ON sales_regions(geom)
  INDEXTYPE IS MDSYS.SPATIAL_INDEX;

Rem Index-creation for Competitors_sales_regions table
CREATE INDEX cr_sidx ON competitors_sales_regions(geom)
  INDEXTYPE IS MDSYS.SPATIAL_INDEX;
