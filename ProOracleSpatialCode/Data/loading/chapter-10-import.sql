-- Load network metadata and map definitions
insert into user_sdo_network_metadata
  select * from my_network_metadata;
commit;
