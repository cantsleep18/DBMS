SELECT t.name
FROM Trainer t
JOIN CatchedPokemon cp ON t.id=cp.owner_id
JOIN Pokemon p ON cp.pid = p.id
WHERE hometown = 'Sangnok City' AND p.name LIKE 'p%'
ORDER BY name;