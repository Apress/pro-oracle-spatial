-- Load map definitions
insert into user_sdo_styles
  select * from my_styles;
insert into user_sdo_themes
  select * from my_themes;
insert into user_sdo_maps
  select * from my_maps;
commit;
