select nickname
from CatchedPokemon cp
JOIN Trainer t ON (t.id = cp.owner_id)
JOIN (
  	SELECT hometown, MAX(level) as max_lv
     	from CatchedPokemon 
	JOIN Trainer ON Trainer.id = CatchedPokemon.owner_id
	GROUP BY hometown
     ) max_join on (max_join.max_lv =  cp.level and max_join.hometown = t.hometown)
ORDER BY nickname;