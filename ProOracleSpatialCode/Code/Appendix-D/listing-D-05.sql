-- Listing D-5. Initializing the georaster Column in the branches Table
UPDATE branches
   SET georaster = SDO_GEOR.INIT('BRANCHES_RDT')
 WHERE id=1;
