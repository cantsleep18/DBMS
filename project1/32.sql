SELECT  t.name, p.name, count(p.name) as num_caught
FROM Trainer t 
JOIN CatchedPokemon cp on t.id = cp.owner_id
JOIN (SELECT owner_id ,  count(Distinct type) as cnt
      FROM CatchedPokemon cp
      JOIN Pokemon p on cp.pid = p.id
      GROUP BY owner_id
      HAVING count(Distinct type) = 1) a on cp.owner_id = a.owner_id 
JOIN Pokemon p on cp.pid = p.id
GROUP BY t.name, p.name
ORDER BY t.name;