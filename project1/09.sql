SELECT name, nickname
FROM Trainer t
JOIN
	(SELECT *
	FROM CatchedPokemon cp
	WHERE owner_id IN 
			(SELECT owner_id
	         FROM CatchedPokemon
        	 GROUP BY owner_id
     	    HAVING Count(owner_id)>=4)
	) a on t.id = a.owner_id
INNER JOIN
	(SELECT owner_id, MAX(level) as max_lv
     FROM CatchedPokemon
     GROUP BY owner_id
    ) b ON (a.owner_id = b.owner_id) and (a.level = b.max_lv)
 ORDER BY nickname;