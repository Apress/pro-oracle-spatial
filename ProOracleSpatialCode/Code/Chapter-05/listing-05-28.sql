-- Listing 5-28. Using the VALIDATE_LAYER_WITH_CONTEXT Procedure
CREATE TABLE validate_results(sdo_rowid ROWID, status VARCHAR2(2000));
BEGIN
  SDO_GEOM.VALIDATE_LAYER_WITH_CONTEXT
  (
    'SALES_REGIONS',
    'GEOM',
    'VALIDATE_RESULTS'
  );
END;
/
SELECT * FROM validate_results;
