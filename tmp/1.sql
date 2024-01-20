--update ip_term set occurences = (select ) where ip_id = 
--select ip_id, count (*) from ip_term group by ip_id ORDER BY ip_id
--select ip_term.*, a.t s, times*1./(select sum(times) from ip_term where ip_id = 694) occ, a.t*1./(select sum(times) from ip_term) occdb, weight - times*1./(select sum(times) from ip_term where ip_id = 694) + a.t*1./(select sum(times) from ip_term) w
--  from ip_term inner join (select term, sum(times) t from ip_term group by term) a on ip_term.term = a.term where ip_id = 694

select term, sum(times) t from ip_term group by term order by term

--SELECT * FROM ip_term where ip_id=658 order by term
--update ip_term set occurences = times*1./(select sum(times) from ip_term where ip_id = 658) where ip_id = 658
--update ip_term b set weight = times*1./(select sum(c.times) from ip_term c where c.ip_id = 658) - (select sum(a.times) from ip_term a where a.term = b.term)*1./(select sum(d.times) from ip_term d) where b.ip_id = 658
--SELECT * INTO ip_term_658 FROM ip_term where ip_id = 658

--select * from ip_term where term = 'АВТОЗАПЧАСТЬ'
--delete from ip_term where id=0
--select ip_term.*, a.t s, times*1./(select sum(times) from ip_term where ip_id = 694) occ, a.t*1./(select sum(times) from ip_term) occdb, weight - times*1./(select sum(times) from ip_term where ip_id = 694) + a.t*1./(select sum(times) from ip_term) w
--  from ip_term inner join (select term, sum(times) t from ip_term group by term) a on ip_term.term = a.term where ip_id = 694 order by term
--select * from ip_term where ip_id = 694