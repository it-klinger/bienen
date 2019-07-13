-- bw.sql

drop table gewicht;
drop table waage;
drop table standort;

create table standort (
	id	int primary key,
	name	varchar(20)
);

create table waage (
	id	serial primary key,
	sta_id	int	references standort (id),
	idx	int,
	name	varchar(20)
);

create table gewicht (
	id	serial primary key,
	waa_id	int	references waage (id),
	value	int,
	ts	timestamp
);


insert into standort (id, name) values (1, 'Grabenreith') returning id;

insert into waage (sta_id, idx, name) values (1, 0, 'Waage-00');
insert into waage (sta_id, idx, name) values (1, 1, 'Waage-01');
insert into waage (sta_id, idx, name) values (1, 2, 'Waage-02');
insert into waage (sta_id, idx, name) values (1, 3, 'Waage-03');
insert into waage (sta_id, idx, name) values (1, 4, 'Waage-04');
insert into waage (sta_id, idx, name) values (1, 5, 'Waage-05');
insert into waage (sta_id, idx, name) values (1, 6, 'Waage-06');
insert into waage (sta_id, idx, name) values (1, 7, 'Waage-07');

insert into standort (id, name) values (2, 'Anhaltsberg') returning id;

insert into waage (sta_id, idx, name) values (2, 0, 'Waage-00');
insert into waage (sta_id, idx, name) values (2, 1, 'Waage-01');
insert into waage (sta_id, idx, name) values (2, 2, 'Waage-02');
insert into waage (sta_id, idx, name) values (2, 3, 'Waage-03');
insert into waage (sta_id, idx, name) values (2, 4, 'Waage-04');
insert into waage (sta_id, idx, name) values (2, 5, 'Waage-05');
insert into waage (sta_id, idx, name) values (2, 6, 'Waage-06');
insert into waage (sta_id, idx, name) values (2, 7, 'Waage-07');

insert into standort (id, name) values (3, 'Linuxhotel') returning id;

insert into waage (sta_id, idx, name) values (3, 0, 'Waage-00');
insert into waage (sta_id, idx, name) values (3, 1, 'Waage-01');

