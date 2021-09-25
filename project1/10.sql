SELECT p.name
FROM Pokemon p 
LEFT OUTER JOIN CatchedPokemon cp ON (p.id = cp.pid)
WHERE cp.owner_id is NULL
ORDER BY p.name;