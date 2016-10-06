-- Listing 3-13: Selecting srids of Projected Coordinate Systems.
SELECT SRID
FROM MDSYS.CS_SRS
WHERE WKTEXT LIKE 'PROJCS%';
