SELECT name
FROM Trainer t
INNER JOIN Gym g on t.id = g.leader_id
WHERE city = 'Brown City';