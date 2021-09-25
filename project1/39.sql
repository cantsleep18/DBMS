SELECT name
FROM Pokemon p 
INNER JOIN CatchedPokemon cp on p.id = cp. pid
WHERE owner_id = (SELECT id
                  FROM Trainer t
                  INNER JOIN Gym g on t.id = g.leader_id
                  WHERE city = 'Rainbow City')
ORDER BY name;