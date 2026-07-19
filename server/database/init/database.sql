-- Script for creating database with tables

create table if not exists users (
  id serial primary key,
  login varchar(30) unique not null,
  passwd text not null,
  last_seen timestamp default null,
  avatar text default null
);

create table if not exists msgs_queue (
  id serial primary key,
  sender_id int not null,
  receiver_id int not null,
  content text not null,
  sent_at timestamp not null,
  constraint fk_sender foreign key (sender_id) references users(id),
  constraint fk_receiver foreign key (receiver_id) references users(id),
  constraint chk_sender_receiver check (sender_id != receiver_id)
);

create table if not exists relations (
  id serial primary key,
  user_id int not null,
  friend_id int not null,
  status varchar(10) not null,
  constraint fk_user foreign key (user_id) references users(id),
  constraint fk_friend foreign key (friend_id) references users(id),
  constraint uc_user_friend unique (user_id, friend_id),
  constraint chk_user_friend check (user_id != friend_id)
);
