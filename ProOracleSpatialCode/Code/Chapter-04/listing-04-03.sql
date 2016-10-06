-- Listing 4-3: Example of SDO_GTYPE in the location column of customers table.
SELECT a.location.sdo_gtype
FROM customers a
WHERE id=1;
