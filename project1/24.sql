SELECT name 
FROM Pokemon
WHERE (name like 'A%' OR
	  name like 'E%' OR
      name like 'I%' OR
      name like 'O%' OR
      name like 'U%')
ORDER BY name;