SELECT name
FROM Pokemon p
JOIN
	(SELECT pid
     FROM CatchedPokemon cp
     INNER JOIN Trainer t on cp.owner_id = t.id
     WHERE hometown = 'Sangnok city') a on p.id = a.pid
JOIN
	(SELECT pid
     FROM CatchedPokemon cp
     INNER JOIN Trainer t on cp.owner_id = t.id
     WHERE hometown = 'Blue city') b on a.pid = b.pid
ORDER BY name;