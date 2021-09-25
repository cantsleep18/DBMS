SELECT name
FROM Pokemon p
JOIN Evolution e ON p.id = e.before_id
WHERE before_id > after_id
ORDER BY name;