SELECT AVG(level)
FROM CatchedPokemon cp
INNER JOIN Pokemon p ON cp.pid = p.id
WHERE type = 'Water';