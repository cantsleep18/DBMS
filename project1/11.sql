SELECT nickname
FROM CatchedPokemon cp
JOIN Pokemon p ON (cp.pid = p.id)
WHERE owner_id = 
	(SELECT leader_id
     FROM Gym
     WHERE city = 'Sangnok City'
    )
    AND
    type = 'Water'
ORDER BY nickname;