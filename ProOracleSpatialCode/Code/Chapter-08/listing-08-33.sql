-- Listing 8-33. SDO_FILTER Operator Retrieving All Customers Within a Competitorís Service Area
SELECT ct.id, ct.name
  FROM competitors_regions comp, customers ct
 WHERE comp.id=1
   AND SDO_FILTER(ct.location, comp.geom)='TRUE'
 ORDER BY ct.id;
